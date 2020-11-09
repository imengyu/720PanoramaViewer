#include "CCFileManager.h"
#include "COpenGLRenderer.h"
#include "COpenGLView.h"
#include "CImageLoader.h"
#include "CCFileReader.h"
#include "CApp.h"
#include "SystemHelper.h"
#include "PathHelper.h"

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
}
void CCFileManager::DeleteCurrentFile() {
    if (CurrentFileLoader) 
        Render->View->SendWindowsMessage(WM_CUSTOM_DEL_FILE, (WPARAM)CurrentFileLoader->GetPath(), 0);
}
void CCFileManager::OpenCurrentFileAs() {
    if (CurrentFileLoader)
        SystemHelper::OpenAs(CurrentFileLoader->GetPath());
}
std::wstring CCFileManager::GetCurrentFileName() {
    if (CurrentFileLoader) 
        return Path::GetFileName(CurrentFileLoader->GetPath());
    return std::wstring();
}
bool CCFileManager::DoOpenFile(const wchar_t* path) {

    CloseFile();

    if (!SystemHelper::FileExists(path)) {
        lastErr = L"�ļ�������";
        return false;
    }

    CurrenImageType = CImageLoader::CheckImageType(path);
    CurrentFileLoader = CImageLoader::CreateImageLoaderAuto(path);
    if (CurrentFileLoader == nullptr) {
        lastErr = L"��֧�������ļ���ʽ";
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
        lastErr = L"��ͼ����תΪJPEG��ʽ�򿪣�ͼ���С����4096x2048��";
        CloseFile();
        return false;
    }

    long long fileSize = 0;
    HANDLE hFile = CreateFile(path, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile) {
        LARGE_INTEGER liFileSize;
        if(GetFileSizeEx(hFile, &liFileSize))
            fileSize = liFileSize.QuadPart;
        CloseHandle(hFile);
    }

    imageInfoTitle = Path::GetFileName(path);
    imageInfoTitle += CStringHlp::FormatString(L" (%dx%dx%db, %hs)", (int)size.x, (int)size.y,
        (int)CurrentFileLoader->GetImageDepth(), CStringHlp::GetFileSizeStringAuto(fileSize).c_str());

    ImageRatioNotStandard = (glm::abs(size.x / size.y - 2.0f) > 0.2f);
    return true;
}
const wchar_t* CCFileManager::GetLastError()
{
    return lastErr.c_str();
}
const wchar_t* CCFileManager::GetCurrentFileInfoTitle()
{
    return imageInfoTitle.c_str();
}
CCFileManager::CCFileManager(COpenGLRenderer* render)
{
    logger = Logger::GetStaticInstance();
    Render = render;
}
