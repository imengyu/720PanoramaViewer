#include "CWindowsGameRenderer.h"
#include "CWindowsOpenGLView.h"
#include "CImageLoader.h"
#include "CCRenderGlobal.h"
#include "CCMaterial.h"
#include "CCursor.h"
#include "CApp.h"
#include "CStringHlp.h"
#include "SettingHlp.h"
#include "PathHelper.h"
#include "imgui/imgui.h"
#include <Shlwapi.h>
#include <time.h>

CWindowsGameRendererInternal::CWindowsGameRendererInternal()
{
    logger = Logger::GetStaticInstance();
}
CWindowsGameRendererInternal::~CWindowsGameRendererInternal()
{
}

void CWindowsGameRendererInternal::SetOpenFilePath(const wchar_t* path)
{
	currentOpenFilePath = path;
}
const wchar_t* CWindowsGameRendererInternal::GetOpenFilePath()
{
    return currentOpenFilePath.c_str();
}
void CWindowsGameRendererInternal::DoOpenFile()
{
    UpdateLoadingState(true);
    if (fileManager->DoOpenFile(currentOpenFilePath.c_str())) {

        /*
        if (fileManager->ImageRatioNotStandard && mode <= PanoramaMode::PanoramaOuterBall) {
            if (uiWapper->ShowConfirmBox(L"看起来这张图片的长宽比不是 2:1，不是标准的720度全景图像，如果要显示此图像，可能会导致图像变形，是否继续？", 
                L"提示", L"", L"", CAppUIMessageBoxIcon::IconWarning) == CAppUIMessageBoxResult::ResultCancel)
            {
                currentImageOpened = false;
                renderer->renderOn = false;
                loading_dialog_active = false;
                return;
            }
        }
        */

        LoadImageInfo();

        //主图
        renderer->panoramaThumbnailTex = texLoadQueue->Push(new CCTexture(), 0, 0, -1);//MainTex
        renderer->panoramaTexPool.push_back(renderer->panoramaThumbnailTex);
        renderer->UpdateMainModelTex();

        //检查是否需要分片并加载
        needTestImageAndSplit = true;
        currentImageOpened = true;
        renderer->renderOn = true;
    }
    else {
        ShowErrorDialog();
        CallFileStatusChangedCallback(false, GAME_FILE_OPEN_FAILED);
    }
}
void CWindowsGameRendererInternal::ShowErrorDialog() {
    currentImageOpened = false;
    renderer->renderOn = false;
    UpdateLoadingState(false);
    uiWapper->MessageBeep(CAppUIMessageBoxIcon::IconWarning);
}
void CWindowsGameRendererInternal::LoadImageInfo() {
    //获取图片信息
    auto loader = fileManager->CurrentFileLoader;
    auto imgSize = loader->GetImageSize();
    auto imgFileInfo = loader->GetImageFileInfo();

    currentImageLoading = true;
    currentImageType = fileManager->CurrenImageType;
    currentImageChangeDate = imgFileInfo->Write;
    currentImageFileSize = CStringHlp::GetFileSizeStringAuto(imgFileInfo->fileSize);
    currentImageResolutionSize = CStringHlp::FormatString(L"%dx%dx%db", (int)imgSize.x, (int)imgSize.y, loader->GetImageDepth());
    currentImageInfoTitle = CStringHlp::FormatString(L"%s (%s, %s)",
        Path::GetFileName(currentOpenFilePath).c_str(),
        currentImageResolutionSize.c_str(), currentImageFileSize.c_str());

    CallSampleEventCallback(GAME_EVENT_IMAGE_INFO_LOADED, 0);

    //jpg文件还可以加载EXIF信息
    if (currentImageType == ImageType::JPG) {
        currentImageHasExifData = true;
        currentImageExifDataLoading = true;
        currentImageExifInfo.clear();
        CreateThread(NULL, NULL, LoadImageInfoThread, this, NULL, NULL);
    }
    else {
        currentImageHasExifData = false;
    }
}
void CWindowsGameRendererInternal::TestSplitImageAndLoadTexture() {
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

        currentImageAllChunks = chunkWi * chunkHi;
        currentImageLoadChunks = 0;
        logger->Log(L"Image use split mode , size: %d, %d", chunkWi, chunkHi);
        renderer->sphereFullSegmentX = renderer->sphereSegmentX + (renderer->sphereSegmentX % chunkWi);
        renderer->sphereFullSegmentY = renderer->sphereSegmentY + (renderer->sphereSegmentY % chunkHi);
        renderer->GenerateFullModel(chunkWi, chunkHi);
    }
    else {
        currentImageAllChunks = 0;
    }

    SwitchMode(mode);
}
void CWindowsGameRendererInternal::LoadImageExifInfo() {

    FILE* fp = nullptr;
    _wfopen_s(&fp, currentOpenFilePath.c_str(), L"rb");
    if (!fp) {
        logger->LogError2(L"Can't open JPEG file : %s.", currentOpenFilePath.c_str());
        currentImageExifDataLoading = false;
        return;
    }
    fseek(fp, 0, SEEK_END);
    unsigned long fsize = ftell(fp);
    rewind(fp);
    unsigned char* buf = new unsigned char[fsize];
    if (fread(buf, 1, fsize, fp) != fsize) {
        logger->LogError2(L"Can't read JPEG file : size : %d", fsize);
        currentImageExifDataLoading = false;
        delete[] buf;
        return;
    }
    fclose(fp);

    int code = currentImageExifInfo.parseFrom(buf, fsize);
    delete[] buf;
    if (!code) {
        logger->LogError2(L"Can't read JPEG EXIF Data : %d", code);
        currentImageExifDataLoading = true;
        return;
    }
    
    CallSampleEventCallback(GAME_EVENT_IMAGE_INFO_LOADED2, 0);
}
DWORD WINAPI CWindowsGameRendererInternal::LoadImageInfoThread(LPVOID lpParam)
{
    auto* view = (CWindowsGameRendererInternal*)lpParam;
    view->LoadImageExifInfo();
    return 0;
}

