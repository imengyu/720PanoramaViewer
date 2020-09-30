#include "CGameRenderer.h"
#include "COpenGLView.h"
#include "CImageLoader.h"
#include "CCursor.h"
#include "CApp.h"
#include "StringHlp.h"
#include "SettingHlp.h"
#include <Shlwapi.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

CGameRenderer::CGameRenderer()
{
    logger = CApp::Instance->GetLogger();
    memset(texture_chunks, 0, sizeof(texture_chunks));
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
    ReqFileLoadTicked = true;
}

const char* vertexShaderSource = "\
#version 410 core\n\
in vec4 gl_Vertex;\n\
in vec4 gl_MultiTexCoord0;\n\
out vec2 TexCoord;\n\
\n\
uniform mat4 model;\n\
uniform mat4 view;\n\
uniform mat4 projection;\n\
void main() {\n\
    gl_Position = projection * view * model * gl_Vertex;\n\
    TexCoord = vec2(gl_MultiTexCoord0.st);\n\
}\0";

const char* fragmentShaderSource = "\
#version 410 core\n\
out vec4 FragColor; \n\
in vec2 TexCoord;\n\
uniform sampler2D ourTexture;\n\
uniform bool useColor;\n\
uniform vec3 ourColor;\n\
void main(){\n\
    FragColor = useColor ? vec4(ourColor, 1.0f) : texture(ourTexture, TexCoord);\n\
}\n\0";

const int Y_SEGMENTS = 16;
const int X_SEGMENTS = 32;

bool CGameRenderer::Init()
{
    CCursor::SetViewCursur(View, CCursor::Default);
    LoadSettings();

    glViewport(0, 0, Width, Height);
    glEnable(GL_TEXTURE_2D);
    glVendor = (GLubyte*)glGetString(GL_VENDOR);            //���ظ���ǰOpenGLʵ�ֳ��̵�����
    glRenderer = (GLubyte*)glGetString(GL_RENDERER);    //����һ����Ⱦ����ʶ����ͨ���Ǹ�Ӳ��ƽ̨
    glVersion = (GLubyte*)glGetString(GL_VERSION);    //���ص�ǰOpenGLʵ�ֵİ汾��
    glslVersion = (GLubyte*)glGetString(GL_SHADING_LANGUAGE_VERSION);//������ɫԤѹ�������汾��

    createShader();

    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    ourTextrueLoc = glGetUniformLocation(shaderProgram, "ourTexture");
    useColorLoc = glGetUniformLocation(shaderProgram, "useColor");
    ourColorLoc = glGetUniformLocation(shaderProgram, "ourColor");

    View->SetMouseCallback(MouseCallback);
    View->SetScrollCallback(ScrollCallback);

    SwitchMode(PanoramaMode::Sphere);
    UpdateModelVectors();

	return true;
}

