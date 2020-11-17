#include "CGdiPlusUtils.h"
#include "SystemHelper.h"
#include <string>
#include <math.h>
#include <Shlwapi.h>
#include "utils.h"

PropertyItem* CGdiPlusUtils::GetPropertyItemFromImage(Image* bm, PROPID propId) {
    UINT itemSize = bm->GetPropertyItemSize(propId);
    if (!itemSize) {
        return 0;
    }
    PropertyItem* item = reinterpret_cast<PropertyItem*>(malloc(itemSize));
    if (bm->GetPropertyItem(propId, itemSize, item) != Ok) {
        free(item);
        return 0;
    }
    return item;
}
UINT CGdiPlusUtils::VoidToInt(void* data, unsigned int size) {
    switch (size) {
    case 8:
        return *reinterpret_cast<UINT*>(data);
    case 4:
        return *reinterpret_cast<DWORD*>(data);
    case 2:
        return *reinterpret_cast<WORD*>(data);
    default:
        return *reinterpret_cast<BYTE*>(data);
    }
}
Bitmap* CGdiPlusUtils::BitmapFromMemory(BYTE* data, unsigned int imageSize) {
    if (SystemHelper::IsVistaOrLater()) {

        Bitmap* bitmap;
        IStream* pStream = SHCreateMemStream(data, imageSize);
        if (pStream) {
            bitmap = Bitmap::FromStream(pStream);
            pStream->Release();
            if (bitmap) {
                if (bitmap->GetLastStatus() == Gdiplus::Ok) {
                    return bitmap;
                }
                delete bitmap;
            }
        }
    }
    else {
        HGLOBAL buffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
        if (buffer) {
            void* pBuffer = ::GlobalLock(buffer);
            if (pBuffer) {
                Bitmap* bitmap;
                CopyMemory(pBuffer, data, imageSize);

                IStream* pStream = NULL;
                if (::CreateStreamOnHGlobal(buffer, FALSE, &pStream) == S_OK) {
                    bitmap = Bitmap::FromStream(pStream);
                    pStream->Release();
                    if (bitmap) {
                        if (bitmap->GetLastStatus() == Gdiplus::Ok) {
                            return bitmap;
                        }

                        delete bitmap;
                    }
                }
                ::GlobalUnlock(buffer);
            }
            ::GlobalFree(buffer);
        }
    }

    return 0;
}

// Based on original method from http://danbystrom.se/2009/01/05/imagegetthumbnailimage-and-beyond/