bool CWindowsGameRendererInternal::Init()
{
    CCursor::SetViewCursur(View, CCursor::Default);

    camera = new CCPanoramaCamera();
    renderer = new CCPanoramaRenderer(this);
    fileManager = new CCFileManager(this);
    uiWapper = new CAppUIWapper(this->View);
    texLoadQueue = new CCTextureLoadQueue();

    renderer->Init();
    texLoadQueue->SetLoadHandle(LoadTexCallback, this);
    camera->SetMode(CCPanoramaCameraMode::CenterRoate);
    camera->SetFOVChangedCallback(CameraFOVChanged, this);
    camera->SetOrthoSizeChangedCallback(CameraOrthoSizeChanged, this);
    camera->SetRotateCallback(CameraRotate, this);
    camera->Background = CColor::FromRGBA(0.9529f, 0.9529f, 0.9529f);
    fileManager->SetOnCloseCallback(FileCloseCallback, this);

    View->SetCamera(camera);
    View->SetBeforeQuitCallback(BeforeQuitCallback);
    View->SetMouseCallback(MouseCallback);
    View->SetScrollCallback(ScrollCallback);

    LoadSettings();
    SwitchMode(mode);

    //renderer->renderPanoramaFullTest = true;
    //renderer->renderPanoramaFullRollTest = true;
    //renderer->renderPanoramaATest = true;
    //TestSplitImageAndLoadTexture();

    render_init_finish = true;
	return true;
}
void CWindowsGameRendererInternal::Destroy()
{
    destroying = true;
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
char* CWindowsGameRendererInternal::GetPanoramaModeStr(PanoramaMode mode)
{
    switch (mode)
    {
    case PanoramaSphere:
        return (char*)u8"球面";
    case PanoramaAsteroid:
        return (char*)u8"小行星";
    case PanoramaOuterBall:
        return (char*)u8"水晶球";
    case PanoramaCylinder:
        return (char*)u8"平面";
    case PanoramaMercator:
        return (char*)u8"全景";
    case PanoramaFull360:
        return (char*)u8"360度全景";
    case PanoramaFullOrginal:
        return (char*)u8"原始图像";
    default:
        break;
    }
    return nullptr;
}
void CWindowsGameRendererInternal::Resize(int Width, int Height)
{
    COpenGLRenderer::Resize(Width, Height);
    glViewport(0, 0, Width, Height);
}

//输入处理

void CWindowsGameRendererInternal::MouseCallback(COpenGLView* view, float xpos, float ypos, int button, int type) {
    auto* renderer = (CWindowsGameRendererInternal*)view->GetRenderer();

    if (type == ViewMouseEventType::ViewMouseMouseDown) {
        renderer->lastMoved = false;
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
        if ((button & MK_RBUTTON) == MK_RBUTTON) {
            if (!renderer->lastMoved)
                renderer->CallSampleEventCallback(GAME_EVENT_SHOW_RIGHT_MENU, 0);
        }
    }
    else  if (type == ViewMouseEventType::ViewMouseMouseMove) {
        renderer->lastMoved = true;

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
            if (renderer->mode <= PanoramaMode::PanoramaOuterBall) {
                float xoffset = -renderer->xoffset * renderer->MouseSensitivity;
                float yoffset = -renderer->yoffset * renderer->MouseSensitivity;
                renderer->renderer->RotateModel(xoffset, yoffset);
            }
            //全景模式是更改U偏移和纬度偏移
            else if(renderer->mode == PanoramaMode::PanoramaMercator) {

            }
            else if (renderer->mode == PanoramaMode::PanoramaFull360 
                || renderer->mode == PanoramaMode::PanoramaFullOrginal) {
                float xoffset = -renderer->xoffset * renderer->MouseSensitivity;
                float yoffset = -renderer->yoffset * renderer->MouseSensitivity;
                renderer->renderer->MoveModel(xoffset, yoffset);
            }
        }

        renderer->main_menu_active = ypos < 100 || ypos >  renderer->View->Height - 70;
    }
}
void CWindowsGameRendererInternal::ScrollCallback(COpenGLView* view, float x, float yoffset, int button, int type) {
    auto* renderer = (CWindowsGameRendererInternal*)view->GetRenderer();
    renderer->camera->ProcessMouseScroll(yoffset);
}
void CWindowsGameRendererInternal::KeyMoveCallback(CCameraMovement move) {
    if (mode <= PanoramaMode::PanoramaOuterBall) {
        switch (move)
        {
        case CCameraMovement::ROATE_UP:
            renderer->RotateModelForce(0, -RoateSpeed * View->GetDeltaTime());
            break;
        case CCameraMovement::ROATE_DOWN:
            renderer->RotateModelForce(0, RoateSpeed * View->GetDeltaTime());
            break;
        case CCameraMovement::ROATE_LEFT:
            renderer->RotateModelForce(-RoateSpeed * View->GetDeltaTime(), 0);
            break;
        case CCameraMovement::ROATE_RIGHT:
            renderer->RotateModelForce(RoateSpeed * View->GetDeltaTime(), 0);
            break;
        }
    }
    else if (mode == PanoramaMode::PanoramaMercator) {

    }
    else if (mode == PanoramaMode::PanoramaFull360 || mode == PanoramaMode::PanoramaFullOrginal) {
        switch (move)
        {
        case CCameraMovement::ROATE_UP:
            renderer->MoveModelForce(0, -MoveSpeed * View->GetDeltaTime());
            break;
        case CCameraMovement::ROATE_DOWN:
            renderer->MoveModelForce(0, MoveSpeed * View->GetDeltaTime());
            break;
        case CCameraMovement::ROATE_LEFT:
            renderer->MoveModelForce(-MoveSpeed * View->GetDeltaTime(), 0);
            break;
        case CCameraMovement::ROATE_RIGHT:
            renderer->MoveModelForce(MoveSpeed * View->GetDeltaTime(), 0);
            break;
        }
    }
}

