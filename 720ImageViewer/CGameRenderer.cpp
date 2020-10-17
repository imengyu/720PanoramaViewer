#include "CGameRenderer.h"
#include "COpenGLView.h"
#include "CImageLoader.h"
#include "CCRenderGlobal.h"
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
}

void CGameRenderer::SetOpenFilePath(const wchar_t* path)
{
	currentOpenFilePath = path;
}
void CGameRenderer::DoOpenFile()
{
    loading_dialog_active = true;
    if (fileManager->DoOpenFile(currentOpenFilePath.c_str())) {
        //主图
        renderer->panoramaThumbnailTex = texLoadQueue->Push(new CCTexture(), 0, 0, -1);//MainTex
        renderer->panoramaTexPool.push_back(renderer->panoramaThumbnailTex);

        //检查是否需要分片并加载
        TestSplitImageAndLoadTexture();
    }
    else {
        loading_dialog_active = false;
        uiWapper->ShowMessageBox(fileManager->GetLastError(), L"打开文件失败", IconWarning);
    }
}
void CGameRenderer::TestSplitImageAndLoadTexture() {

}

bool CGameRenderer::Init()
{
    CCursor::SetViewCursur(View, CCursor::Default);

    camera = new CCPanoramaCamera();
    renderer = new CCPanoramaRenderer(this);
    fileManager = new CCFileManager(this);
    uiWapper = new CAppUIWapper(this->View);
    texLoadQueue = new CCTextureLoadQueue();

    renderer->Init();
    texLoadQueue->SetLoadHandle(LoadTexCallback, this);
    camera->SetFOVChangedCallback(CameraFOVChanged, this);
    camera->Background = CColor::FromString("#4682B4");
    fileManager->SetOnCloseCallback(FileCloseCallback, this);

    View->SetShaderProgram(renderer->shaderProgram);
    View->SetCamera(camera);
    View->SetCameraLoc(renderer->globalRenderInfo->viewLoc, renderer->globalRenderInfo->projectionLoc);
    View->SetBeforeQuitCallback(BeforeQuitCallback);
    View->SetMouseCallback(MouseCallback);
    View->SetScrollCallback(ScrollCallback);

    LoadSettings();
    SwitchMode(PanoramaMode::PanoramaSphere);

	return true;
}
void CGameRenderer::Destroy()
{
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
        delete camera;
        camera = nullptr;
        View->SetCamera(nullptr);
    }
    if (renderer != nullptr) {
        renderer->Destroy();
        delete renderer;
        renderer = nullptr;
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
                renderer->renderer->RotateModel(xoffset, yoffset);
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
    debug_tool_active = settings->GetSettingBool(L"DebugTool", false);
    renderer->renderDebugWireframe = settings->GetSettingBool(L"renderDebugWireframe", false);
    renderer->renderDebugVector = settings->GetSettingBool(L"renderDebugVector", false);
    View->IsFullScreen = settings->GetSettingBool(L"FullScreen", false);
    View->Width = settings->GetSettingInt(L"Width", 1024);
    View->Height = settings->GetSettingInt(L"Height", 768);
    View->UpdateFullScreenState();
    if (View->Width <= 800) View->Width = 1024;
    if (View->Height <= 600) View->Height = 768;
    View->Resize(View->Width, View->Height, true);
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
}

//绘制

void CGameRenderer::Render(float FrameTime)
{
    //渲染
    //===========================

    glLoadIdentity();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    renderer->Render();

    glLoadIdentity();

    //在渲染线程中加载贴图
    //===========================

    texLoadQueue->ResolveRender();
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

                    if (ImGui::BeginMenu(u8"Debug render"))
                    {
                        ImGui::MenuItem("DebugWireframe", "", &renderer->renderDebugWireframe);
                        ImGui::MenuItem("DebugVector", "", &renderer->renderDebugVector);
                        ImGui::EndMenu();
                    }
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
            if (ImGui::SliderFloat("Camera fiv", &camera->FiledOfView, 0.0f, 179.0f)) camera->ForceUpdate();

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
            ImGui::Separator();
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
            if (ImGui::Button(u8"OK")) glinfo_dialog_active = false;
            ImGui::End();
        }
    }
    if (render_dialog_active) {
        if (ImGui::Begin(u8"渲染配置", &render_dialog_active)) {

            ImGui::SliderInt(u8"球体 X  轴分段", &renderer->sphereSegmentX, 10, 90);
            ImGui::SliderInt(u8"球体 Y  轴分段", &renderer->sphereSegmentY, 10, 90);

            ImGui::SliderInt(u8"细球体 X  轴分段", &renderer->sphereFullSegmentX, 10, 90);
            ImGui::SliderInt(u8"细球体 Y  轴分段", &renderer->sphereFullSegmentY, 10, 90);

            if (ImGui::Button(u8"好的")) glinfo_dialog_active = false;

            ImGui::End();
        }
    }

    //对话框  
    if (loading_dialog_active) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::SetNextWindowBgAlpha(0.8f);
        if (ImGui::Begin("loading_bg", 0, overlay_window_flags))
            ImGui::End();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        if (ImGui::Begin("loading_box", 0, overlay_window_flags))
        {
            ImGui::Text(u8"图像载入中，请稍后...");
            ImGui::End();
        }
    }
}
void CGameRenderer::Update()
{
    //加载队列处理
    //===========================

    texLoadQueue->ResolveMain();

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

//逻辑控制

TextureLoadQueueDataResult* CGameRenderer::LoadTexCallback(TextureLoadQueueInfo* info, CCTexture* texture, void* data) {
    CGameRenderer* ptr = (CGameRenderer*)data;
    if (info->id == -1) {
        ptr->logger->Log(L"Load main tex: id: -1");

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
        return result;
    }
    else {
        ptr->logger->Log2(L"Load block tex : x: %d y: %d id: %d", info->x, info->y, info->id);


    }
    return nullptr;
}
void CGameRenderer::FileCloseCallback(void* data) {
    CGameRenderer* ptr = (CGameRenderer*)data;
    ptr->renderer->ReleaseTexPool();
}
void CGameRenderer::CameraFOVChanged(void* data, float fov) {
    CGameRenderer* ptr = (CGameRenderer*)data;
    if (ptr->mode == PanoramaSphere || ptr->mode == PanoramaCylinder) {
        ptr->renderer->renderPanoramaFull = fov >= 40;
    }
}
void CGameRenderer::BeforeQuitCallback(COpenGLView* view) {
    CGameRenderer* renderer = (CGameRenderer*)view->GetRenderer();
    renderer->SaveSettings();
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
    if (show_console) View->Active();
}