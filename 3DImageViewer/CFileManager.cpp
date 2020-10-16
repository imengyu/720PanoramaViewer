#include "CFileManager.h"
#include "COpenGLRenderer.h"
#include "COpenGLView.h"
#include "CImageLoader.h"

void CFileManager::OpenFile() {
   Render->View->SendMessage(WM_CUSTOM_OPEN_FILE, 0, 0);
}
void CFileManager::CloseFile() {
    if (CurrentFileLoader != nullptr) {
        CurrentFileLoader->Destroy();
        delete CurrentFileLoader;
        CurrentFileLoader = nullptr;
    }
}
void CFileManager::DoOpenFile(wchar_t* path) {

}

CFileManager::CFileManager(COpenGLRenderer* render)
{
    Render = render;
}