//Settings

void CWindowsGameRendererInternal::LoadSettings()
{
    settings = AppGetAppInstance()->GetSettings();

    ((CWindowsOpenGLView*)View)->ShowInfoOverlay = settings->GetSettingBool(L"ShowInfoOverlay", false);
    show_console = settings->GetSettingBool(L"ShowConsole", false);
    debug_tool_active = settings->GetSettingBool(L"DebugTool", false);
    renderer->renderDebugWireframe = settings->GetSettingBool(L"renderDebugWireframe", false);
    renderer->renderDebugVector = settings->GetSettingBool(L"renderDebugVector", false);
    renderer->sphereSegmentX = settings->GetSettingInt(L"sphereSegmentX", renderer->sphereSegmentX);
    renderer->sphereSegmentY = settings->GetSettingInt(L"sphereSegmentY", renderer->sphereSegmentY);
    mode = (PanoramaMode)settings->GetSettingInt(L"LastMode", mode);
    //View->IsFullScreen = settings->GetSettingBool(L"FullScreen", false);
    //View->Width = settings->GetSettingInt(L"Width", 1024);
    //View->Height = settings->GetSettingInt(L"Height", 768);
    //if (View->Width <= 800) View->Width = 1024;
    //if (View->Height <= 600) View->Height = 768;
    //View->Resize(View->Width, View->Height, true);
    //View->UpdateFullScreenState();

    UpdateConsoleState();
}
void CWindowsGameRendererInternal::SaveSettings()
{
    settings->SetSettingBool(L"ShowInfoOverlay", ((CWindowsOpenGLView*)View)->ShowInfoOverlay);
    settings->SetSettingBool(L"DebugTool", debug_tool_active);
    settings->SetSettingBool(L"ShowConsole", show_console);
    settings->SetSettingBool(L"renderDebugWireframe", renderer->renderDebugWireframe);
    settings->SetSettingBool(L"renderDebugVector", renderer->renderDebugVector);
    settings->SetSettingBool(L"FullScreen", View->IsFullScreen);
    settings->SetSettingInt(L"Width", View->Width);
    settings->SetSettingInt(L"Height", View->Height);
    settings->SetSettingInt(L"sphereSegmentX", renderer->sphereSegmentX);
    settings->SetSettingInt(L"sphereSegmentY", renderer->sphereSegmentY);
    settings->SetSettingInt(L"LastMode", mode);
}