void CGameRenderer::SwitchMode(PanoramaMode mode)
{
    this->mode = mode;
    switch (mode)
    {
    case Flat:
        View->Camera.SetMode(Camera_Mode::Static);
        UVOffest = glm::vec2(0.0f);
        MouseSensitivity = 0.01f;
        resetModel();
        break;
    case Asteroid:
        View->Camera.SetMode(Camera_Mode::CenterRoate);
        View->Camera.Position.z = 1.0f;
        View->Camera.Zoom = 135.0f;
        View->Camera.ZoomMin = 75.0f;
        View->Camera.ZoomMax = 135.0f;
        MouseSensitivity = 0.1f;
        break;
    case Cylinder:
        View->Camera.SetMode(Camera_Mode::CenterRoate);
        View->Camera.Position.z = 0.0f;
        View->Camera.Zoom = 70.0f;
        View->Camera.ZoomMin = 70.0f;
        View->Camera.ZoomMax = 120.0f;
        MouseSensitivity = 0.1f;
        break;
    case Sphere:
        View->Camera.SetMode(Camera_Mode::CenterRoate);
        View->Camera.Position.z = 0.5f;
        View->Camera.Zoom = 50.0f;
        View->Camera.ZoomMin = 30.0f;
        View->Camera.ZoomMax = 75.0f;
        MouseSensitivity = 0.1f;
        break;
    case OuterBall:
        View->Camera.SetMode(Camera_Mode::CenterRoate);
        View->Camera.Zoom = 90.0f;
        View->Camera.Position.z = 1.5f;
        View->Camera.ZoomMin = 45.0f;
        View->Camera.ZoomMax = 90.0f;
        MouseSensitivity = 0.1f;
        break;
    default:
        break;
    }
}

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
            if (renderer->mode == PanoramaMode::OuterBall || renderer->mode == PanoramaMode::Sphere 
                || renderer->mode == PanoramaMode::Asteroid || renderer->mode == PanoramaMode::Cylinder) {
                float xoffset = -renderer->xoffset * renderer->MouseSensitivity;
                float yoffset = -renderer->yoffset * renderer->MouseSensitivity;
                renderer->Rotation.y += xoffset;
                renderer->Rotation.z -= yoffset;
                renderer->UpdateModelVectors();
            }
            //ȫ��ģʽ�Ǹ���Uƫ�ƺ�γ��ƫ��
            else if (renderer->mode == PanoramaMode::Flat) {
                float xoffset = renderer->xoffset * renderer->MouseSensitivity;
                float yoffset = renderer->yoffset * renderer->MouseSensitivity;

                renderer->UVOffest.x += xoffset;
                renderer->UVOffest.y += yoffset;

                if (renderer->UVOffest.x <= -1)renderer->UVOffest.x = 1.0f;
                else if (renderer->UVOffest.x >= 1)renderer->UVOffest.x = -1.0f;
                if (renderer->UVOffest.y <= -renderer->FlatZoom / 2.0f)renderer->UVOffest.y =-renderer->FlatZoom / 2.0f;
                else if (renderer->UVOffest.y >= renderer->FlatZoom / 2.0f) renderer->UVOffest.y = renderer->FlatZoom / 2.0f;

            }

        }

        renderer->main_menu_active = ypos < 100;
    }
}
void CGameRenderer::ScrollCallback(COpenGLView* view, float x, float yoffset, int button, int type) {
    CGameRenderer* renderer = (CGameRenderer*)view->GetRenderer();
    if (renderer->mode == PanoramaMode::Flat)
        renderer->HandleFlatZoom(yoffset);
    else view->Camera.ProcessMouseScroll(yoffset);
}
void CGameRenderer::KeyMoveCallback(Camera_Movement move) {
    if (mode == PanoramaMode::OuterBall || mode == PanoramaMode::Sphere
        || mode == PanoramaMode::Asteroid || mode == PanoramaMode::Cylinder) {
        switch (move)
        {
        case ROATE_UP:
            Rotation.z -= RoateSpeed * View->GetDeltaTime();
            UpdateModelVectors();
            break;
        case ROATE_DOWN:
            Rotation.z += RoateSpeed * View->GetDeltaTime();
            UpdateModelVectors();
            break;
        case ROATE_LEFT:
            Rotation.y -= RoateSpeed * View->GetDeltaTime();
            UpdateModelVectors();
            break;
        case ROATE_RIGHT:
            Rotation.y += RoateSpeed * View->GetDeltaTime();
            UpdateModelVectors();
            break;
        }
    }
    else {
        float xoffset = 0;
        switch (move)
        {
        case ROATE_UP:
            UVOffest.y -= RoateSpeed * View->GetDeltaTime() / 100.0f;
            break;
        case ROATE_DOWN:
            UVOffest.y += RoateSpeed * View->GetDeltaTime() / 100.0f;
            break;
        case ROATE_LEFT:
            UVOffest.x += RoateSpeed * View->GetDeltaTime() / 100.0f;
            break;
        case ROATE_RIGHT:
            UVOffest.x -= RoateSpeed * View->GetDeltaTime() / 100.0f;
            break;
        }
        if (UVOffest.x <= -1) UVOffest.x = 1.0f;
        else if (UVOffest.x >= 1) UVOffest.x = -1.0f;
        if (UVOffest.y <= -1) UVOffest.y = 1.0f;
        else if (UVOffest.y >= 1) UVOffest.y = -1.0f;
    }
}
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
    UpdateConsoleState();
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
void CGameRenderer::ResetCamera() {
    View->Camera.SetMode(View->Camera.Mode);
}
void CGameRenderer::HandleFlatZoom(float yoffset) {
    if (FlatZoom >= FlatZoomMin && FlatZoom <= FlatZoomMax) FlatZoom += yoffset * FlatZoomSpeed;
    if (FlatZoom <= FlatZoomMin) FlatZoom = FlatZoomMin;
    if (FlatZoom >= FlatZoomMax) FlatZoom = FlatZoomMax;
}

