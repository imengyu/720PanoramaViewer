#include "CCPanoramaRenderer.h"
#include "CApp.h"
#include "COpenGLRenderer.h"
#include "CGameRenderer.h"
#include "CCMesh.h"
#include "CCRenderGlobal.h"

const char* vertexShaderSource = "\
#version 410 core\n\
layout (location = 0) in vec3 aPosition;\n\
layout (location = 1) in vec2 aUv;\n\
out vec2 TexCoord;\n\
\n\
uniform mat4 model;\n\
uniform mat4 view;\n\
uniform mat4 projection;\n\
void main() {\n\
    gl_Position = projection * view * model * vec4(aPosition, 1.0f);\n\
    TexCoord = aUv;\n\
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

CCPanoramaRenderer::CCPanoramaRenderer(COpenGLRenderer* renderer)
{
    Renderer = renderer;
    logger = CApp::Instance->GetLogger();
}

void CCPanoramaRenderer::Init()
{
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);

    CreateShader();
    CreateMainModel();

    globalRenderInfo = new CCRenderGlobal();

    globalRenderInfo->glVendor = (GLubyte*)glGetString(GL_VENDOR);            //返回负责当前OpenGL实现厂商的名字
    globalRenderInfo->glRenderer = (GLubyte*)glGetString(GL_RENDERER);    //返回一个渲染器标识符，通常是个硬件平台
    globalRenderInfo->glVersion = (GLubyte*)glGetString(GL_VERSION);    //返回当前OpenGL实现的版本号
    globalRenderInfo->glslVersion = (GLubyte*)glGetString(GL_SHADING_LANGUAGE_VERSION);//返回着色预压编译器版本号

    globalRenderInfo->viewLoc = glGetUniformLocation(shaderProgram, "view");
    globalRenderInfo->projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    globalRenderInfo->modelLoc = glGetUniformLocation(shaderProgram, "model");
    globalRenderInfo->ourTextrueLoc = glGetUniformLocation(shaderProgram, "ourTexture");
    globalRenderInfo->useColorLoc = glGetUniformLocation(shaderProgram, "useColor");
    globalRenderInfo->ourColorLoc = glGetUniformLocation(shaderProgram, "ourColor");

    CCRenderGlobal::SetInstance(globalRenderInfo);
}
void CCPanoramaRenderer::Destroy()
{
    CCRenderGlobal::Destroy();

    if (mainModel != nullptr) {
        delete mainModel;
        mainModel = nullptr;
    }

    glDeleteProgram(shaderProgram);
}

