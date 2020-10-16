#include "CGameRenderer.h"
#include "COpenGLView.h"
#include "CImageLoader.h"
#include "CCursor.h"
#include "CApp.h"
#include "StringHlp.h"
#include "SettingHlp.h"
#include <Shlwapi.h>


CGameRenderer::CGameRenderer()
{
    logger = CApp::Instance->GetLogger();
}
CGameRenderer::~CGameRenderer()
{
    if (message_dialog_message != nullptr) {
        delete message_dialog_message;
        message_dialog_message = nullptr;
    }
}

void CGameRenderer::SetOpenFilePath(const wchar_t* path)
{
	currentOpenFilePath = path;
}
void CGameRenderer::DoOpenFile()
{
}

bool CGameRenderer::Init()
{
    CCursor::SetViewCursur(View, CCursor::Default);

    LoadSettings();

    camera = new CCPanoramaCamera();
    renderer = new CCPanoramaRenderer(this);
    mainModel = new CCModel();
    fileManager = new CFileManager(this);

    renderer->Init();

    View->SetCamera(camera);
    View->SetMouseCallback(MouseCallback);
    View->SetScrollCallback(ScrollCallback);

    SwitchMode(PanoramaMode::PanoramaSphere);

	return true;
}
void CGameRenderer::Destroy()
{
    SaveSettings();

    renderer->Destroy();

    if (camera != nullptr) {
        delete camera;
        camera = nullptr;
    }
    if (renderer != nullptr) {
        delete renderer;
        renderer = nullptr;
    }
    if (mainModel != nullptr) {
        delete mainModel;
        mainModel = nullptr;
    }    
    if (fileManager != nullptr) {
        delete fileManager;
        fileManager = nullptr;
    }
}
void CGameRenderer::Resize(int Width, int Height)
{
    glViewport(0, 0, Width, Height);
}

