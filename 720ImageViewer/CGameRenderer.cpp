#include "CGameRenderer.h"
#include "COpenGLView.h"
#include "CImageLoader.h"
#include "CCRenderGlobal.h"
#include "CCursor.h"
#include "CApp.h"
#include "StringHlp.h"
#include "SettingHlp.h"
#include <Shlwapi.h>
#include <time.h>

CGameRenderer::CGameRenderer()
{
    logger = CApp::Instance->GetLogger();
}
CGameRenderer::~CGameRenderer()
{
}

void CGameRenderer::SetOpenFilePath(const wchar_t* path)
{
	currentOpenFilePath = path;
}
void CGameRenderer::DoOpenFile()
{
    loading_dialog_active = true;
    if (fileManager->DoOpenFile(currentOpenFilePath.c_str())) {

        if (fileManager->ImageRatioNotStandard) {
            if (uiWapper->ShowConfirmBox(L"����������ͼƬ�ĳ���Ȳ��� 2:1�����Ǳ�׼��720��ȫ��ͼ�����Ҫ��ʾ��ͼ�񣬿��ܻᵼ��ͼ����Σ��Ƿ������", 
                L"��ʾ", L"", L"", CAppUIMessageBoxIcon::IconWarning) == CAppUIMessageBoxResult::ResultCancel)
            {
                welecome_dialog_active = true;
                file_opened = false;
                renderer->renderNoPanoramaSmall = true;
                loading_dialog_active = false;
                return;
            }
        }

        LoadImageInfo();

        //��ͼ
        renderer->panoramaThumbnailTex = texLoadQueue->Push(new CCTexture(), 0, 0, -1);//MainTex
        renderer->panoramaTexPool.push_back(renderer->panoramaThumbnailTex);
        renderer->UpdateMainModelTex();

        //����Ƿ���Ҫ��Ƭ������
        needTestImageAndSplit = true;
        welecome_dialog_active = false;
        file_opened = true;
        renderer->renderNoPanoramaSmall = false;
    }
    else {
        welecome_dialog_active = true;
        file_opened = false;
        renderer->renderNoPanoramaSmall = true;
        loading_dialog_active = false;
        uiWapper->ShowMessageBox(fileManager->GetLastError(), L"���ļ�ʧ��", IconWarning);
    }
}
void CGameRenderer::LoadImageInfo() {
    //��ȡͼƬ��Ϣ
    auto loader = fileManager->CurrentFileLoader;
    auto imgSize = loader->GetImageSize();
    auto imgFileInfo = loader->GetImageFileInfo();

    uiInfo->currentImageType = fileManager->CurrenImageType;
    uiInfo->currentImageName = StringHlp::UnicodeToUtf8(fileManager->GetCurrentFileName());
    uiInfo->currentImageImgSize = StringHlp::FormatString("%dx%dx%db", (int)imgSize.x, (int)imgSize.y, loader->GetImageDepth());
    uiInfo->currentImageSize = StringHlp::GetFileSizeStringAuto(imgFileInfo->fileSize);
    uiInfo->currentImageChangeDate = imgFileInfo->Write;

    uiInfo->currentImageOpened = true;
}
void CGameRenderer::TestSplitImageAndLoadTexture() {
    glm::vec2 size = fileManager->CurrentFileLoader->GetImageSize();
    SplitFullImage = size.x > 4096 || size.y > 2048;
    if (SplitFullImage) {
        float chunkW = size.x / 2048.0f;
        float chunkH = size.y / 1024.0f;
        if (chunkW < 2) chunkW = 2;
        if (chunkH < 2) chunkH = 2;
        if (chunkW > 64 || chunkH > 32) {
            logger->LogError2(L"Too big image (%.2f, %.2f) that cant split chunks.", chunkW, chunkH);
            SplitFullImage = false;
            return;
        }

        int chunkWi = (int)ceil(chunkW), chunkHi = (int)ceil(chunkH);

        uiInfo->currentImageAllChunks = chunkWi * chunkHi;
        uiInfo->currentImageLoadChunks = 0;
        logger->Log(L"Image use split mode , size: %d, %d", chunkWi, chunkHi);
        renderer->sphereFullSegmentX = renderer->sphereSegmentX + (renderer->sphereSegmentX % chunkWi);
        renderer->sphereFullSegmentY = renderer->sphereSegmentY + (renderer->sphereSegmentY % chunkHi);
        renderer->GenerateFullModel(chunkWi, chunkHi);
    }

    SwitchMode(mode);
}