//绘制

void CWindowsGameRendererInternal::Render(float FrameTime)
{
    //渲染
    //===========================

    glLoadIdentity();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    renderer->Render(View->GetDeltaTime());

    glLoadIdentity();

    //在渲染线程中加载贴图
    //===========================

    texLoadQueue->ResolveRender();

    if (needTestImageAndSplit) {
        needTestImageAndSplit = false;
        TestSplitImageAndLoadTexture();
    }

    //文件操作
    //===========================

    if (should_open_file && render_init_finish) {
        should_open_file = false;
        DoOpenFile();
    }

    if (should_close_file) {

        should_close_file = false;
        std::wstring path = fileManager->CurrentFileLoader->GetPath();
        fileManager->CloseFile();

        if (delete_after_close) {
            bool ok = DeleteFile(path.c_str());
            CallSampleEventCallback(GAME_EVENT_FILE_DELETE_BACK, (void*)ok);
        }
    }
}
void CWindowsGameRendererInternal::RenderUI() 
{
    const  ImGuiWindowFlags overlay_window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    const ImGuiIO& io = ImGui::GetIO();

    ui_update_tick += View->GetDeltaTime();
    if (ui_update_tick > 0.5f) ui_update_tick = 0.0f;

    if (main_menu_active && renderQuitFullScreenButton) {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 150.0f, 2.0f));
        ImGui::SetNextWindowSize(ImVec2(150.0f, 23.0f));
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        bool open = ImGui::Begin("quit_full_bar", nullptr, 
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav);
        ImGui::PopStyleVar(4);
        if (open)
        {
            if (ImGui::Button(u8"退出全屏 (F11)")) {
                CallSampleEventCallback(GAME_EVENT_QUIT_FULLSCREEN, 0);
            }
            ImGui::End();
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

        ImGui::SliderFloat("Limit fps", &((CWindowsOpenGLView*)View)->LimitFps, 1.0f, 120.0f);

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

            if (ImGui::CollapsingHeader("Camera")) {

                if (ImGui::SliderFloat3("Pos", glm::value_ptr(camera->Position), -5.0f, 5.0f)) camera->ForceUpdate();
                if (ImGui::SliderFloat3("Roate", glm::value_ptr(camera->Rotate), -180.0f, 180.0f)) camera->ForceUpdate();
                if (ImGui::SliderFloat("Fov", &camera->FiledOfView, 0.0f, 179.0f)) camera->ForceUpdate();
                ImGui::SliderFloat("OrthographicSize", &camera->OrthographicSize, 0.0f, 10.0f);

                const char* itemsProjection[] = { "Perspective", "Orthographic" };
                ImGui::Combo("Projection", (int*)&camera->Projection, itemsProjection, IM_ARRAYSIZE(itemsProjection));
            }
            if (ImGui::CollapsingHeader("Camera Move")) {

                ImGui::SliderFloat("RoateSpeed", &camera->RoateSpeed, 0.0f, 40.0f);
                ImGui::SliderFloat("MovementSpeed", &camera->MovementSpeed, 0.0f, 40.0f);
                ImGui::SliderFloat("MouseSensitivity", &camera->MouseSensitivity, 0.0f, 1.0f);
            }
        }
        else if (debug_tool_active_tab == 2) {
            ImGui::SliderFloat3("Model pos", glm::value_ptr(renderer->mainModel->Position), -8.0f, 8.0f);
            if (ImGui::SliderFloat3("Model roate", glm::value_ptr(renderer->mainModel->Rotation), -8.0f, 8.0f)) 
                renderer->mainModel->UpdateVectors();
        }
        else if (debug_tool_active_tab == 3) {
            if (ImGui::CollapsingHeader("Flat Normal")) {
                ImGui::SliderFloat2("FlatModelMin", glm::value_ptr(renderer->FlatModelMin), -1.0f, 1.0f);
                ImGui::SliderFloat2("FlatModelMax", glm::value_ptr(renderer->FlatModelMax), -1.0f, 1.0f);
                if (renderer->mainFlatModel && !renderer->mainFlatModel->Material.IsNullptr())
                    ImGui::SliderFloat2("FlatModel Material offest", glm::value_ptr(renderer->mainFlatModel->Material->offest), -1.0f, 1.0f);
            }
            if (ImGui::CollapsingHeader("Flat Mercator")) {
                ImGui::Separator();
                ImGui::InputFloat2("ControlPoint0", glm::value_ptr(renderer->MercatorControlPoint0));
                ImGui::InputFloat2("ControlPoint1", glm::value_ptr(renderer->MercatorControlPoint1));
                ImGui::InputFloat2("ControlPoint2", glm::value_ptr(renderer->MercatorControlPoint2));


                if (ImGui::Button("UpdateMercatorControl")) {
                    renderer->UpdateMercatorControl();
                }
            }
        }
        else if (debug_tool_active_tab == 4) {
            ImGui::InputInt("FullTestIndex", &renderer->renderPanoramaFullTestIndex);
            ImGui::Checkbox("Full Test", &renderer->renderPanoramaFullTest);
            ImGui::Checkbox("Full Roll Test", &renderer->renderPanoramaFullRollTest);
            ImGui::Checkbox("Full Test Auto loop", &renderer->renderPanoramaFullTestAutoLoop);

            ImGui::Separator();
            ImGui::Checkbox("Render On", &renderer->renderOn);

            ImGui::Separator();
            ImGui::Checkbox("Render full", &renderer->renderPanoramaFull);
            ImGui::Checkbox("No Panorama Small", &renderer->renderNoPanoramaSmall);
            ImGui::Checkbox("Render Flat", &renderer->renderPanoramaFlat);

            ImGui::Separator();
            ImGui::Checkbox("RenderTest modul", &renderer->renderPanoramaATest);
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
            if (ImGui::Button(u8"好的")) glinfo_dialog_active = false;
            ImGui::End();
        }
    }
    if (render_dialog_active) {
        if (ImGui::Begin(u8"渲染配置", &render_dialog_active)) {

            ImGui::SliderFloat(u8"帧率限制", &((CWindowsOpenGLView*)View)->LimitFps, 1.0f, 30.0f);
            ImGui::Separator();

            ImGui::SliderInt(u8"球体 X  轴分段", &renderer->sphereSegmentX, 10, 90);
            ImGui::SliderInt(u8"球体 Y  轴分段", &renderer->sphereSegmentY, 10, 90);
            ImGui::Text(u8"(球体分段设置需要重新启动程序才能生效)");

            ImGui::Separator();

            ImGui::Text(u8"细球体 X  轴分段：%d", renderer->sphereFullSegmentX);
            ImGui::Text(u8"细球体 Y  轴分段：%d", renderer->sphereFullSegmentY);

            ImGui::Spacing();
            ImGui::Spacing();
            if (ImGui::Button(u8"好的"))
                render_dialog_active = false;
            ImGui::SameLine();
            if (ImGui::Button(u8"OpenGL 信息"))
                glinfo_dialog_active = true;

            ImGui::End();
        }
    }
}
void CWindowsGameRendererInternal::Update()
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

    if (View->GetKeyDown(VK_ESCAPE)) //ESC
        CallSampleEventCallback(GAME_EVENT_QUIT_FULLSCREEN, 0);
    if (View->GetKeyDown(VK_F11)) //F11
        CallSampleEventCallback(GAME_EVENT_GO_FULLSCREEN, 0);
    if (View->GetKeyDown(VK_F10)) //F10
        SwitchMode(GetMode());
}