void CGameRenderer::Render(float FrameTime)
{
    glClearColor(0.43f, 0.45f, 0.70f, 1.0f);
    glLoadIdentity();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glColor3f(0.8f, 0.2f, 0.7f);

    //���������任
    if (mode == Flat) {
        //����������ģʽ�õ���ƽ���ã����Բ���Ҫ�����
        glm::mat4 view(1.0f);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glm::mat4 projection(1.0f);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    }
    else {
        glm::mat4 view = View->Camera.GetViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        
        glm::mat4 projection = CameraPerspective ?
            glm::perspective(glm::radians(View->Camera.Zoom), (float)View->Width / (float)View->Height, 0.1f, 100.0f)
            : glm::ortho(-(float)View->Width / (float)View->Height, (float)View->Width / (float)View->Height, CameraPerspectiveBottom, CameraPerspectiveTop, 0.1f, 100.0f);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    }

    //ģ�ͱ任
    glm::mat4 model(1.0f);
    model = glm::translate(model, Positon); 
    model = glm::rotate(model, glm::radians(Rotation.x), Right);
    model = glm::rotate(model, glm::radians(Rotation.y), Up);
    model = glm::rotate(model, glm::radians(Rotation.z), Front);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    if (texture_one > 0) {
        glBindTexture(GL_TEXTURE_2D, texture_one);
    }
    if (fileOpened) {
        glUniform1i(useColorLoc, 0);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(useColorLoc, 1);
        glUniform3f(ourColorLoc, 0.43f, 0.45f, 0.70f);
    }

    glUseProgram(shaderProgram);

    //����
    switch (mode)
    {
    case OuterBall:
    case Cylinder:
    case Asteroid:
    case Sphere:  renderSphere(false); break;
    case Flat: renderPanel(false); break;
    }

    //�Ƿ�����߿�
    if (PolygonMode) {

        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(useColorLoc, 1);
        glUniform3f(ourColorLoc, 1.0f, 0.0f, 0.0f);

        switch (mode)
        {
        case OuterBall:
        case Cylinder:
        case Asteroid:
        case Sphere:  renderSphere(true); break;
        case Flat: renderPanel(true); break;
        }
    }
    else {
        if (!fileOpened) {
            renderABox();
        }
    }

    //������������
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

    //����Ⱦ�߳��м�����ͼ��opengl
    if (ReqTexLoadTicked) {
        ReqTexLoadTicked = false;
        currentFileLoading = false;  

        createPanoramaTexture();
        
        loading_dialog_active = false;
    }
    if (ReqLogoTexLoad) {
        ReqLogoTexLoad = false;
        loadLogoTexture();
    }
    if (currentLoadTextureReqLoadToGL) {
        currentLoadTextureReqLoadToGL = false;
        createPanoramaTextureChunk();
    }

    glLoadIdentity();
}
void CGameRenderer::Update()
{
    //�������
    if (debug_tool_active) {
        if (View->GetKeyPress(0x57)) //W
            View->Camera.ProcessKeyboard(FORWARD, View->GetDeltaTime());
        if (View->GetKeyPress(0x53)) //S
            View->Camera.ProcessKeyboard(BACKWARD, View->GetDeltaTime());
        if (View->GetKeyPress(0x41)) //A
            View->Camera.ProcessKeyboard(LEFT, View->GetDeltaTime());
        if (View->GetKeyPress(0x44)) //D
            View->Camera.ProcessKeyboard(RIGHT, View->GetDeltaTime());
    }

    if (View->GetKeyPress(VK_LEFT)) KeyMoveCallback(ROATE_LEFT);
    if (View->GetKeyPress(VK_UP)) KeyMoveCallback(ROATE_UP);
    if (View->GetKeyPress(VK_RIGHT)) KeyMoveCallback(ROATE_RIGHT);
    if (View->GetKeyPress(VK_DOWN)) KeyMoveCallback(ROATE_DOWN);

    if (View->GetKeyDown(VK_ESCAPE)) {//ESC
        if(View->GetIsFullScreen()) View->SetFullScreen(false);
    }
    if (View->GetKeyDown(VK_F11)) { //F11
        View->SetFullScreen(!View->GetIsFullScreen());
    }

    if (!fileOpened && !PolygonMode) {
        Rotation.x += 2.0f * View->GetDeltaTime();
        Rotation.z += 2.0f * View->GetDeltaTime();
        Rotation.y += 2.0f * View->GetDeltaTime();
        UpdateModelVectors();
    }

    //�����ļ�
    if (ReqFileLoadTicked) {
        ReqFileLoadTicked = false;
        doOpenFile();
    }
    //���������ͼ
    if (currentLoadTextureReqNext) {
        loadPanoramaTextureChunk();
        currentLoadTextureReqNext = false;
    }
}
void CGameRenderer::RenderUI()
{
    const  ImGuiWindowFlags overlay_window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    const ImGuiIO& io = ImGui::GetIO();

    //���˵�
    if (main_menu_active || !View->IsFullScreen) {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu(u8"�ļ�"))
            {
                if (ImGui::MenuItem(u8"��ȫ��ͼ�ļ�")) openFile();
                if (ImGui::MenuItem(u8"�رյ�ǰ�ļ�")) closeFile();
                
                ImGui::Separator();
                if (ImGui::MenuItem(u8"�˳�����")) { View->CloseView(); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(u8"����"))
            {
                if (ImGui::BeginMenu(u8"����"))
                {
                    ImGui::MenuItem("Debug tool", "", &debug_tool_active);
                    ImGui::MenuItem("Info Overlay", "", &View->ShowInfoOverlay);
                    ImGui::MenuItem("PolygonMode", "", &PolygonMode);
                    ImGui::MenuItem("DrawVector", "", &DrawVector);
                    ImGui::EndMenu();
                }

                ImGui::Separator();

                if (ImGui::MenuItem((View->IsFullScreen ? u8"�˳�ȫ��" : u8"ȫ��"), "F11", &View->IsFullScreen))
                    View->UpdateFullScreenState();

                if (ImGui::MenuItem(u8"��ʾ����̨", "", &show_console))
                    UpdateConsoleState();

                ImGui::MenuItem(u8"��Ⱦ����", "", &render_dialog_active);

                ImGui::EndMenu();
            }  
            if (ImGui::BeginMenu(u8"ģʽ"))
            {
                if (ImGui::RadioButton(u8"����", mode == PanoramaMode::Sphere)) SwitchMode(PanoramaMode::Sphere);
                if (ImGui::RadioButton(u8"ƽ��", mode == PanoramaMode::Cylinder))  SwitchMode(PanoramaMode::Cylinder);
                if (ImGui::RadioButton(u8"С����", mode == PanoramaMode::Asteroid)) SwitchMode(PanoramaMode::Asteroid);
                if (ImGui::RadioButton(u8"ˮ����", mode == PanoramaMode::OuterBall))  SwitchMode(PanoramaMode::OuterBall);
                if (ImGui::RadioButton(u8"ȫ��", mode == PanoramaMode::Flat))  SwitchMode(PanoramaMode::Flat);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(u8"����"))
            {
                if (ImGui::MenuItem(u8"ʹ�ð���")) {
                    if (!help_dialog_showed) {
                        help_dialog_showed = true;
                        View->SendMessage(WM_CUSTOM_SHOW_HELPBOX, 0, 0);
                    }
                }
                if (ImGui::MenuItem(u8"����")) about_dialog_active = true;
                   
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    //���Թ���
    if (debug_tool_active) {
        ImGui::Begin("Debug Tool", &debug_tool_active, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {

            if (ImGui::BeginMenu(u8"Action"))
            {
                if (ImGui::MenuItem("Reset camera"))ResetCamera();
                if (ImGui::MenuItem("Reset model")) resetModel();
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

            if (ImGui::SliderFloat3("Camera pos", glm::value_ptr(View->Camera.Position), -5.0f, 5.0f))
                View->Camera.ForceUpdate();
            if (ImGui::SliderFloat3("Camera roate", glm::value_ptr(View->Camera.Rotate), -180.0f, 180.0f))
                View->Camera.ForceUpdate();
            if (ImGui::SliderFloat("Camera zoom", &View->Camera.Zoom, 0.0f, 179.0f))
                View->Camera.ForceUpdate();

            ImGui::Separator();

            ImGui::SliderFloat("RoateSpeed", &View->Camera.RoateSpeed, 0.0f, 40.0f);
            ImGui::SliderFloat("MovementSpeed", &View->Camera.MovementSpeed, 0.0f, 40.0f);
            ImGui::SliderFloat("MouseSensitivity", &View->Camera.MouseSensitivity, 0.0f, 1.0f);

            ImGui::Separator();

            ImGui::Checkbox("CameraPerspective", &CameraPerspective);
            ImGui::SliderFloat("CameraPerspectiveBottom", &CameraPerspectiveBottom, -1.0f, 1.0f);
            ImGui::SliderFloat("CameraPerspectiveTop", &CameraPerspectiveTop, -1.0f, 1.0f);
        }
        else if (debug_tool_active_tab == 2) {

            ImGui::SliderFloat3("Model pos", glm::value_ptr(Positon), -180.0f, 180.0f);
            if (ImGui::SliderFloat3("Model roate", glm::value_ptr(Rotation), -180.0f, 180.0f))
                UpdateModelVectors();

            ImGui::InputFloat3("Model Front", glm::value_ptr(Front), 3);
            ImGui::InputFloat3("Model Up", glm::value_ptr(Up), 3);
            ImGui::InputFloat3("Model Right", glm::value_ptr(Right), 3);

        }
        else if (debug_tool_active_tab == 3) {

            ImGui::SliderFloat2("UV Offest", glm::value_ptr(UVOffest), -10.0f, 10.0f);
            ImGui::Separator();

        }
        else if (debug_tool_active_tab == 4) {

            ImGui::Checkbox("FileOpened", &fileOpened);
            ImGui::Checkbox("currentImageUseChunkLoad", &currentImageUseChunkLoad);

            ImGui::Separator();

        }
        ImGui::End();
    }

    //�Ի���
    if (!fileOpened && !currentFileLoading) {
        ImGui::SetNextWindowBgAlpha(0.55f); // Transparent background
        ImGui::SetNextWindowSize(ImVec2(300, 100));
        ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - 300) / 2.0f, (io.DisplaySize.y - 100) / 2.0f));
        if (ImGui::Begin("welecome_overlay", &fileOpened, overlay_window_flags))
        {
            ImGui::Text(u8"��ӭʹ�� 720 Image Viewer\n���ȴ�һ��ȫ��ͼ�ļ�");
            ImGui::Separator();
            for (int i = 0; i < 6; i++)
                ImGui::Spacing();
            if (ImGui::Button(u8"��", ImVec2(284, 20)))
                openFile();     
            ImGui::End();
        }
    }
    if (about_dialog_active) {

        ImGui::SetNextWindowSize(ImVec2(330, 300));
        ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - 330) / 2.0f, (io.DisplaySize.y - 300) / 2.0f));

        if (ImGui::Begin(u8"���� 720 Image Viewer", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse)) {
         
            ImGui::Text(u8"720 Image Viewer");
            ImGui::Separator();
            ImGui::Image((ImTextureID)texture_logo,  ImVec2(texture_logo_size.x, texture_logo_size.y));
            ImGui::Separator();
            ImGui::Text(u8"����һ���򵥵� 720 ȫ��ͼ�鿴�����");
            ImGui::Text(u8"֧�ֶ���ͶӰ��ʽ���ɿ��ٴ�������� 720 ȫ��ͼ��");
            ImGui::Spacing();
            ImGui::Spacing();            ImGui::Spacing();
            ImGui::Separator();

            ImGui::SameLine();
            if (ImGui::Button(u8"������Ϣ") && !about_dialog_showed) {
                about_dialog_showed = true;
                View->SendMessage(WM_CUSTOM_SHOW_ABOUTBOX, 0, 0);
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"OpenGL ��Ϣ"))
                glinfo_dialog_active = true;
            ImGui::SameLine();
            if (ImGui::Button(u8"�õ�")) about_dialog_active = false;

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
        if (ImGui::Begin(u8"��Ⱦ����", &render_dialog_active)) {

            ImGui::SliderInt(u8"���� X  ��ֶ�", &preferImageXSeg, 10, 90);
            ImGui::SliderInt(u8"���� Y  ��ֶ�", &preferImageYSeg, 10, 90);

            ImGui::Text(u8"��ǰX ��ֶ�: %d", currentImageXSeg);
            ImGui::Text(u8"��ǰY ��ֶ�: %d", currentImageYSeg);

            if (ImGui::Button(u8"�õ�")) glinfo_dialog_active = false;

            ImGui::End();
        }
    }

    //�Ի���  
    if (loading_dialog_active) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::SetNextWindowBgAlpha(0.6f);
        if (ImGui::Begin("loading_bg", 0, overlay_window_flags))
            ImGui::End();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        if (ImGui::Begin("loading_box", 0, overlay_window_flags))
        {
            ImGui::Text(u8"ͼ�������У����Ժ�...");
            ImGui::End();
        }
    }
    if (message_dialog_active) {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        if (ImGui::Begin(u8"��ʾ", &message_dialog_active, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Text(message_dialog_message);
            ImGui::Separator();
            if (ImGui::Button(u8"ȷ��")) message_dialog_active = false;
            ImGui::End();
        }
    }
}