void CCPanoramaRenderer::Render()
{
    CCTexture::UnUse();

    //模型位置和矩阵映射
    glUniformMatrix4fv(globalRenderInfo->modelLoc, 1, GL_FALSE, glm::value_ptr(mainModel->GetMatrix()));

    //绘制外层缩略图
    if (panoramaThumbnailTex) {
        glUniform1i(globalRenderInfo->useColorLoc, 0);
        panoramaThumbnailTex->Use();
        RenderThumbnail();
    }

    CCTexture::UnUse();

    if (renderPanoramaFull) {
        RenderFullChunks();
    }

    //绘制调试线框
    if (renderDebugWireframe) {
        glUniform1i(globalRenderInfo->useColorLoc, 1);
        glUniform3f(globalRenderInfo->ourColorLoc, wireframeColor.r, wireframeColor.g, wireframeColor.b);
        glColor3f(wireframeColor.r, wireframeColor.g, wireframeColor.b);
        RenderThumbnail(true);
    }
}
void CCPanoramaRenderer::CreateMainModelSphereMesh(CCMesh* mesh) {

    float r = 1.0f;
    float ustep = 1.0f / sphereSegmentX, vstep = 1.0f / sphereSegmentY;
    float u = 0, v = 0;

    //顶点
    //=======================================================

    mesh->vertices.push_back(GetSpherePoint(0.0f, 0.0f, r));
    mesh->uv.push_back(glm::vec2(0.0f, 0.0f));

    u = 0, v = vstep;
    for (int j = 1; j < sphereSegmentY; j++, v += vstep) {
        u = 0;
        for (int i = 0; i < sphereSegmentX; i++, u += ustep) {
            mesh->vertices.push_back(GetSpherePoint(u, v, r));
            mesh->uv.push_back(glm::vec2(u, v));
        }
    }

    mesh->vertices.push_back(GetSpherePoint(0.0f, 1.0f, r));
    mesh->uv.push_back(glm::vec2(0.0f, 1.0f));

    //顶点索引
    //=======================================================

    int all_vertices_count = mesh->vertices.size();

    for (int i = 0; i < sphereSegmentX; i++) {
        mesh->indices.push_back(i == sphereSegmentX - 1 ? 1 : i + 2);
        mesh->indices.push_back(i + 1);
        mesh->indices.push_back(0);
    } 
    for (int j = 1; j < sphereSegmentY - 1; j++) {
        int line_start_pos = 1 + (j - 1) * sphereSegmentX;
        for (int i = 0; i < sphereSegmentX; i++) {

            mesh->indices.push_back(line_start_pos + i);
            mesh->indices.push_back(i == sphereSegmentX - 1 ? line_start_pos + sphereSegmentX : line_start_pos + i + sphereSegmentX + 1);
            mesh->indices.push_back(line_start_pos + i + sphereSegmentX);
            
            mesh->indices.push_back(i == sphereSegmentX - 1 ? line_start_pos + sphereSegmentX : line_start_pos + i + sphereSegmentX + 1);
            mesh->indices.push_back(line_start_pos + i);
            mesh->indices.push_back(i == sphereSegmentX - 1 ? line_start_pos : line_start_pos + i + 1);
        }
    }
    int line_start_pos = 1 + sphereSegmentX * (sphereSegmentY - 3);
    for (int i = 0; i < sphereSegmentX; i++) {
        mesh->indices.push_back(i == sphereSegmentX - 1 ? line_start_pos : line_start_pos + i + 1);
        mesh->indices.push_back(all_vertices_count - 1);
        mesh->indices.push_back(line_start_pos + i);

    }  

    //创建缓冲区
    mesh->GenerateBuffer();
}
void CCPanoramaRenderer::CreateMainModel() {
    mainModel = new CCModel();
    mainModel->Mesh = new CCMesh();

    CreateMainModelSphereMesh(mainModel->Mesh);
}
bool CCPanoramaRenderer::CreateShader() {

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint compileResult = GL_TRUE;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileResult);
    if (compileResult == GL_FALSE) {
        char szLog[1024] = { 0 }; GLsizei logLen = 0;
        glGetShaderInfoLog(vertexShader, 1024, &logLen, szLog);

        CApp::Instance->GetLogger()->LogError2(L"Compile vertexShader FAILED ! \n%hs", szLog);

        glDeleteShader(vertexShader);
        return false;
    }

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    compileResult = GL_TRUE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileResult);
    if (compileResult == GL_FALSE) {
        char szLog[1024] = { 0 }; GLsizei logLen = 0;
        glGetShaderInfoLog(fragmentShader, 1024, &logLen, szLog);

        CApp::Instance->GetLogger()->LogError2(L"Compile fragmentShader FAILED ! \n%hs", szLog);

        glDeleteShader(fragmentShader);
        return false;
    }

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}
void CCPanoramaRenderer::ReleaseTexPool() {
    renderPanoramaFull = false;
    if (panoramaTexPool.size() > 0) {
        std::vector<CCTexture*>::iterator it;
        for (it = panoramaTexPool.begin(); it != panoramaTexPool.end(); it++) {
            CCTexture* tex = *it;
            delete tex;
        }
        panoramaTexPool.clear();
    }
    panoramaThumbnailTex = nullptr;
}

void CCPanoramaRenderer::ResetModel()
{
    mainModel->Reset();
}
void CCPanoramaRenderer::RotateModel(float xoffset, float yoffset)
{
    mainModel->Rotation.y += xoffset;
    mainModel->Rotation.z -= yoffset;
    mainModel->UpdateVectors();
}

void CCPanoramaRenderer::RenderThumbnail(bool wireframe)
{
    glPolygonMode(GL_FRONT, wireframe ? GL_LINE : GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);

    //绘制
    mainModel->Mesh->RenderMesh();
}
void CCPanoramaRenderer::RenderFullChunks()
{




}

glm::vec2 CCPanoramaRenderer::GetSphereUVPoint(float u, float v, short i) {
    return glm::vec2(u, v);
}
glm::vec3 CCPanoramaRenderer::GetSpherePoint(float u, float v, float r)
{
    
    const float PI = glm::pi<float>();
    float x = r * glm::sin(PI * v) * glm::sin(PI * u * 2);
    float y = r * glm::cos(PI * v);
    float z = r * glm::sin(PI * v) * glm::cos(PI * u * 2);
    return glm::vec3(x, y, z);
}