//逻辑控制

TextureLoadQueueDataResult* CWindowsGameRendererInternal::LoadChunkTexCallback(TextureLoadQueueInfo* info, CCTexture* texture) {

    if (!currentImageOpened)
        return nullptr;

    auto imgSize = fileManager->CurrentFileLoader->GetImageSize();
    int chunkW = (int)imgSize.x / renderer->panoramaFullSplitW;
    int chunkH = (int)imgSize.y / renderer->panoramaFullSplitH;
    int chunkX = info->x * chunkW;
    int chunkY = info->y * chunkH;

    currentImageLoading = true;
    currentImageLoadChunks = info->id;
    
    //Load full main tex
    TextureLoadQueueDataResult* result = new TextureLoadQueueDataResult();
    result->buffer = fileManager->CurrentFileLoader->GetImageChunkData(chunkX, chunkY, chunkW, chunkH);
    result->size = fileManager->CurrentFileLoader->GetChunkDataSize();
    result->compoents = fileManager->CurrentFileLoader->GetImageDepth();
    result->width = chunkW;
    result->height = chunkH;
    result->success = true;

    currentImageLoadedChunks++;
    currentImageLoading = false;

    return result;
}
TextureLoadQueueDataResult* CWindowsGameRendererInternal::LoadTexCallback(TextureLoadQueueInfo* info, CCTexture* texture, void* data) {
    auto* ptr = (CWindowsGameRendererInternal*)data;
    if (ptr->destroying)
        return nullptr;
    if (info->id == -1) {
        ptr->logger->Log(L"Load main tex: id: -1");
        ptr->currentImageLoading = true;

        //Load full main tex
        TextureLoadQueueDataResult* result = new TextureLoadQueueDataResult();
        result->buffer = ptr->fileManager->CurrentFileLoader->GetAllImageData();
        if (!result->buffer) {
            ptr->fileManager->UpdateLastError();
            ptr->ShowErrorDialog();
            ptr->logger->LogError2(L"Load tex main buffer failed : %s", ptr->fileManager->CurrentFileLoader->GetLastError());
            delete result;
            ptr->CallFileStatusChangedCallback(false, GAME_FILE_OPEN_FAILED);
            return nullptr;
        }

        result->size = ptr->fileManager->CurrentFileLoader->GetFullDataSize();
        result->compoents = ptr->fileManager->CurrentFileLoader->GetImageDepth();
        glm::vec2 size = ptr->fileManager->CurrentFileLoader->GetImageScaledSize();
        result->width = (int)size.x;
        result->height = (int)size.y;
        result->success = true;

        ptr->bestZoom = ptr->camera->FovMin + (result->height > 2048 ? 1 : result->height / 2048.0f) *
            ((ptr->camera->FovMax - ptr->camera->FovMin) / 2);
        ptr->bestOrthoSize = ptr->camera->FovMin + (result->height > 2048 ? 1 : result->height / 2048.0f) *
            ((ptr->camera->OrthoSizeMax - ptr->camera->OrthoSizeMin) / 2);

        ptr->logger->Log(L"Load tex buffer: w: %d h: %d (%d)  Buffer Size: %d", (int)size.x, (int)size.y, result->compoents, result->size);
        ptr->UpdateLoadingState(false);
        ptr->currentImageLoading = false;
        ptr->CallFileStatusChangedCallback(true, GAME_FILE_OK);

        return result;
    }
    else {
        ptr->logger->Log(L"Load block tex : x: %d y: %d id: %d", info->x, info->y, info->id);
        return ptr->LoadChunkTexCallback(info, texture);
    }
    return nullptr;
}
void CWindowsGameRendererInternal::FileCloseCallback(void* data) {
    auto* ptr = (CWindowsGameRendererInternal*)data;
    ptr->renderer->panoramaThumbnailTex = nullptr;
    ptr->renderer->renderPanoramaFull = false;
    ptr->texLoadQueue->ForceCancelLoading();
    ptr->renderer->ReleaseFullModel();
    ptr->renderer->ReleaseTexPool();
    ptr->renderer->UpdateMainModelTex();
    ptr->renderer->renderOn = false;
    ptr->currentImageOpened = false;
    ptr->CallFileStatusChangedCallback(false, GAME_FILE_OK);
}
void CWindowsGameRendererInternal::CameraFOVChanged(void* data, float fov) {
    auto* ptr = (CWindowsGameRendererInternal*)data;
    if (ptr->mode == PanoramaSphere || ptr->mode == PanoramaCylinder) {
        ptr->renderer->renderPanoramaFull = ptr->SplitFullImage && fov < 40;
        if(ptr->renderer->renderPanoramaFull) ptr->renderer->UpdateFullChunksVisible();
    }
}
void CWindowsGameRendererInternal::CameraOrthoSizeChanged(void* data, float fov) {
    auto* ptr = (CWindowsGameRendererInternal*)data;
    ptr->renderer->UpdateFlatModelMinMax(fov);
}