void CGameRenderer::Resize(int Width, int Height)
{
    glViewport(0, 0, Width, Height);
}
void CGameRenderer::Destroy()
{
    SaveSettings();
    closeFile();
    glDeleteProgram(shaderProgram);
}

void CGameRenderer::renderPanel(bool wireframe) {

    float zoomZv = FlatZoom * 2.0f;
    float vratio = Width > 0 ? ((float)Height / (float)Width) : zoomZv;
    float ustep = zoomZv / currentImageXSeg, vstep = vratio / currentImageYSeg;
    float u = -zoomZv / 2.0f, v = -zoomZv / 2.0f;

    for (int i = 0; i < currentImageXSeg; i++)
    {
        if (u > 1.0f) break;
        if (u < -1.0f - ustep) {
            u += ustep;
            continue;
        }

        v = -zoomZv / 2.0f;
        for (int j = 0; j < currentImageYSeg; j++)
        {
            if (v > 1.0f) break;
            if (v < -1.0f - vstep) {
                v += vstep;
                continue;
            }

            glBegin(wireframe ? GL_LINE_LOOP : GL_TRIANGLES);

            glm::vec2 uv1 = getMercatorPoint((u + ustep) / zoomZv, (v) / zoomZv);
            glm::vec2 uv2 = getMercatorPoint((u + ustep) / zoomZv, (v + vstep) / zoomZv);
            glm::vec2 uv3 = getMercatorPoint((u) / zoomZv, (v + vstep) / zoomZv);
            glm::vec2 uv4 = getMercatorPoint((u) / zoomZv, (v) / zoomZv);

            glTexCoord2f(uv1.x, uv1.y);
            glVertex3f(u + ustep, v, 0.0f);
            glTexCoord2f(uv2.x, uv2.y);
            glVertex3f(u + ustep, v + vstep, 0.0f);
            glTexCoord2f(uv4.x, uv4.y);
            glVertex3f(u, v, 0.0f);
            
            glEnd();

            glBegin(wireframe ? GL_LINE_LOOP : GL_TRIANGLES);

            glTexCoord2f(uv4.x, uv4.y);
            glVertex3f(u, v, 0.0f);
            glTexCoord2f(uv2.x, uv2.y);
            glVertex3f(u + ustep, v + vstep, 0.0f);
            glTexCoord2f(uv3.x, uv3.y);
            glVertex3f(u, v + vstep, 0.0f);

            glEnd();

            v += vstep;
        } 
        u += ustep;
    }
}
void CGameRenderer::renderSphere(bool wireframe) {

    float ustep = 1.0f / currentImageXSeg, vstep = 1.0f / currentImageYSeg;
    float u = 0, v = 0;

    //�����¶���������
    for (int i = 0; i < currentImageXSeg; i++)
    {
        if (!wireframe && currentImageUseChunkLoad) fasterSwitchTextureToChunk(i, 0);

        glBegin(wireframe ? GL_LINE_LOOP : GL_TRIANGLES);

        glm::vec3 a = getPoint(0.0f, 0.0f);
        glm::vec3 b = getPoint(u, vstep);
        glm::vec3 c = getPoint(u + ustep, vstep);
        
        glm::vec2 uva = getUVPoint(1.0f - (u + ustep), vstep, 0);
        glm::vec2 uvb = getUVPoint(1.0f - u, vstep, 1);
        glm::vec2 uvc = getUVPoint(1.0f - u, 0.0f, 2);

        glTexCoord2f(uva.x, uva.y);
        glVertex3f(c.x, c.y, c.z);
        glTexCoord2f(uvb.x, uvb.y);
        glVertex3f(b.x, b.y, b.z);
        glTexCoord2f(uvc.x, uvc.y);
        glVertex3f(a.x, a.y, a.z);
        
        u += ustep;
        glEnd();
    }
    //�����м��ı�����
    u = 0, v = vstep;
    for (int j = 1; j < currentImageYSeg - 1; j++)
    {
        for (int i = 0; i < currentImageXSeg; i++)
        {
            if (!wireframe && currentImageUseChunkLoad) fasterSwitchTextureToChunk(i, j);

            glBegin(wireframe ? GL_LINE_LOOP : GL_QUADS);

            glm::vec3 a = getPoint(u, v);
            glm::vec3 b = getPoint(u + ustep, v);
            glm::vec3 c = getPoint(u + ustep, v + vstep);
            glm::vec3 d = getPoint(u, v + vstep);

            glm::vec2 uva = getUVPoint(1.0f - u, v, 0);
            glm::vec2 uvb = getUVPoint(1.0f - (u + ustep), v, 1);
            glm::vec2 uvc = getUVPoint(1.0f - (u + ustep), v + vstep, 2);
            glm::vec2 uvd = getUVPoint(1.0f - u, v + vstep, 3);

            glTexCoord2f(uva.x, uva.y);
            glVertex3f(a.x, a.y, a.z);
            glTexCoord2f(uvb.x, uvb.y);
            glVertex3f(b.x, b.y, b.z);
            glTexCoord2f(uvc.x, uvc.y);
            glVertex3f(c.x, c.y, c.z);
            glTexCoord2f(uvd.x, uvd.y);
            glVertex3f(d.x, d.y, d.z);
          
            u += ustep;
            glEnd();
        }
        v += vstep;
    }
    //�����¶���������
    u = 0;
    for (int i = 0; i < currentImageXSeg; i++)
    {
        if (!wireframe && currentImageUseChunkLoad) fasterSwitchTextureToChunk(i, currentImageYSeg - 1);

        glBegin(wireframe ? GL_LINE_LOOP : GL_TRIANGLES);

        glm::vec3 a = getPoint(0.0f, 1.0f);
        glm::vec3 b = getPoint(u, 1 - vstep);
        glm::vec3 c = getPoint(u + ustep, 1 - vstep);

        glm::vec2 uva = getUVPoint(1.0f - u, 1.0f, 0);
        glm::vec2 uvb = getUVPoint(1.0f - u, 1 - vstep, 1);
        glm::vec2 uvc = getUVPoint(1.0f - (u + ustep), 1 - vstep, 2);

        glTexCoord2f(uva.x, uva.y);
        glVertex3f(a.x, a.y, a.z);
        glTexCoord2f(uvb.x, uvb.y);
        glVertex3f(b.x, b.y, b.z);
        glTexCoord2f(uvc.x, uvc.y);
        glVertex3f(c.x, c.y, c.z);
        
        u += ustep;
        glEnd();
    }
}
void CGameRenderer::renderABox() {

    float s = 0.1f;

    glDisable(GL_CULL_FACE);
    glBindTexture(GL_TEXTURE_2D, texture_logo);
    glUniform1i(useColorLoc, 0);

    // ǰ��
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(s, -s, s);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-s, -s, s);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-s, s, s);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(s, s, s);
    glEnd();


    glUniform1i(useColorLoc, 1);
    glUniform3f(ourColorLoc, 0.6f, 0.7f, 0.2f);

    // ����
    glBegin(GL_LINE_LOOP);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-s, -s, -s);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(s, -s, -s);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(s, -s, s);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-s, -s, s);
    glEnd();


    // ����
    glBegin(GL_LINE_LOOP);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-s, s, s);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-s, s, -s);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(s, s, -s);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(s, s, s);
    glEnd();

    // ����
    glBegin(GL_LINE_LOOP);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-s, -s, -s);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(s, -s, -s);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(s, s, -s);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-s, s, -s);
    glEnd();


    // ����
    glBegin(GL_LINE_LOOP);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(s, -s, -s);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(s, -s, s);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(s, s, s);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(s, s, -s);
    glEnd();

    // ����
    glBegin(GL_LINE_LOOP);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-s, -s, -s);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-s, -s, s);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-s, s, s);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-s, s, -s);
    glEnd();
}

