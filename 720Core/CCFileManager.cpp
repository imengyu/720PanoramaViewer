#include "CCFileManager.h"
#include "COpenGLRenderer.h"
#include "COpenGLView.h"
#include "CImageLoader.h"
#include "CCFileReader.h"
#include "CApp.h"
#include "SystemHelper.h"
#include "PathHelper.h"
#include "ImageUtils.h"
#include "720Core.h"

void CCFileManager::OpenFile() {
   Render->View->SendWindowsMessage(WM_CUSTOM_OPEN_FILE, 0, 0);
}
void CCFileManager::CloseFile() {
    logger->Log(L"Closing file");
    if (onCloseCallback)
        onCloseCallback(onCloseCallbackData);
    if (CurrentFileLoader != nullptr) {
        CurrentFileLoader->Destroy();
        delete CurrentFileLoader;
        CurrentFileLoader = nullptr;
    }
    isThisCloseWillOpenNext = false;
}
void CCFileManager::DeleteCurrentFile() {
    if (CurrentFileLoader) 
        Render->View->SendWindowsMessage(WM_CUSTOM_DEL_FILE, (WPARAM)CurrentFileLoader->GetPath(), 0);
}
bool CCFileManager::DoOpenFile(const wchar_t* path) {

    isThisCloseWillOpenNext = true;
    CloseFile();

    if (!SystemHelper::FileExists(path)) {
        lastErr = L"�ļ��Ҳ�����, �����ܱ��ƶ�����������";
        return false;
    }

    CurrenImageType = CImageLoader::CheckImageType(path);
    CurrentFileLoader = CImageLoader::CreateImageLoaderAuto(path);
    if (CurrentFileLoader == nullptr) {
        lastErr = L"�ƺ���֧�������ļ���ʽ";
        return false;
    }

    logger->Log2(L"Open file \"%s\" type: %d", path, CurrenImageType);

    glm::vec2 size = CurrentFileLoader->GetImageSize();
    if (size.x > 65536 || size.y > 32768) {
        logger->LogError2(L"Image size too big : %dx%d > 65536x32768", size.x, size.y);
        lastErr = L"������ʱ�޷��򿪷ǳ����ͼ��ͼ���С����65536x32768��";
        CloseFile();
        return false;
    }

    if (CurrenImageType != ImageType::JPG && (size.x > 4096 || size.y > 2048)) {
        logger->LogError2(L"Image size too big (not jpeg) : %dx%d > 4096x2048", size.x, size.y);
        lastErr = L"������ʱ�޷��򿪷ǳ����ͼ��ͼ���С����4096x2048�����ɳ���תΪJPEG��ʽ�ٴ�";
        CloseFile();
        return false;
    }

    ImageRatioNotStandard = !ImageUtils::CheckSizeIsNormalPanorama(size);

    UpdateLastError();
    return true;
}
const wchar_t* CCFileManager::GetLastError()
{
    return lastErr.c_str();
}
void CCFileManager::UpdateLastError()
{
    if (CurrentFileLoader)
        lastErr = CurrentFileLoader->GetLastError();
}
bool CCFileManager::IsThisCloseWillOpenNext()
{
    return isThisCloseWillOpenNext;
}

const wchar_t* CCFileManager::GetCurrentFileLoadingPrecent() {
    if (CurrentFileLoader)
        imageLoadingPrecent = CStringHlp::FormatString(L"%d%%", (int)(CurrentFileLoader->GetLoadingPrecent()*100));
    return imageLoadingPrecent.c_str();
}
std::wstring CCFileManager::GetCurrentFileName() {
    if (CurrentFileLoader)
        return Path::GetFileName(CurrentFileLoader->GetPath());
    return std::wstring();
}
CCFileManager::CCFileManager(COpenGLRenderer* render)
{
    logger = Logger::GetStaticInstance();
    Render = render;
}
