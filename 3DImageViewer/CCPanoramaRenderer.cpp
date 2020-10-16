#include "CCPanoramaRenderer.h"
#include "CApp.h"
#include "COpenGLRenderer.h"
#include "CGameRenderer.h"

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

CCPanoramaRenderer::CCPanoramaRenderer(COpenGLRenderer* renderer)
{
    Renderer = renderer;
}

void CCPanoramaRenderer::Init()
{

    glEnable(GL_TEXTURE_2D);
    glVendor = (GLubyte*)glGetString(GL_VENDOR);            //返回负责当前OpenGL实现厂商的名字
    glRenderer = (GLubyte*)glGetString(GL_RENDERER);    //返回一个渲染器标识符，通常是个硬件平台
    glVersion = (GLubyte*)glGetString(GL_VERSION);    //返回当前OpenGL实现的版本号
    glslVersion = (GLubyte*)glGetString(GL_SHADING_LANGUAGE_VERSION);//返回着色预压编译器版本号

    CreateShader();

    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    ourTextrueLoc = glGetUniformLocation(shaderProgram, "ourTexture");
    useColorLoc = glGetUniformLocation(shaderProgram, "useColor");
    ourColorLoc = glGetUniformLocation(shaderProgram, "ourColor");
}
void CCPanoramaRenderer::Destroy()
{
    glDeleteProgram(shaderProgram);
}

void CCPanoramaRenderer::Render()
{
    CCTexture::UnUse();
    glUseProgram(shaderProgram);

    //绘制调试线框
    if (renderDebugWireframe) {
        glUniform1i(useColorLoc, 1);
        glUniform3f(ourColorLoc, 0.43f, 0.45f, 0.70f);
        RenderThumbnail(true);
    }

    //绘制外层缩略图
    if (panoramaThumbnailTex) {
        glUniform1i(useColorLoc, 0);
        panoramaThumbnailTex->Use();
        RenderThumbnail();
    }

    CCTexture::UnUse();

    if (renderPanoramaFull)
        RenderFullChunks();
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

void CCPanoramaRenderer::RenderThumbnail(bool wireframe)
{
    float r = 1.0f;
    float ustep = 1.0f / sphereSegmentX, vstep = 1.0f / sphereSegmentY;
    float u = 0, v = 0;

    //绘制下端三角形组
    for (int i = 0; i < sphereSegmentX; i++)
    {
        glBegin(wireframe ? GL_LINE_LOOP : GL_TRIANGLES);

        glm::vec3 a = GetSpherePoint(0.0f, 0.0f, r);
        glm::vec3 b = GetSpherePoint(u, vstep, r);
        glm::vec3 c = GetSpherePoint(u + ustep, vstep, r);

        glm::vec2 uva = GetSphereUVPoint(1.0f - (u + ustep), vstep, 0);
        glm::vec2 uvb = GetSphereUVPoint(1.0f - u, vstep, 1);
        glm::vec2 uvc = GetSphereUVPoint(1.0f - u, 0.0f, 2);

        glTexCoord2f(uva.x, uva.y);
        glVertex3f(c.x, c.y, c.z);
        glTexCoord2f(uvb.x, uvb.y);
        glVertex3f(b.x, b.y, b.z);
        glTexCoord2f(uvc.x, uvc.y);
        glVertex3f(a.x, a.y, a.z);

        u += ustep;
        glEnd();
    }
    //绘制中间四边形组
    u = 0, v = vstep;
    for (int j = 1; j < sphereSegmentY - 1; j++)
    {
        for (int i = 0; i < sphereSegmentX; i++)
        {
            glBegin(wireframe ? GL_LINE_LOOP : GL_QUADS);

            glm::vec3 a = GetSpherePoint(u, v, r);
            glm::vec3 b = GetSpherePoint(u + ustep, v, r);
            glm::vec3 c = GetSpherePoint(u + ustep, v + vstep, r);
            glm::vec3 d = GetSpherePoint(u, v + vstep, r);

            glm::vec2 uva = GetSphereUVPoint(1.0f - u, v, 0);
            glm::vec2 uvb = GetSphereUVPoint(1.0f - (u + ustep), v, 1);
            glm::vec2 uvc = GetSphereUVPoint(1.0f - (u + ustep), v + vstep, 2);
            glm::vec2 uvd = GetSphereUVPoint(1.0f - u, v + vstep, 3);

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
    //绘制下端三角形组
    u = 0;
    for (int i = 0; i < sphereSegmentX; i++)
    {
        glBegin(wireframe ? GL_LINE_LOOP : GL_TRIANGLES);

        glm::vec3 a = GetSpherePoint(0.0f, 1.0f, r);
        glm::vec3 b = GetSpherePoint(u, 1 - vstep, r);
        glm::vec3 c = GetSpherePoint(u + ustep, 1 - vstep, r);

        glm::vec2 uva = GetSphereUVPoint(1.0f - u, 1.0f, 0);
        glm::vec2 uvb = GetSphereUVPoint(1.0f - u, 1 - vstep, 1);
        glm::vec2 uvc = GetSphereUVPoint(1.0f - (u + ustep), 1 - vstep, 2);

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