glm::vec2 CGameRenderer::getUVPoint(float u, float v, short i) {
    //��ȡ������UVӳ���
    if (currentImageUseChunkLoad) {
        switch (i)
        {
        case 0:
            return glm::vec2(0.0f, 0.0f);
        case 1:
            return glm::vec2(1.0f, 0.0f);
        case 2:
            return glm::vec2(1.0f, 1.0f);
        case 3:
            return glm::vec2(0.0f, 1.0f);
        }
    }
    return glm::vec2(u, v);
}
glm::vec3 CGameRenderer::getPoint(float u, float v)
{
    //��ȡ�����ϵĵ�
    const float PI = glm::pi<float>();
    float x = glm::sin(PI * v) * glm::sin(PI * u * 2);
    float y = glm::cos(PI * v);
    float z = glm::sin(PI * v) * glm::cos(PI * u * 2);
    return glm::vec3(x, y, z);
}
glm::vec2 CGameRenderer::getMercatorPoint(float u, float v)
{    
    //��ȡƽ����UVӳ���
    return glm::vec2(-u / 2.0f + UVOffest.x, 0.5f - v + UVOffest.y);
}

void CGameRenderer::fasterSwitchTextureToChunk(int i, int j) {
    GLuint tex = texture_chunks[i][j];
    glBindTexture(GL_TEXTURE_2D, tex);
}