bool CGameRenderer::Init()
{
    CCursor::SetViewCursur(View, CCursor::Default);

    camera = new CCPanoramaCamera();
    renderer = new CCPanoramaRenderer(this);
    fileManager = new CCFileManager(this);
    uiWapper = new CAppUIWapper(this->View);
    texLoadQueue = new CCTextureLoadQueue();
    uiInfo = new CCGUInfo();

    renderer->Init();
    texLoadQueue->SetLoadHandle(LoadTexCallback, this);
    camera->SetMode(CCPanoramaCameraMode::CenterRoate);
    camera->SetFOVChangedCallback(CameraFOVChanged, this);
    camera->SetRotateCallback(CameraRotate, this);
    camera->Background = CColor::FromString("#FFFFFF");
    fileManager->SetOnCloseCallback(FileCloseCallback, this);

    View->SetCamera(camera);
    View->SetBeforeQuitCallback(BeforeQuitCallback);
    View->SetMouseCallback(MouseCallback);
    View->SetScrollCallback(ScrollCallback);

    LoadSettings();
    SwitchMode(PanoramaMode::PanoramaSphere);

    //renderer->renderPanoramaFullTest = true;
    //renderer->renderPanoramaFullRollTest = true;
    //renderer->renderPanoramaATest = true;
    //TestSplitImageAndLoadTexture();

	return true;
}
void CGameRenderer::Destroy()
{
    destroying = true;
    if (uiInfo != nullptr) {
        delete uiInfo;
        uiInfo = nullptr;
    }
    if (fileManager != nullptr) {
        delete fileManager;
        fileManager = nullptr;
    }
    if (uiWapper != nullptr) {
        delete uiWapper;
        uiWapper = nullptr;
    }
    if (texLoadQueue != nullptr) {
        delete texLoadQueue;
        texLoadQueue = nullptr;
    }
    if (camera != nullptr) {
        View->SetCamera(nullptr);
        delete camera;
        camera = nullptr;
    }
    if (renderer != nullptr) {
        renderer->Destroy();
        delete renderer;
        renderer = nullptr;
    }
}
char* CGameRenderer::GetCurrentPanoramaModeStr()
{
    switch (mode)
    {
    case PanoramaSphere:
        return (char*)u8"����";
    case PanoramaAsteroid:
        return (char*)u8"С����";
    case PanoramaOuterBall:
        return (char*)u8"ˮ����";
    case PanoramaCylinder:
        return (char*)u8"ƽ��";
    case PanoramaMercator:
        return (char*)u8"ȫ��";
    case PanoramaFull360:
        return (char*)u8"360��ȫ��";
    default:
        break;
    }
    return nullptr;
}
void CGameRenderer::Resize(int Width, int Height)
{
    glViewport(0, 0, Width, Height);
}

//���봦��

void CGameRenderer::MouseCallback(COpenGLView* view, float xpos, float ypos, int button, int type) {
    CGameRenderer* renderer = (CGameRenderer*)view->GetRenderer();

    if (type == ViewMouseEventType::ViewMouseMouseDown) {
        if ((button & MK_LBUTTON) == MK_LBUTTON) {
            renderer->lastX = xpos;
            renderer->lastY = ypos;
            view->MouseCapture();
            CCursor::SetViewCursur(view, CCursor::Grab);
        }
    }
    else  if (type == ViewMouseEventType::ViewMouseMouseUp) {
        if ((button & MK_LBUTTON) == MK_LBUTTON) {
            view->ReleaseCapture();
            CCursor::SetViewCursur(view, CCursor::Default);
        }
    }
    else  if (type == ViewMouseEventType::ViewMouseMouseMove) {

        //Skip when mouse hover on window
        if (ImGui::IsAnyWindowHovered()) {
            renderer->main_menu_active = true;
            return;
        }

        if ((button & MK_LBUTTON) == MK_LBUTTON) {//left button down

            renderer->xoffset = xpos - renderer->lastX;
            renderer->yoffset = renderer->lastY - ypos; // reversed since y-coordinates go from bottom to top

            renderer->lastX = xpos;
            renderer->lastY = ypos;

            //��ת����
            if (renderer->mode == PanoramaMode::PanoramaOuterBall || renderer->mode == PanoramaMode::PanoramaSphere
                || renderer->mode == PanoramaMode::PanoramaAsteroid || renderer->mode == PanoramaMode::PanoramaCylinder) {
                float xoffset = -renderer->xoffset * renderer->MouseSensitivity;
                float yoffset = -renderer->yoffset * renderer->MouseSensitivity;
                renderer->renderer->RotateModel(xoffset, yoffset);
            }
            //ȫ��ģʽ�Ǹ���Uƫ�ƺ�γ��ƫ��


        }

        renderer->main_menu_active = ypos < 100 || ypos >  renderer->View->Height - 70;
    }
}
void CGameRenderer::ScrollCallback(COpenGLView* view, float x, float yoffset, int button, int type) {
    CGameRenderer* renderer = (CGameRenderer*)view->GetRenderer();
    if (renderer->mode <= PanoramaMode::PanoramaCylinder) {
        renderer->camera->ProcessMouseScroll(yoffset);
    }
}
void CGameRenderer::KeyMoveCallback(CCameraMovement move) {
    if (mode == PanoramaMode::PanoramaOuterBall || mode == PanoramaMode::PanoramaSphere
        || mode == PanoramaMode::PanoramaAsteroid || mode == PanoramaMode::PanoramaCylinder) {
        switch (move)
        {
        case CCameraMovement::ROATE_UP:
            renderer->mainModel->Rotation.z -= RoateSpeed * View->GetDeltaTime();
            renderer->mainModel->UpdateVectors();
            break;
        case CCameraMovement::ROATE_DOWN:
            renderer->mainModel->Rotation.z += RoateSpeed * View->GetDeltaTime();
            renderer->mainModel->UpdateVectors();
            break;
        case CCameraMovement::ROATE_LEFT:
            renderer->mainModel->Rotation.y -= RoateSpeed * View->GetDeltaTime();
            renderer->mainModel->UpdateVectors();
            break;
        case CCameraMovement::ROATE_RIGHT:
            renderer->mainModel->Rotation.y += RoateSpeed * View->GetDeltaTime();
            renderer->mainModel->UpdateVectors();
            break;
        }
    }
}