Bitmap* CGdiPlusUtils::GetThumbnail(Image* bm, int width, int height, Gdiplus::Size* realSize) {
    using namespace Gdiplus;
    if (realSize) {
        realSize->Width = bm->GetWidth();
        realSize->Height = bm->GetHeight();
    }
    Size sz = AdaptProportionalSize(Size(width, height), Size(bm->GetWidth(), bm->GetHeight()));
    Bitmap* res = new Bitmap(sz.Width, sz.Height);
    Graphics gr(res);

    gr.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    UINT size = bm->GetPropertyItemSize(PropertyTagThumbnailData);
    if (size) {
        // Loading thumbnail from EXIF data (fast)
        enum ThumbCompression { ThumbCompressionJPEG, ThumbCompressionRGB, ThumbCompressionYCbCr, ThumbCompressionUnknown }
        compression = ThumbCompressionJPEG;

        PropertyItem* thumbnailFormatItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailFormat);
        if (thumbnailFormatItem) {
            UINT format = VoidToInt(thumbnailFormatItem->value, thumbnailFormatItem->length);
            if (format == 0) {
                compression = ThumbCompressionRGB;
            }
            else if (format == 1) {
                compression = ThumbCompressionJPEG;
            }
            else {
                compression = ThumbCompressionUnknown;
            }
            free(thumbnailFormatItem);
        }
        else {
            PropertyItem* compressionItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailCompression);
            if (compressionItem) {
                WORD compressionTag = *reinterpret_cast<WORD*>(compressionItem->value);
                if (compressionTag == 1) {
                    compression = ThumbCompressionRGB;
                    PropertyItem* photometricInterpretationItem = GetPropertyItemFromImage(bm, PropertyTagPhotometricInterp);
                    if (photometricInterpretationItem) {
                        WORD photoMetricInterpretationTag = VoidToInt(photometricInterpretationItem->value, photometricInterpretationItem->length);
                        free(photometricInterpretationItem);
                        if (photoMetricInterpretationTag == 6) {
                            compression = ThumbCompressionYCbCr;
                        }
                    }

                }
                else if (compressionTag == 6) {
                    compression = ThumbCompressionJPEG;
                }

                free(compressionItem);
            }
        }

        int originalThumbWidth = 0, originalThumbHeight = 0;
        if (compression == ThumbCompressionJPEG || compression == ThumbCompressionRGB) {
            PropertyItem* thumbDataItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailData);
            if (thumbDataItem) {
                if (compression == ThumbCompressionJPEG) {
                    Bitmap* src = BitmapFromMemory(reinterpret_cast<BYTE*>(thumbDataItem->value), thumbDataItem->length);

                    if (src) {
                        gr.DrawImage(src, 0, 0, sz.Width, sz.Height);
                        delete src;
                        free(thumbDataItem);
                        return res;
                    }
                }
                else if (compression == ThumbCompressionRGB) {
                    PropertyItem* widthItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailImageWidth);
                    if (widthItem) {
                        originalThumbWidth = VoidToInt(widthItem->value, widthItem->length);
                        free(widthItem);
                    }
                    PropertyItem* heightItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailImageHeight);
                    if (heightItem) {
                        originalThumbHeight = VoidToInt(heightItem->value, heightItem->length);
                        free(heightItem);
                    }
                    if (originalThumbWidth && originalThumbHeight) {
                        BITMAPINFOHEADER bih;
                        memset(&bih, 0, sizeof(bih));
                        bih.biSize = sizeof(bih);
                        bih.biWidth = originalThumbWidth;
                        bih.biHeight = -originalThumbHeight;
                        bih.biPlanes = 1;
                        bih.biBitCount = 24;

                        BITMAPINFO bi;
                        memset(&bi, 0, sizeof(bi));
                        bi.bmiHeader = bih;

                        BYTE* data = reinterpret_cast<BYTE*>(thumbDataItem->value);
                        BYTE temp;
                        // Convert RGB to BGR
                        for (unsigned int offset = 0; offset < thumbDataItem->length; offset += 3) {
                            temp = data[offset];
                            data[offset] = data[offset + 2];
                            data[offset + 2] = temp;
                        }
                        Bitmap src(&bi, thumbDataItem->value);

                        if (src.GetLastStatus() == Ok) {
                            gr.DrawImage(&src, 0, 0, sz.Width, sz.Height);
                            free(thumbDataItem);
                            return res;
                        }
                    }

                }
                else {
                    // other type of compression not implemented
                }
                free(thumbDataItem);
            }
        }
    }
    // Fallback - Load full image and draw it  (slow)
    gr.DrawImage(bm, 0, 0, sz.Width, sz.Height);

    return res;
}
Bitmap* CGdiPlusUtils::GetThumbnail(std::wstring& filename, int width, int height, Gdiplus::Size* realSize) {
    using namespace Gdiplus;
    Image* bm = Image::FromFile(filename.c_str());
    if (bm->GetLastStatus() != Ok) {
        LOGEF(L"[CGdiPlusUtils::GetThumbnail] Load image failed! %s, Status: %d", filename.c_str(), bm->GetLastStatus());
        return 0;
    }
    Bitmap* bs = GetThumbnail(bm, width, height, realSize);
    delete bm;
    return bs;
}
Gdiplus::Size CGdiPlusUtils::AdaptProportionalSize(const Gdiplus::Size& szMax, const Gdiplus::Size& szReal)
{
    int nWidth;
    int nHeight;
    double sMaxRatio;
    double sRealRatio;

    if (szMax.Width < 1 || szMax.Height < 1 || szReal.Width < 1 || szReal.Height < 1)
        return Size();

    sMaxRatio = szMax.Width / static_cast<double>(szMax.Height);
    sRealRatio = szReal.Width / static_cast<double>(szReal.Height);

    if (sMaxRatio < sRealRatio) {
        nWidth = min(szMax.Width, szReal.Width);
        nHeight = static_cast<int>(round(nWidth / sRealRatio));
    }
    else {
        nHeight = min(szMax.Height, szReal.Height);
        nWidth = static_cast<int>(round(nHeight * sRealRatio));
    }

    return Size(nWidth, nHeight);
}

INT CGdiPlusUtils::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders      
    UINT  size = 0;         // size of the image encoder array in bytes      

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure      

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure      

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success      
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure     
}

CLSID pngClsid;
CLSID jpegClsid;

void CGdiPlusUtils::Init()
{
    GetEncoderClsid(L"image/jpeg", &jpegClsid);
    GetEncoderClsid(L"image/png", &pngClsid);
}

CLSID *CGdiPlusUtils::GetPngClsid() {  return &pngClsid; }
CLSID *CGdiPlusUtils::GetJpegClsid() { return &jpegClsid; }