void CGameRenderer::finishImageChunkLoad() {
    currentFileLoading = false;
    resetModel();

    if (currentImageData != nullptr) {
        free(currentImageData);
        currentImageData = nullptr;
    }
}
BYTE* CGameRenderer::getImageChunkData(int x, int y, int chunkW, int chunkH)
{
    if (currentImageData) {

        currentImageChunkData = (LPBYTE)malloc(currentImageChunkDataSize);

        ULONG index = 0;
        ULONG offStart = (y * currentImageW + x) * currentImageDepth;
        ULONG off = offStart;
        ULONG offLine = (currentImageW - chunkW ) * currentImageDepth;

        for (int i = 0; i < chunkH; i++) {
            for (int j = 0; j < chunkW; j++) {
                currentImageChunkData[index] = currentImageData[off];
                currentImageChunkData[index + 1] = currentImageData[off + 1];
                currentImageChunkData[index + 2] = currentImageData[off + 2];
                index += 3;
                off += currentImageDepth;
            }
            off += offLine;
        }

        return currentImageChunkData;
    }
    return nullptr;
}

void CGameRenderer::UpdateModelVectors()
{
    // �����µ�ǰ����
    glm::vec3 front;
    front.x = cos(glm::radians(Rotation.y)) * cos(glm::radians(Rotation.x));
    front.y = sin(glm::radians(Rotation.x));
    front.z = sin(glm::radians(Rotation.y)) * cos(glm::radians(Rotation.x));
    Front = glm::normalize(front);
    // �ټ�����������������
    Right = glm::normalize(glm::cross(Front, WorldUp));  // ��׼��
    Up = glm::normalize(glm::cross(Right, Front));
}
void CGameRenderer::UpdateConsoleState() {
    ShowWindow(GetConsoleWindow(), show_console ? SW_SHOW : SW_HIDE);
}