//输入处理

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

            //旋转球体
            if (renderer->mode == PanoramaMode::PanoramaOuterBall || renderer->mode == PanoramaMode::PanoramaSphere
                || renderer->mode == PanoramaMode::PanoramaAsteroid || renderer->mode == PanoramaMode::PanoramaCylinder) {
                float xoffset = -renderer->xoffset * renderer->MouseSensitivity;
                float yoffset = -renderer->yoffset * renderer->MouseSensitivity;
                renderer->mainModel->Rotation.y += xoffset;
                renderer->mainModel->Rotation.z -= yoffset;
                renderer->mainModel->UpdateVectors();
            }
            //全景模式是更改U偏移和纬度偏移


        }

        renderer->main_menu_active = ypos < 100;
    }
}
void CGameRenderer::ScrollCallback(COpenGLView* view, float x, float yoffset, int button, int type) {
    CGameRenderer* renderer = (CGameRenderer*)view->GetRenderer();
    if (renderer->mode == PanoramaMode::PanoramaOuterBall || renderer->mode == PanoramaMode::PanoramaSphere
        || renderer->mode == PanoramaMode::PanoramaAsteroid || renderer->mode == PanoramaMode::PanoramaCylinder) {
        renderer->camera->ProcessMouseScroll(yoffset);
    }
}
void CGameRenderer::KeyMoveCallback(CCameraMovement move) {
    if (mode == PanoramaMode::PanoramaOuterBall || mode == PanoramaMode::PanoramaSphere
        || mode == PanoramaMode::PanoramaAsteroid || mode == PanoramaMode::PanoramaCylinder) {
        switch (move)
        {
        case CCameraMovement::ROATE_UP:
            mainModel->Rotation.z -= RoateSpeed * View->GetDeltaTime();
            mainModel->UpdateVectors();
            break;
        case CCameraMovement::ROATE_DOWN:
            mainModel->Rotation.z += RoateSpeed * View->GetDeltaTime();
            mainModel->UpdateVectors();
            break;
        case CCameraMovement::ROATE_LEFT:
            mainModel->Rotation.y -= RoateSpeed * View->GetDeltaTime();
            mainModel->UpdateVectors();
            break;
        case CCameraMovement::ROATE_RIGHT:
            mainModel->Rotation.y += RoateSpeed * View->GetDeltaTime();
            mainModel->UpdateVectors();
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
    debug_tool_active = settings->GetSettingBool(L"DebugTool", false);
    PolygonMode = settings->GetSettingBool(L"PolygonMode", false);
    DrawVector = settings->GetSettingBool(L"DrawVector", false);
    View->IsFullScreen = settings->GetSettingBool(L"FullScreen", false);
    View->Width = settings->GetSettingInt(L"Width", 1024);
    View->Height = settings->GetSettingInt(L"Height", 768);
    View->UpdateFullScreenState();
    if (View->Width <= 800) View->Width = 1024;
    if (View->Height <= 600) View->Height = 768;
    View->Resize(View->Width, View->Height, true);
    //UpdateConsoleState();
}
void CGameRenderer::SaveSettings()
{
    settings->SetSettingBool(L"ShowInfoOverlay", View->ShowInfoOverlay);
    settings->SetSettingBool(L"DebugTool", debug_tool_active);
    settings->SetSettingBool(L"ShowConsole", show_console);
    settings->SetSettingBool(L"PolygonMode", PolygonMode);
    settings->SetSettingBool(L"DrawVector", DrawVector);
    settings->SetSettingBool(L"FullScreen", View->IsFullScreen);
    settings->SetSettingInt(L"Width", View->Width);
    settings->SetSettingInt(L"Height", View->Height);
}

//绘制

void CGameRenderer::Render(float FrameTime)
{
    glLoadIdentity();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    renderer->Render();

    //绘制向量标线
    if (DrawVector) {

        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(useColorLoc, 1);
        glUniform3f(ourColorLoc, 1.0f, 0.0f, 0.0f);

        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(Up.x, Up.y, Up.z);
        glEnd();

        glUniform3f(ourColorLoc, 0.0f, 1.0f, 0.0f);

        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(Front.x, Front.y, Front.z);
        glEnd();

        glUniform3f(ourColorLoc, 0.0f, 0.0f, 1.0f);

        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(Right.x, Right.y, Right.z);
        glEnd();
    }

    glLoadIdentity();
}
void CGameRenderer::RenderUI()
{
    const  ImGuiWindowFlags overlay_window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    const ImGuiIO& io = ImGui::GetIO();

    //主菜单
    if (main_menu_active || !View->IsFullScreen) {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu(u8"文件"))
            {
                if (ImGui::MenuItem(u8"打开全景图文件")) fileManager->OpenFile();
                if (ImGui::MenuItem(u8"关闭当前文件")) fileManager->CloseFile();
                
                ImGui::Separator();
                if (ImGui::MenuItem(u8"退出程序")) { View->CloseView(); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(u8"设置"))
            {
                if (ImGui::BeginMenu(u8"调试"))
                {
                    ImGui::MenuItem("Debug tool", "", &debug_tool_active);
                    ImGui::MenuItem("Info Overlay", "", &View->ShowInfoOverlay);
                    ImGui::MenuItem("PolygonMode", "", &PolygonMode);
                    ImGui::MenuItem("DrawVector", "", &DrawVector);
                    ImGui::EndMenu();
                }

                ImGui::Separator();

                if (ImGui::MenuItem((View->IsFullScreen ? u8"退出全屏" : u8"全屏"), "F11", &View->IsFullScreen))
                    View->UpdateFullScreenState();

                if (ImGui::MenuItem(u8"显示控制台", "", &show_console))
                    UpdateConsoleState();

                ImGui::MenuItem(u8"渲染配置", "", &render_dialog_active);

                ImGui::EndMenu();
            }  
            if (ImGui::BeginMenu(u8"模式"))
            {
                if (ImGui::RadioButton(u8"球面", mode == PanoramaMode::PanoramaSphere)) SwitchMode(PanoramaMode::PanoramaSphere);
                if (ImGui::RadioButton(u8"平面", mode == PanoramaMode::PanoramaCylinder))  SwitchMode(PanoramaMode::PanoramaCylinder);
                if (ImGui::RadioButton(u8"小行星", mode == PanoramaMode::PanoramaAsteroid)) SwitchMode(PanoramaMode::PanoramaAsteroid);
                if (ImGui::RadioButton(u8"水晶球", mode == PanoramaMode::PanoramaOuterBall))  SwitchMode(PanoramaMode::PanoramaOuterBall);
                //if (ImGui::RadioButton(u8"全景", mode == PanoramaMode::PanoramaMercator))  SwitchMode(PanoramaMode::PanoramaMercator);
                //if (ImGui::RadioButton(u8"360全景", mode == PanoramaMode::PanoramaFull360))  SwitchMode(PanoramaMode::PanoramaFull360);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(u8"帮助"))
            {
                if (ImGui::MenuItem(u8"使用帮助")) {
                    if (!help_dialog_showed) {
                        help_dialog_showed = true;
                        View->SendMessage(WM_CUSTOM_SHOW_HELPBOX, 0, 0);
                    }
                }
                if (ImGui::MenuItem(u8"关于")) about_dialog_active = true;
                   
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    //调试工具
    if (debug_tool_active) {
        ImGui::Begin("Debug Tool", &debug_tool_active, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {

            if (ImGui::BeginMenu(u8"Action"))
            {
                if (ImGui::MenuItem("Reset camera"))camera->Reset();
                if (ImGui::MenuItem("Reset model")) mainModel->Reset();
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
            if (ImGui::SliderFloat("Camera fiv", &camera->FiledOfView, 0.0f, 179.0f)) camera->ForceUpdate();

            ImGui::Separator();

            ImGui::SliderFloat("RoateSpeed", &camera->RoateSpeed, 0.0f, 40.0f);
            ImGui::SliderFloat("MovementSpeed", &camera->MovementSpeed, 0.0f, 40.0f);
            ImGui::SliderFloat("MouseSensitivity", &camera->MouseSensitivity, 0.0f, 1.0f);
        }
        else if (debug_tool_active_tab == 2) {
            ImGui::SliderFloat3("Model pos", glm::value_ptr(mainModel->Positon), -180.0f, 180.0f);
            if (ImGui::SliderFloat3("Model roate", glm::value_ptr(mainModel->Rotation), -180.0f, 180.0f)) mainModel->UpdateVectors();
        }
        else if (debug_tool_active_tab == 3) {
        }
        else if (debug_tool_active_tab == 4) {
            ImGui::Separator();
        }
        ImGui::End();
    }

    //对话框
    if (!fileOpened && !currentFileLoading) {
        ImGui::SetNextWindowBgAlpha(0.55f); // Transparent background
        ImGui::SetNextWindowSize(ImVec2(300, 100));
        ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - 300) / 2.0f, (io.DisplaySize.y - 100) / 2.0f));
        if (ImGui::Begin("welecome_overlay", &fileOpened, overlay_window_flags))
        {
            ImGui::Text(u8"欢迎使用 720 Image Viewer\n请先打开一个全景图文件");
            ImGui::Separator();
            for (int i = 0; i < 6; i++)
                ImGui::Spacing();
            if (ImGui::Button(u8"打开", ImVec2(284, 20)))
                openFile();     
            ImGui::End();
        }
    }
    if (about_dialog_active) {

        ImGui::SetNextWindowSize(ImVec2(330, 300));
        ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - 330) / 2.0f, (io.DisplaySize.y - 300) / 2.0f));

        if (ImGui::Begin(u8"关于 720 Image Viewer", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse)) {
         
            ImGui::Text(u8"720 Image Viewer");
            ImGui::Separator();
            ImGui::Image((ImTextureID)texture_logo,  ImVec2(texture_logo_size.x, texture_logo_size.y));
            ImGui::Separator();
            ImGui::Text(u8"这是一个简单的 720 全景图查看软件。");
            ImGui::Text(u8"支持多种投影方式，可快速打开浏览您的 720 全景图。");
            ImGui::Spacing();
            ImGui::Spacing();            ImGui::Spacing();
            ImGui::Separator();

            ImGui::SameLine();
            if (ImGui::Button(u8"程序信息") && !about_dialog_showed) {
                about_dialog_showed = true;
                View->SendMessage(WM_CUSTOM_SHOW_ABOUTBOX, 0, 0);
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"OpenGL 信息"))
                glinfo_dialog_active = true;
            ImGui::SameLine();
            if (ImGui::Button(u8"好的")) about_dialog_active = false;

            ImGui::End();
        }
    }
    //Glingo
    if (glinfo_dialog_active) {
        ImGui::SetNextWindowSize(ImVec2(280, 140));
        if (ImGui::Begin("OpnGL Info", 0, ImGuiWindowFlags_NoResize)) {
            ImGui::Text(u8"glVendor : %s", glVendor);
            ImGui::Text(u8"glRenderer : %s", glRenderer);
            ImGui::Text(u8"glVersion : %s", glVersion);
            ImGui::Text(u8"glslVersion : %s", glslVersion);
            if (ImGui::Button(u8"OK")) glinfo_dialog_active = false;
            ImGui::End();
        }
    }
    if (render_dialog_active) {
        if (ImGui::Begin(u8"渲染配置", &render_dialog_active)) {

            ImGui::SliderInt(u8"球体 X  轴分段", &preferImageXSeg, 10, 90);
            ImGui::SliderInt(u8"球体 Y  轴分段", &preferImageYSeg, 10, 90);

            ImGui::Text(u8"当前X 轴分段: %d", currentImageXSeg);
            ImGui::Text(u8"当前Y 轴分段: %d", currentImageYSeg);

            if (ImGui::Button(u8"好的")) glinfo_dialog_active = false;

            ImGui::End();
        }
    }

    //对话框  
    if (loading_dialog_active) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::SetNextWindowBgAlpha(0.6f);
        if (ImGui::Begin("loading_bg", 0, overlay_window_flags))
            ImGui::End();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        if (ImGui::Begin("loading_box", 0, overlay_window_flags))
        {
            ImGui::Text(u8"图像载入中，请稍后...");
            ImGui::End();
        }
    }
    if (message_dialog_active) {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        if (ImGui::Begin(u8"提示", &message_dialog_active, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Text(message_dialog_message);
            ImGui::Separator();
            if (ImGui::Button(u8"确定")) message_dialog_active = false;
            ImGui::End();
        }
    }
}
void CGameRenderer::Update()
{



    //按键检测
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
}