//Settings

void CGameRenderer::LoadSettings()
{
    settings = CApp::Instance->GetSettings();

    View->ShowInfoOverlay = settings->GetSettingBool(L"ShowInfoOverlay", false);
    show_console = settings->GetSettingBool(L"ShowConsole", false);
    show_fps = settings->GetSettingBool(L"ShowFps", show_fps);
    show_status_bar = settings->GetSettingBool(L"ShowStatusBar", show_status_bar);
    debug_tool_active = settings->GetSettingBool(L"DebugTool", false);
    renderer->renderDebugWireframe = settings->GetSettingBool(L"renderDebugWireframe", false);
    renderer->renderDebugVector = settings->GetSettingBool(L"renderDebugVector", false);
    renderer->sphereSegmentX = settings->GetSettingInt(L"sphereSegmentX", renderer->sphereSegmentX);
    renderer->sphereSegmentY = settings->GetSettingInt(L"sphereSegmentY", renderer->sphereSegmentY);
    View->IsFullScreen = settings->GetSettingBool(L"FullScreen", false);
    View->Width = settings->GetSettingInt(L"Width", 1024);
    View->Height = settings->GetSettingInt(L"Height", 768);
    if (View->Width <= 800) View->Width = 1024;
    if (View->Height <= 600) View->Height = 768;
    View->Resize(View->Width, View->Height, true);
    View->UpdateFullScreenState();

    UpdateConsoleState();
}
void CGameRenderer::SaveSettings()
{
    settings->SetSettingBool(L"ShowInfoOverlay", View->ShowInfoOverlay);
    settings->SetSettingBool(L"DebugTool", debug_tool_active);
    settings->SetSettingBool(L"ShowConsole", show_console);
    settings->SetSettingBool(L"renderDebugWireframe", renderer->renderDebugWireframe);
    settings->SetSettingBool(L"renderDebugVector", renderer->renderDebugVector);
    settings->SetSettingBool(L"FullScreen", View->IsFullScreen);
    settings->SetSettingInt(L"Width", View->Width);
    settings->SetSettingInt(L"Height", View->Height);
    settings->SetSettingInt(L"sphereSegmentX", renderer->sphereSegmentX);
    settings->SetSettingInt(L"sphereSegmentY", renderer->sphereSegmentY);
    settings->SetSettingBool(L"ShowFps", show_fps);
    settings->SetSettingBool(L"ShowStatusBar", show_status_bar);
}

//����

void CGameRenderer::Render(float FrameTime)
{
    //��Ⱦ
    //===========================

    glLoadIdentity();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    renderer->Render(View->GetDeltaTime());

    glLoadIdentity();

    //����Ⱦ�߳��м�����ͼ
    //===========================

    texLoadQueue->ResolveRender();

    if (needTestImageAndSplit) {
        needTestImageAndSplit = false;
        TestSplitImageAndLoadTexture();
    }

    //loop count
    //===========================

    LoadAndChechRegister();
    if (should_close_file) {
        should_close_file = false;
        std::wstring path = fileManager->CurrentFileLoader->GetPath();
        fileManager->CloseFile();
        if (delete_after_close)
            DeleteFile(path.c_str());
    }
}
void CGameRenderer::RenderUI()
{
    const  ImGuiWindowFlags overlay_window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    const ImGuiIO& io = ImGui::GetIO();

    ui_update_tick += View->GetDeltaTime();
    if (ui_update_tick > 0.5f) ui_update_tick = 0.0f;

    //����
    if (main_menu_active || !View->IsFullScreen) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        //���˵�
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu(u8"�ļ�"))
            {
                if (ImGui::MenuItem(u8"��ȫ��ͼ�ļ�")) fileManager->OpenFile();

                ImGui::Separator();

                if (ImGui::MenuItem(u8"�رյ�ǰ�ļ�", nullptr, nullptr, file_opened)) fileManager->CloseFile();
                if (ImGui::MenuItem(u8"ɾ��", nullptr, nullptr, file_opened)) fileManager->DeleteCurrentFile();
                if (ImGui::MenuItem(u8"ʹ�����������", nullptr, nullptr, file_opened)) fileManager->OpenCurrentFileAs();

                ImGui::Separator();
                if (ImGui::MenuItem(u8"�˳�����", "Alt+F4")) { View->CloseView(); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(u8"��ͼ"))
            {
                if (ImGui::MenuItem((View->IsFullScreen ? u8"�˳�ȫ��" : u8"ȫ��"), "F11", &View->IsFullScreen))
                    View->UpdateFullScreenState();

                if (file_opened) {
                    if (ImGui::BeginMenu(u8"ģʽ"))
                    {
                        if (ImGui::RadioButton(u8"����", mode == PanoramaMode::PanoramaSphere)) SwitchMode(PanoramaMode::PanoramaSphere);
                        if (ImGui::RadioButton(u8"ƽ��", mode == PanoramaMode::PanoramaCylinder))  SwitchMode(PanoramaMode::PanoramaCylinder);
                        if (ImGui::RadioButton(u8"С����", mode == PanoramaMode::PanoramaAsteroid)) SwitchMode(PanoramaMode::PanoramaAsteroid);
                        if (ImGui::RadioButton(u8"ˮ����", mode == PanoramaMode::PanoramaOuterBall))  SwitchMode(PanoramaMode::PanoramaOuterBall);
                        //if (ImGui::RadioButton(u8"ȫ��", mode == PanoramaMode::PanoramaMercator))  SwitchMode(PanoramaMode::PanoramaMercator);
                        //if (ImGui::RadioButton(u8"360ȫ��", mode == PanoramaMode::PanoramaFull360))  SwitchMode(PanoramaMode::PanoramaFull360);
                        ImGui::EndMenu();
                    }
                    if (ImGui::MenuItem(u8"������ͼ", "F10")) renderer->ResetModel();
                }

                ImGui::Separator();

                ImGui::MenuItem(u8"��ʾFPS", "", &show_fps);
                ImGui::MenuItem(u8"״̬��", "", &show_status_bar);

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(u8"����"))
            {
                ImGui::MenuItem(u8"��Ⱦ����", "", &render_dialog_active);

                ImGui::Separator();

                if (ImGui::BeginMenu(u8"����"))
                {
                    ImGui::MenuItem("Debug tool", "", &debug_tool_active);
                    ImGui::MenuItem("Info Overlay", "", &View->ShowInfoOverlay);

                    if (ImGui::BeginMenu(u8"Debug render"))
                    {
                        ImGui::MenuItem("DebugWireframe", "", &renderer->renderDebugWireframe);
                        ImGui::MenuItem("DebugVector", "", &renderer->renderDebugVector);
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem(u8"��ʾ����̨", "", &show_console)) UpdateConsoleState();

                ImGui::EndMenu();
            }  
            if (ImGui::BeginMenu(u8"����"))
            {
                if (ImGui::MenuItem(u8"ʹ�ð���")) {
                    if (!help_dialog_showed) {
                        help_dialog_showed = true;
                        View->SendWindowsMessage(WM_CUSTOM_SHOW_HELPBOX, 0, 0);
                    }
                }
                if (ImGui::MenuItem(u8"����")) {
                    if (!about_dialog_showed) {
                        about_dialog_showed = true;
                        View->SendWindowsMessage(WM_CUSTOM_SHOW_ABOUTBOX, 0, 0);
                    }
                }
                   
                ImGui::EndMenu();
            }

            //FPS��ʾ
            if (show_fps) {
                ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 150.0f, 2.0f));
                ImGui::SetNextWindowSize(ImVec2(150.0f, 23.0f));
                ImGui::SetNextWindowBgAlpha(0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                bool open = ImGui::BeginChild("fps_bar", ImVec2(0, 0), false,
                    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                    ImGuiWindowFlags_NoNav);
                ImGui::PopStyleVar(4);
                if (open)
                {
                    if (ui_update_tick == 0) {
                        current_fps = View->GetCurrentFps();
                        current_draw_time = View->GetDrawTime();
                    }
                    ImGui::Text("FPS: %0.2f (%d ms)", current_fps, current_draw_time);
                    ImGui::EndChild();
                }
            }


            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar(1);
    }

    //����
    if (main_menu_active || !View->IsFullScreen) {

        if (show_status_bar && file_opened) {
            //������
            ImGui::SetNextWindowPos(ImVec2(0.0f, io.DisplaySize.y - 23.0f));
            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 260.0f, 23.0f));
            ImGui::SetNextWindowBgAlpha(0.6f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            bool open = ImGui::Begin("bottom_bar", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
            ImGui::PopStyleVar(4);
            if (open)
            {
                if (uiInfo->currentImageOpened) {

                    switch (uiInfo->currentImageType) {
                    case ImageType::BMP:
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 0.8f, 0.8f));
                        ImGui::Button("BMP"); ImGui::SameLine();
                        ImGui::PopStyleColor(1);
                        break;
                    case ImageType::JPG:
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.2f, 0.8f, 0.8f));
                        ImGui::Button("JPG"); ImGui::SameLine();
                        ImGui::PopStyleColor(1);
                        break;
                    case ImageType::PNG:
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.6f, 0.0f, 0.8f));
                        ImGui::Button("PNG"); ImGui::SameLine();
                        ImGui::PopStyleColor(1);
                        break;
                    }
                    ImGui::Text(uiInfo->currentImageImgSize.c_str()); ImGui::SameLine();
                    ImGui::Text(uiInfo->currentImageSize.c_str()); ImGui::SameLine();
               
                    if (uiInfo->currentImageLoading) {
                        ImGui::Text("|"); ImGui::SameLine();
                        float precent = fileManager->CurrentFileLoader->GetLoadingPrecent();
                        ImGui::Text(u8"������: %d%%", (int)(precent * 100)); ImGui::SameLine();
                    }

                    ImGui::Text("|"); ImGui::SameLine();

                    if (uiInfo->currentImageAllChunks > 0) {
                        ImGui::Text("%d-%d/%d", uiInfo->currentImageLoadChunks, uiInfo->currentImageLoadedChunks, uiInfo->currentImageAllChunks); ImGui::SameLine();
                        ImGui::Text("|"); ImGui::SameLine();
                    }

                    ImGui::Text(u8"%s �޸�����: %s", uiInfo->currentImageName.c_str(), uiInfo->currentImageChangeDate.c_str()); ImGui::SameLine();
                }

                ImGui::End();
            }

            //������
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 260.0f, io.DisplaySize.y - 23.0f));
            ImGui::SetNextWindowSize(ImVec2(260.0f, 23.0f));
            ImGui::SetNextWindowBgAlpha(0.6f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            open = ImGui::Begin("bottom_bar_right", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
            ImGui::PopStyleVar(4);

            if (open)
            {
                //����ģʽ
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
                if (ImGui::Button(GetCurrentPanoramaModeStr()))
                    ImGui::OpenPopup("mode_popup");
                else if (!ImGui::IsItemActivated() && ImGui::IsItemHovered())
                    ImGui::SetTooltip(u8"����ȫ��ģʽ");
                if (ImGui::BeginPopup("mode_popup"))
                {
                    if (ImGui::RadioButton(u8"����", mode == PanoramaMode::PanoramaSphere)) {
                        SwitchMode(PanoramaMode::PanoramaSphere);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::RadioButton(u8"ƽ��", mode == PanoramaMode::PanoramaCylinder)) {
                        SwitchMode(PanoramaMode::PanoramaCylinder);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::RadioButton(u8"С����", mode == PanoramaMode::PanoramaAsteroid)) {
                        SwitchMode(PanoramaMode::PanoramaAsteroid);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::RadioButton(u8"ˮ����", mode == PanoramaMode::PanoramaOuterBall)) {
                        SwitchMode(PanoramaMode::PanoramaOuterBall);
                        ImGui::CloseCurrentPopup();
                    }
                    /*
                    if (ImGui::RadioButton(u8"ȫ��", mode == PanoramaMode::PanoramaMercator)) {
                        SwitchMode(PanoramaMode::PanoramaMercator);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::RadioButton(u8"360ȫ��", mode == PanoramaMode::PanoramaFull360)) {
                        SwitchMode(PanoramaMode::PanoramaFull360);
                        ImGui::CloseCurrentPopup();
                    }
                    */
                    ImGui::EndMenu();
                }

                ImGui::PopStyleColor(1);
                ImGui::SameLine();

                //ȫ��
                if (ImGui::Button(View->IsFullScreen ? u8"�˳�ȫ��" : u8"ȫ��")) {
                    View->IsFullScreen = !View->IsFullScreen;
                    View->UpdateFullScreenState();
                }
                else if (!ImGui::IsItemActivated() && ImGui::IsItemHovered())
                    ImGui::SetTooltip(View->IsFullScreen ? u8"�˳�ȫ��" : u8"ȫ��");
                ImGui::SameLine();

                //���Ź���
                if (mode <= PanoramaMode::PanoramaCylinder) {
                    if (ImGui::Button(u8"��"))
                        camera->ProcessMouseScroll(-120);
                    else  if (!ImGui::IsItemActivated() && ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"��С");

                    ImGui::SameLine();

                    ImGui::SetNextItemWidth(100.0f);
                    if (ImGui::SliderInt("", &zoom_slider_value, 0, 100, u8"����: %d%%"))
                        camera->SetFOV((1.0f - (zoom_slider_value / 100.0f)) * (camera->FovMax - camera->FovMin) + camera->FovMin);
                    ImGui::SameLine();

                    if (ImGui::Button(u8"��"))
                        camera->ProcessMouseScroll(120);
                    else if (!ImGui::IsItemActivated() && ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"�Ŵ�");

                    ImGui::SameLine();
                }

                ImGui::End();
            }
        }
    }

    //���Թ���
    if (debug_tool_active) {
        ImGui::Begin("Debug Tool", &debug_tool_active, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {

            if (ImGui::BeginMenu(u8"Action"))
            {
                if (ImGui::MenuItem("Reset camera"))camera->Reset();
                if (ImGui::MenuItem("Reset model")) renderer->ResetModel();
                ImGui::Separator();
                if (ImGui::MenuItem("Close")) debug_tool_active = false;
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::SliderFloat("Limit fps", &View->LimitFps, 1.0f, 120.0f);

        ImGui::Separator();

        if (ImGui::BeginTabBar("")) {

            if (ImGui::TabItemButton("Camera")) debug_tool_active_tab = 1;
            if (ImGui::TabItemButton("Model")) debug_tool_active_tab = 2;
            if (ImGui::TabItemButton("ModelControl")) debug_tool_active_tab = 3;
            if (ImGui::TabItemButton("Test")) debug_tool_active_tab = 4;

            ImGui::EndTabBar();
        }

        ImGui::Separator();

        if (debug_tool_active_tab == 1) {

            if (ImGui::SliderFloat3("Camera pos", glm::value_ptr(camera->Position), -5.0f, 5.0f)) camera->ForceUpdate();
            if (ImGui::SliderFloat3("Camera roate", glm::value_ptr(camera->Rotate), -180.0f, 180.0f)) camera->ForceUpdate();
            if (ImGui::SliderFloat("Camera fov", &camera->FiledOfView, 0.0f, 179.0f)) camera->ForceUpdate();

            ImGui::Separator();

            ImGui::SliderFloat("RoateSpeed", &camera->RoateSpeed, 0.0f, 40.0f);
            ImGui::SliderFloat("MovementSpeed", &camera->MovementSpeed, 0.0f, 40.0f);
            ImGui::SliderFloat("MouseSensitivity", &camera->MouseSensitivity, 0.0f, 1.0f);
        }
        else if (debug_tool_active_tab == 2) {
            ImGui::SliderFloat3("Model pos", glm::value_ptr(renderer->mainModel->Positon), -180.0f, 180.0f);
            if (ImGui::SliderFloat3("Model roate", glm::value_ptr(renderer->mainModel->Rotation), -180.0f, 180.0f)) 
                renderer->mainModel->UpdateVectors();
        }
        else if (debug_tool_active_tab == 3) {
        }
        else if (debug_tool_active_tab == 4) {
            ImGui::InputInt("FullTestIndex", &renderer->renderPanoramaFullTestIndex);
            ImGui::Checkbox("Full Test", &renderer->renderPanoramaFullTest);
            ImGui::Checkbox("Full Roll Test", &renderer->renderPanoramaFullRollTest);
            ImGui::Checkbox("Full Test Auto loop", &renderer->renderPanoramaFullTestAutoLoop);
            
            ImGui::Separator();
            ImGui::Checkbox("RenderTest modul", &renderer->renderPanoramaATest);
            ImGui::Separator();
            ImGui::Checkbox("Render full", &renderer->renderPanoramaFull);
            ImGui::Checkbox("No Panorama Small", &renderer->renderNoPanoramaSmall);
            
        }
        ImGui::End();
    }

    //Glinfo
    if (glinfo_dialog_active) {
        ImGui::SetNextWindowSize(ImVec2(280, 140));
        if (ImGui::Begin("OpnGL Info", 0, ImGuiWindowFlags_NoResize)) {
            ImGui::Text(u8"glVendor : %s", renderer->globalRenderInfo->glVendor);
            ImGui::Text(u8"glRenderer : %s", renderer->globalRenderInfo->glRenderer);
            ImGui::Text(u8"glVersion : %s", renderer->globalRenderInfo->glVersion);
            ImGui::Text(u8"glslVersion : %s", renderer->globalRenderInfo->glslVersion);

            ImGui::Spacing();
            if (ImGui::Button(u8"�õ�")) glinfo_dialog_active = false;
            ImGui::End();
        }
    }
    if (render_dialog_active) {
        if (ImGui::Begin(u8"��Ⱦ����", &render_dialog_active)) {

            ImGui::SliderFloat(u8"֡������", &View->LimitFps, 1.0f, 30.0f);
            ImGui::Separator();

            ImGui::SliderInt(u8"���� X  ��ֶ�", &renderer->sphereSegmentX, 10, 90);
            ImGui::SliderInt(u8"���� Y  ��ֶ�", &renderer->sphereSegmentY, 10, 90);
            ImGui::Text(u8"(����ֶ�������Ҫ�����������������Ч)");

            ImGui::Separator();

            ImGui::Text(u8"ϸ���� X  ��ֶΣ�%d", renderer->sphereFullSegmentX);
            ImGui::Text(u8"ϸ���� Y  ��ֶΣ�%d", renderer->sphereFullSegmentY);

            ImGui::Spacing();
            ImGui::Spacing();
            if (ImGui::Button(u8"�õ�"))
                render_dialog_active = false;
            ImGui::SameLine();
            if (ImGui::Button(u8"OpenGL ��Ϣ"))
                glinfo_dialog_active = true;

            ImGui::End();
        }
    }

    //�Ի���  
    if (loading_dialog_active) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::SetNextWindowBgAlpha(0.8f);
        if (ImGui::Begin("loading_bg", 0, overlay_window_flags))
            ImGui::End();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_None, ImVec2(0.5f, 0.5f));
        if (ImGui::Begin("loading_box", 0, overlay_window_flags))
        {
            ImGui::Text(u8"ͼ�������У����Ժ�...");
            ImGui::End();
        }
    }

    //��ӭ�Ի���  
    if (welecome_dialog_active) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_None, ImVec2(0.5f, 0.5f));
        if (ImGui::Begin("welecome_box", 0, overlay_window_flags))
        {
            ImGui::Image((ImTextureID)renderer->uiTitleTex->texture, ImVec2(378, 56));
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::SameLine(97);
            if (ImGui::ImageButton((ImTextureID)renderer->uiOpenButtonTex->texture, ImVec2(184, 56)))
                fileManager->OpenFile();
            ImGui::End();
        }
        ImGui::PopStyleVar(4);
    }
}
void CGameRenderer::Update()
{
    //���ض��д���
    //===========================

    texLoadQueue->ResolveMain();

    if (should_open_file) {
        should_open_file = false;
        DoOpenFile();
    }

    //�������
    //===========================

    if (debug_tool_active) {
        if (View->GetKeyPress(0x57)) //W
            camera->ProcessKeyboard(CCameraMovement::FORWARD, View->GetDeltaTime());
        if (View->GetKeyPress(0x53)) //S
            camera->ProcessKeyboard(CCameraMovement::BACKWARD, View->GetDeltaTime());
        if (View->GetKeyPress(0x41)) //A
            camera->ProcessKeyboard(CCameraMovement::LEFT, View->GetDeltaTime());
        if (View->GetKeyPress(0x44)) //D
            camera->ProcessKeyboard(CCameraMovement::RIGHT, View->GetDeltaTime());
    }

    if (View->GetKeyPress(VK_LEFT)) KeyMoveCallback(CCameraMovement::ROATE_LEFT);
    if (View->GetKeyPress(VK_UP)) KeyMoveCallback(CCameraMovement::ROATE_UP);
    if (View->GetKeyPress(VK_RIGHT)) KeyMoveCallback(CCameraMovement::ROATE_RIGHT);
    if (View->GetKeyPress(VK_DOWN)) KeyMoveCallback(CCameraMovement::ROATE_DOWN);

    if (View->GetKeyDown(VK_ESCAPE)) {//ESC
        if (View->GetIsFullScreen()) View->SetFullScreen(false);
    }
    if (View->GetKeyDown(VK_F11)) { //F11
        View->SetFullScreen(!View->GetIsFullScreen());
    }
    if (View->GetKeyDown(VK_F10)) //F10
        renderer->ResetModel();
}