void CGameRenderer::resetModel() {
    Positon = glm::vec3(0.0f);
    Rotation = glm::vec3(0.0f);
    UpdateModelVectors();
}

bool CGameRenderer::createShader() {
    // ********************************
    // 1. ������ɫ������
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 2. ���붥����ɫ��
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint compileResult = GL_TRUE;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileResult);
    if (compileResult == GL_FALSE) {
        char szLog[1024] = { 0 }; GLsizei logLen = 0;
        glGetShaderInfoLog(vertexShader, 1024, &logLen, szLog);

        logger->LogError2(L"Compile vertexShader FAILED ! \n%hs", szLog);

        glDeleteShader(vertexShader);
        return false;
    }

    // --------------------------------
    // 1. Ƭ��ɫ������
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 2. Ƭ��ɫ��
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    compileResult = GL_TRUE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileResult);
    if (compileResult == GL_FALSE) {
        char szLog[1024] = { 0 }; GLsizei logLen = 0;
        glGetShaderInfoLog(fragmentShader, 1024, &logLen, szLog);
        
        logger->LogError2(L"Compile fragmentShader FAILED ! \n%hs", szLog);

        glDeleteShader(fragmentShader);
        return false;
    }

    // --------------------------------
    // 1. ��ɫ���������
    shaderProgram = glCreateProgram();

    // 2. ������ɫ������
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // ---------------------------------
    // 1. ������ɫ������
    glUseProgram(shaderProgram);
    // 2. ������ͷ�ǰ�������ڴ�
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return true;
}
void CGameRenderer::createPanoramaTexture() {

    glGenTextures(1, &texture_one);
    glBindTexture(GL_TEXTURE_2D, texture_one);

    //    Ϊ��ǰ�󶨵�����������û��ơ����˷�ʽ
    //    �������װ����ΪGL_REPEAT��Ĭ�ϰ�װ������
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//  ����������˲���
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)currentImageW, (GLsizei)currentImageH, 0, GL_RGB, GL_UNSIGNED_BYTE, currentImageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    finishImageChunkLoad();
}
void CGameRenderer::createPanoramaTextureChunk() {
    if (currentImageData != nullptr) {        

        for (int i = 0; i < currentImageXSeg; i++) {
            for (int j = 0; j < currentImageYSeg; j++) {
                int x = (int)(i * texture_chunk_size.x);
                int y = (int)(j * texture_chunk_size.y);

                currentImageChunkData = getImageChunkData(x, y, (int)texture_chunk_size.x, (int)texture_chunk_size.y);

                glGenTextures(1, &texture_chunks[i][j]);
                glBindTexture(GL_TEXTURE_2D, texture_chunks[i][j]);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)texture_chunk_size.x, (GLsizei)texture_chunk_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, currentImageChunkData);
                glGenerateMipmap(GL_TEXTURE_2D);

                free(currentImageChunkData);

                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }

        finishImageChunkLoad();
    }
}
void CGameRenderer::loadPanoramaTextureChunk() {
    currentLoadTextureReqLoadToGL = true;
}
void CGameRenderer::loadPanoramaTexture() {

    glm::vec2 size = currentFileLoader->GetImageSize();
    currentImageW = (int)size.x;
    currentImageH = (int)size.y;

    logger->Log(L"Image size %dx%d", currentImageW, currentImageH);

    //ʹ���������ģʽ
    if (currentImageW > 4096 || currentImageH > 4096) {

        //���������С
        currentImageUseChunkLoad = true;
        currentImageXSeg = (int)(currentImageW / (float)preferImageChunkSize) + (currentImageW % preferImageChunkSize == 0 ? 0 : 1);
        currentImageYSeg = (int)(currentImageH / (float)preferImageChunkSize) + (currentImageH % preferImageChunkSize == 0 ? 0 : 1);

        if (currentImageXSeg < preferImageXSeg) currentImageXSeg = preferImageXSeg;
        if (currentImageYSeg < preferImageYSeg) currentImageYSeg = preferImageYSeg;
        if (currentImageXSeg > 64) currentImageXSeg = 64;
        if (currentImageYSeg > 64) currentImageYSeg = 64;

        texture_chunk_size = glm::vec2(currentImageW / (float)currentImageXSeg, currentImageH / (float)currentImageYSeg);

        currentLoadTextureChunkCount = currentImageXSeg * currentImageYSeg;
        currentLoadTextureChunkIndexX = 0;
        currentLoadTextureChunkIndexY = 0;
        currentLoadTextureReqNext = true;//֪ͨ�߳̽��м���
        currentImageData = currentFileLoader->GetAllImageData();
        currentImageDepth = currentFileLoader->GetImageDepth();
        currentImageChunkDataSize = (int)(texture_chunk_size.x * texture_chunk_size.y) * currentImageDepth;
        currentImageChunkOnceLoad = currentLoadTextureChunkCount / 128;
        ReqTexLoadTicked = false;

        logger->Log(L"Use chunk load, chunk size: %.2fx%.2f, count: %d , Data size : %d, chunk data size : %d", texture_chunk_size.x, 
            texture_chunk_size.y, currentLoadTextureChunkCount, currentFileLoader->GetFullDataSize(), currentImageChunkDataSize);
        logger->Log(L"Seg size: %dx%d", currentImageXSeg, preferImageYSeg);

        loading_dialog_active = false;
    }
    else {
        //ʹ�õ���ͼ����ģʽ
        currentLoadTextureChunkCount = 0;
        currentImageXSeg = preferImageXSeg;
        currentImageYSeg = preferImageYSeg;
        currentImageUseChunkLoad = false;
        currentImageData = currentFileLoader->GetAllImageData();
        currentLoadTextureReqNext = false;
        ReqTexLoadTicked = true;

        logger->Log(L"Use singe load : Data size : %d", currentFileLoader->GetFullDataSize());
        logger->Log(L"Seg size: %dx%d", currentImageXSeg, preferImageYSeg);
    }

    fileOpened = true;
}
void CGameRenderer::destroyPanoramaTextures() {
    if (texture_one > 0)
    {
        glDeleteTextures(1, &texture_one);
        texture_one = 0;
    }
    currentLoadTextureReqNext = false;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            if (texture_chunks[i][j] != 0) {
                GLuint d = texture_chunks[i][j];
                glDeleteTextures(1, &d);
                texture_chunks[i][j] = 0;;
            }
        }
    }
}

