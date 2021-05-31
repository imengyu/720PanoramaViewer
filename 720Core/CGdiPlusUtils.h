#pragma once
#include "stdafx.h"
#include <ObjIdl.h>
#include <ShlObj.h>
#include "utils.h"
#include <gdiplus.h>

using namespace Gdiplus;

class CGdiPlusUtils
{
public:
    static PropertyItem* GetPropertyItemFromImage(Image* bm, PROPID propId);
    static UINT VoidToInt(void* data, unsigned int size);
    static Bitmap* BitmapFromMemory(BYTE* data, unsigned int imageSize);

    static Bitmap* GetThumbnail(Image* bm, int width, int height, Gdiplus::Size* realSize = 0);
    static Bitmap* GetThumbnail(std::wstring& filename, int width, int height, Gdiplus::Size* realSize);
    static Size AdaptProportionalSize(const Size& szMax, const Size& szReal);

    static INT GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

    static void Init();

    static CLSID *GetPngClsid();
    static CLSID *GetJpegClsid();
};