void CWindowsGameRendererInternal::CameraRotate(void* data, CCPanoramaCamera* cam)
{
    auto* ptr = (CWindowsGameRendererInternal*)data;
    if (ptr->SplitFullImage) {
    }
}
void CWindowsGameRendererInternal::BeforeQuitCallback(COpenGLView* view) {
    auto* renderer = (CWindowsGameRendererInternal*)view->GetRenderer();
    renderer->SaveSettings();
}

void CWindowsGameRendererInternal::AddTextureToQueue(CCTexture* tex, int x, int y, int id) {
    texLoadQueue->Push(tex, x, y, id);
}
void CWindowsGameRendererInternal::SwitchMode(PanoramaMode mode)
{
    this->mode = mode;
    renderer->renderPanoramaFull = false;
    switch (mode)
    {
    case PanoramaMercator:
        camera->Projection = CCameraProjection::Orthographic;
        camera->SetMode(CCPanoramaCameraMode::Static);
        camera->Position.z = 0.2f;
        renderer->ResetModel();
        renderer->renderPanoramaFlat = true;
        renderer->renderPanoramaFull = false;
        renderer->renderNoPanoramaSmall = true;
        renderer->renderPanoramaFlatXLoop = false;
        renderer->UpdateMercatorControl();
        MouseSensitivity = 0.001f;
        break;
    case PanoramaFullOrginal:
        camera->Projection = CCameraProjection::Orthographic;
        camera->SetMode(CCPanoramaCameraMode::OrthoZoom);
        renderer->ResetModel();
        renderer->renderPanoramaFull = false;
        renderer->renderPanoramaFlat = true;
        renderer->renderNoPanoramaSmall = true;
        renderer->renderPanoramaFlatXLoop = false;
        renderer->ResetMercatorControl();
        MouseSensitivity = 0.001f;
        break;
    case PanoramaFull360:
        camera->Projection = CCameraProjection::Orthographic;
        camera->SetMode(CCPanoramaCameraMode::OrthoZoom);
        renderer->ResetModel();
        renderer->renderPanoramaFull = false;
        renderer->renderPanoramaFlat = true;
        renderer->renderNoPanoramaSmall = true;
        renderer->renderPanoramaFlatXLoop = true;
        renderer->ResetMercatorControl();
        MouseSensitivity = 0.001f;
        break;
    case PanoramaAsteroid:
        camera->Projection = CCameraProjection::Perspective;
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->Position.z = 1.0f;
        camera->FiledOfView = 135.0f;
        camera->FovMin = 35.0f;
        camera->FovMax = 135.0f;
        MouseSensitivity = 0.1f;
        renderer->renderNoPanoramaSmall = false;
        renderer->renderPanoramaFlat = false;
        break;
    case PanoramaCylinder:
        camera->Projection = CCameraProjection::Perspective;
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->Position.z = 0.0f;
        camera->FiledOfView = 70.0f;
        camera->FovMin = 5.0f;
        camera->FovMax = 120.0f;
        renderer->renderPanoramaFull = SplitFullImage && camera->FiledOfView < 30;
        renderer->renderNoPanoramaSmall = false;
        renderer->renderPanoramaFlat = false;
        MouseSensitivity = 0.1f;
        break;
    case PanoramaSphere:
        camera->Projection = CCameraProjection::Perspective;
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->Position.z = 0.5f;
        camera->FiledOfView = 50.0f;
        camera->FovMin = 5.0f;
        camera->FovMax = 75.0f;
        renderer->renderPanoramaFull = SplitFullImage && camera->FiledOfView < 30;
        renderer->renderNoPanoramaSmall = false;
        renderer->renderPanoramaFlat = false;
        MouseSensitivity = 0.1f;
        break;
    case PanoramaOuterBall:
        camera->Projection = CCameraProjection::Perspective;
        camera->SetMode(CCPanoramaCameraMode::CenterRoate);
        camera->FiledOfView = 90.0f;
        camera->Position.z = 1.5f;
        camera->FovMin = 35.0f;
        camera->FovMax = 90.0f;
        renderer->renderPanoramaFull = false;
        renderer->renderNoPanoramaSmall = false;
        renderer->renderPanoramaFlat = false;
        MouseSensitivity = 0.1f;
        break;
    default:
        break;
    }
}
bool CWindowsGameRendererInternal::ZoomIn() { 
    camera->ProcessMouseScroll(120); 
    return camera->ZoomReachedLimit;
}
bool CWindowsGameRendererInternal::ZoomOut() { 
    camera->ProcessMouseScroll(-120); 
    return camera->ZoomReachedLimit;
}
void CWindowsGameRendererInternal::ZoomReset() {
    if (camera->Mode == CCPanoramaCameraMode::CenterRoate)
        camera->FiledOfView = camera->FovMin;
    else if (camera->Mode == CCPanoramaCameraMode::OrthoZoom)
        camera->OrthographicSize = camera->OrthoSizeMin;
}
void CWindowsGameRendererInternal::ZoomBest() { 
    if (camera->Mode == CCPanoramaCameraMode::CenterRoate)
        camera->FiledOfView = bestZoom;
    else if (camera->Mode == CCPanoramaCameraMode::OrthoZoom)
        camera->OrthographicSize = bestOrthoSize;
}