//�߼�����

TextureLoadQueueDataResult* CGameRenderer::LoadChunkTexCallback(TextureLoadQueueInfo* info, CCTexture* texture) {

    auto imgSize = fileManager->CurrentFileLoader->GetImageSize();
    int chunkW = (int)imgSize.x / renderer->panoramaFullSplitW;
    int chunkH = (int)imgSize.y / renderer->panoramaFullSplitH;
    int chunkX = info->x * chunkW;
    int chunkY = info->y * chunkH;

    uiInfo->currentImageLoading = true;
    uiInfo->currentImageLoadChunks = info->id;
    
    //Load full main tex
    TextureLoadQueueDataResult* result = new TextureLoadQueueDataResult();
    result->buffer = fileManager->CurrentFileLoader->GetImageChunkData(chunkX, chunkY, chunkW, chunkH);
    result->size = fileManager->CurrentFileLoader->GetChunkDataSize();
    result->compoents = fileManager->CurrentFileLoader->GetImageDepth();
    result->width = chunkW;
    result->height = chunkH;
    result->success = true;

    uiInfo->currentImageLoadedChunks++;
    uiInfo->currentImageLoading = false;

    return result;
}
TextureLoadQueueDataResult* CGameRenderer::LoadTexCallback(TextureLoadQueueInfo* info, CCTexture* texture, void* data) {
    CGameRenderer* ptr = (CGameRenderer*)data;
    if (ptr->destroying)
        return nullptr;
    if (info->id == -1) {
        ptr->logger->Log(L"Load main tex: id: -1");
        ptr->uiInfo->currentImageLoading = true;

        //Load full main tex
        TextureLoadQueueDataResult* result = new TextureLoadQueueDataResult();
        result->buffer = ptr->fileManager->CurrentFileLoader->GetAllImageData();
        result->size = ptr->fileManager->CurrentFileLoader->GetFullDataSize();
        result->compoents = ptr->fileManager->CurrentFileLoader->GetImageDepth();
        glm::vec2 size = ptr->fileManager->CurrentFileLoader->GetImageScaledSize();
        result->width = (int)size.x;
        result->height = (int)size.y;
        result->success = true;

        ptr->logger->Log2(L"Load tex buffer: w: %d h: %d (%d)  Buffer Size: %d", (int)size.x, (int)size.y, result->compoents, result->size);
        ptr->loading_dialog_active = false;
        ptr->uiInfo->currentImageLoading = false;

        return result;
    }
    else {
        ptr->logger->Log2(L"Load block tex : x: %d y: %d id: %d", info->x, info->y, info->id);
        return ptr->LoadChunkTexCallback(info, texture);
    }
    return nullptr;
}
void CGameRenderer::FileCloseCallback(void* data) {
    CGameRenderer* ptr = (CGameRenderer*)data;
    ptr->renderer->panoramaThumbnailTex = nullptr;
    ptr->renderer->renderPanoramaFull = false;
    ptr->renderer->ReleaseTexPool();
    ptr->renderer->ReleaseFullModel();
    ptr->renderer->UpdateMainModelTex();
    ptr->uiInfo->currentImageOpened = false;
    ptr->renderer->renderNoPanoramaSmall = true;
    ptr->welecome_dialog_active = true;
    ptr->file_opened = false;
}
void CGameRenderer::CameraFOVChanged(void* data, float fov) {
    CGameRenderer* ptr = (CGameRenderer*)data;
    ptr->zoom_slider_value = (int)((1.0f - (fov - ptr->camera->FovMin) / (ptr->camera->FovMax - ptr->camera->FovMin)) * 100);
    if (ptr->mode == PanoramaSphere || ptr->mode == PanoramaCylinder) {
        ptr->renderer->renderPanoramaFull = ptr->SplitFullImage && fov < 40;
        if(ptr->renderer->renderPanoramaFull) ptr->renderer->UpdateFullChunksVisible();
    }
}
void CGameRenderer::CameraRotate(void* data, CCPanoramaCamera* cam)
{
    CGameRenderer* ptr = (CGameRenderer*)data;
    if (ptr->SplitFullImage) {
    }
}
void CGameRenderer::BeforeQuitCallback(COpenGLView* view) {
    CGameRenderer* renderer = (CGameRenderer*)view->GetRenderer();
    renderer->SaveSettings();
}