void CGameRenderer::loadLogoTexture() {
    HRSRC hResource = NULL;
    HGLOBAL hGlobal = NULL;
    LPVOID pData = NULL;
    DWORD dwSize = NULL;
    DWORD writed;

    hResource = FindResourceW(CApp::Instance->GetHInstance(), MAKEINTRESOURCE(IDB_LOGO), L"BIN"); if (!hResource) return;
    hGlobal = LoadResource(CApp::Instance->GetHInstance(), hResource); if (!hGlobal) return;
    pData = LockResource(hGlobal);
    dwSize = SizeofResource(CApp::Instance->GetHInstance(), hResource);

    std::wstring logoPath(CApp::Instance->GetCurrentDir());
    logoPath += L"\\logo.bmp";
    if (!PathFileExists(logoPath.c_str())) {
        HANDLE hFile = CreateFile(logoPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            logger->LogError2(L"Create logo file: %s Failed %d", logoPath.c_str(), GetLastError());
            return;
        }
        if (!WriteFile(hFile, pData, dwSize, &writed, NULL)) {
            logger->LogError2(L"Write logo file: %s Failed %d", logoPath.c_str(), GetLastError());
            CloseHandle(hFile);
            return;
        }
        CloseHandle(hFile);
    }

    char* pathAnsi = StringHlp::UnicodeToAnsi(logoPath.c_str());
    int w, h, nrChannels;
    logoImageData = stbi_load(pathAnsi, &w, &h, &nrChannels, 0);
    StringHlp::FreeStringPtr(pathAnsi);

    texture_logo_size = glm::vec2(w, h);

    glGenTextures(1, &texture_logo);
    glBindTexture(GL_TEXTURE_2D, texture_logo);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)w, (GLsizei)h, 0, GL_RGB, GL_UNSIGNED_BYTE, logoImageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(logoImageData);
    glBindTexture(GL_TEXTURE_2D, 0);

    FreeResource(hGlobal);
}

void CGameRenderer::showMessageDialog(const WCHAR* str)
{
    if (message_dialog_message != nullptr) 
        delete message_dialog_message;
    message_dialog_message = StringHlp::UnicodeToUtf8(str);
    message_dialog_active = true;
}
void CGameRenderer::showMessageDialog(const char* str)
{
    if (message_dialog_message != nullptr)
        delete message_dialog_message;
    message_dialog_message = (char*)str;
    message_dialog_active = true;
}

void CGameRenderer::openFile() {
    View->SendMessage(WM_CUSTOM_OPEN_FILE, 0, 0);
}
void CGameRenderer::closeFile() {
    if (fileOpened) {
        fileOpened = false;
        logger->Log(L"Close file %hs", currentFilePath.c_str());

        destroyFileLoader();
        destroyPanoramaTextures(); 

        resetModel();

        SetWindowText(View->GetHWND(), APP_TITLE);
    }
}
void CGameRenderer::destroyFileLoader() {
    if (currentFileLoader != nullptr) {
        currentFileLoader->Destroy();
        delete currentFileLoader;
        currentFileLoader = nullptr;

        logger->Log2(L"FileLoader destroyed");
    }
}
void CGameRenderer::doOpenFile() {

    if (fileOpened) {
        if (currentFilePath == currentOpenFilePath)
            return;
        closeFile();
    }
    else resetModel();

    loading_dialog_active = true;
    currentFileLoading = true;
    const wchar_t* path = currentOpenFilePath.c_str();
    if (_waccess_s(path, 0) == 0) {
        ImageType type = CImageLoader::CheckImageType(path);
        if (type == ImageType::Unknow)  goto ERR_RETURN;

        logger->Log(L"Load file %s, type : %d", path, type);

        currentFileLoader = CImageLoader::CreateImageLoaderType(type);
        if (currentFileLoader == nullptr) goto ERR_RETURN;
        if (!currentFileLoader->Load(path))  goto ERR_RETURN;
        currentFilePath = path;
        loadPanoramaTexture();

        SetWindowText(View->GetHWND(), FormatString(L"%s - %s", APP_TITLE, path).c_str());
        return;
    ERR_RETURN:
        loading_dialog_active = false;
        currentFileLoading = false;
        if (currentFileLoader != nullptr) {
            showMessageDialog(FormatString(L"ͼ������ʧ��\n������Ϣ��\n%s", currentFileLoader->GetLastError()).c_str());
            destroyFileLoader();
        }
        else {
            showMessageDialog(L"ͼ������ʧ��\n��֧�ֵ�ͼ���ʽ");
        }
    }
    else {
        showMessageDialog(FormatString(L"�ļ� %s �����ڣ�", path).c_str());
        logger->LogError2(L"File %s not exists! ", path);
        currentFileLoading = false;
    }
}