const wchar_t* CWindowsGameRendererInternal::GetCurrentFileInfo(int c) {
    switch (c)
    {
    case GAME_IMAGE_INFO_TYPE: {
        switch (currentImageType)
        {
        case BMP: return L"BMP";
        case JPG: return L"JPG";
        case PNG: return L"PNG";
        case Unknow:
        default: return L"未知";
        }
        break;
    }
    case GAME_IMAGE_INFO_DATE:
        return currentImageChangeDate.c_str();
    case GAME_IMAGE_INFO_SHOOTING_DATE:
        currentImageInfoDateTime = CStringHlp::AnsiToUnicode(currentImageExifInfo.DateTime);
        return currentImageInfoDateTime.c_str();
    case GAME_IMAGE_INFO_SHUTTER_TIME:
        currentImageInfoShutterSpeedValue = currentImageExifInfo.ShutterSpeedValue > 0 ?
            CStringHlp::FormatString(L"%.2f", (float)(1 / currentImageExifInfo.ShutterSpeedValue)) : L"0";
        return currentImageInfoShutterSpeedValue.c_str();
    case GAME_IMAGE_INFO_EXPOSURE_BIAS_VALUE:
        currentImageInfoExposureBiasValue = CStringHlp::FormatString(L"%.2f", currentImageExifInfo.ExposureBiasValue);
        return currentImageInfoExposureBiasValue.c_str();
    case GAME_IMAGE_INFO_ISO_SENSITIVITY:
        currentImageInfoISOSpeedRatings = CStringHlp::FormatString(L"%d", currentImageExifInfo.ISOSpeedRatings);
        return currentImageInfoISOSpeedRatings.c_str();
    case GAME_IMAGE_INFO_FILE_SIZE:
        return currentImageFileSize.c_str();
    case GAME_IMAGE_INFO_RESOLUTION: {
        currentImageInfoResolution = currentImageResolutionSize;
        currentImageInfoResolution += L"  (";
        currentImageInfoResolution += GetCurrentFileInfo(GAME_IMAGE_INFO_TYPE);
        currentImageInfoResolution += L")";
        return currentImageInfoResolution.c_str();
    }
    case GAME_IMAGE_INFO_CAMERA:
        currentImageInfoCamera = CStringHlp::FormatString(L"%hs (%hs)", currentImageExifInfo.Model.c_str(), currentImageExifInfo.Make.c_str());
        return currentImageInfoCamera.c_str();
    case GAME_IMAGE_INFO_FOCAL_LENGTH:
        currentImageInfoFocalLength = CStringHlp::FormatString(L"%d  (35mm : %d)", currentImageExifInfo.FocalLength, currentImageExifInfo.FocalLengthIn35mm);
        return currentImageInfoFocalLength.c_str();
    }
    return nullptr;
}
const wchar_t* CWindowsGameRendererInternal::GetCurrentFileInfoTitle() { 
    return currentImageInfoTitle.c_str();
}
const wchar_t* CWindowsGameRendererInternal::GetCurrentFileLoadingPrecent() { return fileManager->GetCurrentFileLoadingPrecent(); }