void CGameRenderer::SwitchMode(PanoramaMode mode)
{
    this->mode = mode;
    switch (mode)
    {
    case PanoramaMercator:
        camera->SetMode(CCPanoramaCameraMode::Static);
        mainModel->Reset();
        MouseSensitivity = 0.01f;
        break;
    case PanoramaAsteroid:
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->Position.z = 1.0f;
        camera->FiledOfView = 135.0f;
        camera->FovMin = 75.0f;
        camera->FovMax = 135.0f;
        MouseSensitivity = 0.1f;
        break;
    case PanoramaCylinder:
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->Position.z = 0.0f;
        camera->FiledOfView = 70.0f;
        camera->FovMin = 30.0f;
        camera->FovMax = 120.0f;
        MouseSensitivity = 0.1f;
        break;
    case PanoramaSphere:
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->Position.z = 0.5f;
        camera->FiledOfView = 50.0f;
        camera->FovMin = 30.0f;
        camera->FovMax = 75.0f;
        MouseSensitivity = 0.1f;
        break;
    case PanoramaOuterBall:
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->FiledOfView = 90.0f;
        camera->Position.z = 1.5f;
        camera->FovMin = 45.0f;
        camera->FovMax = 90.0f;
        MouseSensitivity = 0.1f;
        break;
    default:
        break;
    }
}
void CGameRenderer::UpdateConsoleState() {
    ShowWindow(GetConsoleWindow(), show_console ? SW_SHOW : SW_HIDE);
}