void CGameRenderer::AddTextureToQueue(CCTexture* tex, int x, int y, int id) {
    texLoadQueue->Push(tex, x, y, id);
}
void CGameRenderer::SwitchMode(PanoramaMode mode)
{
    this->mode = mode;
    renderer->renderPanoramaFull = false;
    switch (mode)
    {
    case PanoramaMercator:
        camera->SetMode(CCPanoramaCameraMode::Static);
        renderer->ResetModel();
        renderer->renderPanoramaFull = false;
        MouseSensitivity = 0.01f;
        break;
    case PanoramaFull360:
        camera->SetMode(CCPanoramaCameraMode::Static);
        renderer->ResetModel();
        renderer->renderPanoramaFull = false;
        MouseSensitivity = 0.01f;
        break;
    case PanoramaAsteroid:
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->Position.z = 1.0f;
        camera->FiledOfView = 135.0f;
        camera->FovMin = 35.0f;
        camera->FovMax = 135.0f;
        MouseSensitivity = 0.1f;
        break;
    case PanoramaCylinder:
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->Position.z = 0.0f;
        camera->FiledOfView = 70.0f;
        camera->FovMin = 5.0f;
        camera->FovMax = 120.0f;
        renderer->renderPanoramaFull = SplitFullImage && camera->FiledOfView < 30;
        MouseSensitivity = 0.1f;
        break;
    case PanoramaSphere:
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->Position.z = 0.5f;
        camera->FiledOfView = 50.0f;
        camera->FovMin = 5.0f;
        camera->FovMax = 75.0f;
        renderer->renderPanoramaFull = SplitFullImage && camera->FiledOfView < 30;
        MouseSensitivity = 0.1f;
        break;
    case PanoramaOuterBall:
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->FiledOfView = 90.0f;
        camera->Position.z = 1.5f;
        camera->FovMin = 35.0f;
        camera->FovMax = 90.0f;
        renderer->renderPanoramaFull = false;
        MouseSensitivity = 0.1f;
        break;
    default:
        break;
    }
}
void CGameRenderer::UpdateConsoleState() {
    ShowWindow(GetConsoleWindow(), show_console ? SW_SHOW : SW_HIDE);
    if (show_console) View->Active();
}
void CGameRenderer::LoadAndChechRegister() {
    if (!reg_dialog_showed) {
        loopCount += View->GetDeltaTime();
        if (loopCount >= 10.0f) {
            loopCount = 0;
            reg_dialog_showed = true;
            time_t timep;
            struct tm* p;
            time(&timep);
            p = gmtime(&timep);
            int lastDayShowRegCount = settings->GetSettingInt(L"regShowLast", p->tm_mday);
            int todayShowRegCount = 0;
            if (lastDayShowRegCount == p->tm_mday) {
                todayShowRegCount = settings->GetSettingInt(L"regShowCount", 0);
            }
            else 
                settings->SetSettingInt(L"regShowCount", 0);

            if (!settings->GetSettingBool(L"registered", false) && todayShowRegCount < 2) {
                settings->SetSettingInt(L"regShowCount", todayShowRegCount++);
                settings->SetSettingInt(L"regShowLast", p->tm_mday);
                View->SendWindowsMessage(WM_CUSTOM_SHOW_REG, 0, 0);
            }
        }
    }
}