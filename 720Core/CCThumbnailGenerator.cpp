#include "CCThumbnailGenerator.h"
#include "SystemHelper.h"
#include "PathHelper.h"
#include "CApp.h"
#include "CStringHlp.h"
#include <ObjIdl.h>
#include <shlwapi.h>
#include <string>
#include <math.h>
#include "utils.h"
#include "md5.h"

wchar_t* CCThumbnailGenerator::GetImageThumbnailAuto(const wchar_t* filePath)
{
    std::wstring filename(filePath);
    std::wstring filePathMD5 = md5(filename);
    std::wstring fileCachePath = CStringHlp::FormatString(L"%s\\thumbnaiCache\\%s", AppGetAppInstance()->GetCurrentDir(), filePathMD5.c_str());

    //Tes Thumbnail cache
    if (Path::Exists(fileCachePath))
        return CStringHlp::AllocString(fileCachePath);

    //Generate Thumbnail
    Bitmap* bitmap = CGdiPlusUtils::GetThumbnail(filename, 300, 150, nullptr);
    if (!bitmap) {
        LOGEF(L"[CCThumbnailGenerator] GetImageThumbnail failed! %s", filePath);
        return nullptr;
    }

    Gdiplus::Status stat = bitmap->Save(fileCachePath.c_str(), CGdiPlusUtils::GetJpegClsid(), nullptr);
    if (stat != Gdiplus::Status::Ok) {
        LOGEF(L"[CCThumbnailGenerator] GetImageThumbnail failed when save bitmap ! %s, Status: %d", filePath, stat);
        delete bitmap;
        return nullptr;
    }

    return CStringHlp::AllocString(fileCachePath);
}
Bitmap* CCThumbnailGenerator::GetImageThumbnail(const wchar_t* filePath, int w, int h)
{
    std::wstring filename(filePath);
    return CGdiPlusUtils::GetThumbnail(filename, w, h, nullptr);
}