void CWindowsGameRendererInternal::UpdateConsoleState() {
    ShowWindow(GetConsoleWindow(), show_console ? SW_SHOW : SW_HIDE);
    if (show_console) View->Active();
}
void CWindowsGameRendererInternal::UpdateLoadingState(bool loading)
{
    CallSampleEventCallback(GAME_EVENT_LOADING_STATUS, (void*)loading);
}

//回调设置
//****************************************************************

void CWindowsGameRendererInternal::SetFileStatusChangedCallback(CGameFileStatusChangedCallback callback, void* data)
{
    fileStatusChangedCallbackData.callback = callback;
    fileStatusChangedCallbackData.data = data;
}
void CWindowsGameRendererInternal::SetSampleEventCallback(CGameSampleEventCallback callback, void* data)
{
    sampleEventCallbackData.callback = callback;
    sampleEventCallbackData.data = data;
}

//回调处理
//****************************************************************

void ThreadSampleEventCallback(void* ptr) {
    auto* a = (SampleEventCallbackData*)ptr;
    a->callback(a->data, a->eventCode, a->param);
    delete a;
}
void ThreadFileStatusChangedCallback(void* ptr) {
    auto* a = (FileStatusChangedCallbackData*)ptr;
    a->callback(a->data, a->isOpen, a->status);
}

void CWindowsGameRendererInternal::CallSampleEventCallback(int code, void* param)
{
    if (sampleEventCallbackData.callback) {
        SampleEventCallbackData* dat = new SampleEventCallbackData();
        dat->eventCode  = code;
        dat->param = param;
        dat->callback = sampleEventCallbackData.callback;
        dat->data = sampleEventCallbackData.data;
        AppGetAppInstance()->GetMessageCenter()->RunOnUIThread(dat, ThreadSampleEventCallback);
    }
}
void CWindowsGameRendererInternal::CallFileStatusChangedCallback(bool isOpen, int status)
{
    if (fileStatusChangedCallbackData.callback) {
        fileStatusChangedCallbackData.isOpen = isOpen;
        fileStatusChangedCallbackData.status = status;
        AppGetAppInstance()->GetMessageCenter()->RunOnUIThread(&fileStatusChangedCallbackData, ThreadFileStatusChangedCallback);
    }
}
