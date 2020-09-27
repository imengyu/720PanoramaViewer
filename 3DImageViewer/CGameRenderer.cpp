#include "CGameRenderer.h"
#include "COpenGLView.h"
#include <vector>

CGameRenderer::CGameRenderer()
{
	currentOpenFilePath = "";
}
CGameRenderer::~CGameRenderer()
{
}

void CGameRenderer::SetOpenFilePath(std::string path)
{
	currentOpenFilePath = path;
}

const char* vertexShaderSource = "\
#version 410 core\n\
layout(location = 0) in vec3 aPos;\n\
\n\
uniform mat4 model;\n\
uniform mat4 view;\n\
uniform mat4 projection;\n\
void main() {\n\
   gl_Position = projection * view * model * vec4(aPos, 1.0);\n\
}\0";

const char* fragmentShaderSource = ""
"#version 410 core\n"
"out vec4 FragColor;\n"        // 颜色属性（输出变量）
"void main(){\n"
"   FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"   // 固定颜色输出
"}\n\0";

bool CGameRenderer::Init()
{
    const int Y_SEGMENTS = 20;
    const int X_SEGMENTS = 40;

    glViewport(0, 0, Width, Height);

    std::vector<GLfloat> sphereVertices;
    std::vector<GLushort> sphereIndices;

    //2-计算球体顶点
    //生成球的顶点
    for (int y = 0; y <= Y_SEGMENTS; y++)
    {
        for (int x = 0; x <= X_SEGMENTS; x++)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);
        }
    }

    //生成球的Indices
    for (int i = 0; i < Y_SEGMENTS; i++)
    {
        for (int j = 0; j < X_SEGMENTS; j++)
        {
            sphereIndices.push_back(i * (X_SEGMENTS + 1) + j);
            sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j);
            sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
            sphereIndices.push_back(i * (X_SEGMENTS + 1) + j);
            sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
            sphereIndices.push_back(i * (X_SEGMENTS + 1) + j + 1);
        }
    }

    vertex_count = X_SEGMENTS * Y_SEGMENTS * 6;

    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    glGenBuffers(1, &vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(GLfloat), &sphereVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &element_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(GLushort), &sphereIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    

    /*float vertices[] = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,

        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,

        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,

        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
    };

    glGenVertexArrays(1, &vertex_array_object);
    glGenBuffers(1, &vertex_buffer_object);
    glBindVertexArray(vertex_array_object);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
*/

    createShader();

    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    modelLoc = glGetUniformLocation(shaderProgram, "model");

    View->Camera.SetPosItion(glm::vec3(0.0f, 1.0f, 4.6f));
    View->Camera.SetRotation(glm::vec3(-10.0f, -90.0f, 0.0f));

	return true;
}

void CGameRenderer::Render(float FrameTime)
{
    glClearColor(0.0f, 0.34f, 0.57f, 1.0f);
    glLoadIdentity();
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glColor3f(0.8f, 0.2f, 0.7f);

    glUseProgram(shaderProgram);

    glm::mat4 view = View->Camera.GetViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 projection = glm::perspective(glm::radians(View->Camera.Zoom), (float)View->Width / (float)View->Height, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


    glBindVertexArray(vertex_array_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);


    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_SHORT, 0);
    //glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    glLoadIdentity();
}
void CGameRenderer::RenderUI()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open")) {

            }
            if (ImGui::MenuItem("Close")) {

            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Options"))
            {
                ImGui::MenuItem("Info Overlay", "", &View->ShowInfoOverlay);
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Quit")) { View->CloseView();  }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void CGameRenderer::Resize(int Width, int Height)
{
    glViewport(0, 0, Width, Height);
}

void CGameRenderer::Destroy()
{
    glDisableVertexAttribArray(0);
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteBuffers(1, &vertex_buffer_object);
    glDeleteBuffers(1, &element_buffer_object);
    glDeleteProgram(shaderProgram);
}

void CGameRenderer::createShader() {
    // ********************************
// 1. 顶点着色器对象
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 2. 编译顶点着色器
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // --------------------------------
    // 1. 片着色器对象
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 2. 片着色器
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // --------------------------------
    // 1. 着色器程序对象
    shaderProgram = glCreateProgram();
    // 2. 链接着色器程序
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // ---------------------------------
    // 1. 激活着色器程序
    glUseProgram(shaderProgram);
    // 2. 激活后，释放前面分配的内存
    //glDeleteShader(vertexShader);
    //glDeleteShader(fragmentShader);
}

glm::vec3  CGameRenderer::getPoint(GLfloat u, GLfloat v) {
    GLfloat r = 0.9f;
    GLfloat pi = glm::pi<float>();
    GLfloat z = r * std::cos(pi * u);
    GLfloat x = r * std::sin(pi * u) * std::cos(2 * pi * v);
    GLfloat y = r * std::sin(pi * u) * std::sin(2 * pi * v);
    // std::cout << x << "," << y << "," << z << std::endl;
    return glm::vec3(x, y, z);
}

void CGameRenderer::createSphere(GLfloat* sphere, GLuint Longitude, GLuint Latitude) {
    // Longitude：经线切分个数
    // Latitude：纬线切分个数
    GLfloat lon_step = 1.0f / Longitude;
    GLfloat lat_step = 1.0f / Latitude;
    GLuint offset = 0;
    for (GLuint lat = 0; lat < Latitude; lat++) {  // 纬线u
        for (GLuint lon = 0; lon < Longitude; lon++) { // 经线v
            // 一次构造4个点，两个三角形，
            glm::vec3 point1 = getPoint(lat * lat_step, lon * lon_step);
            glm::vec3 point2 = getPoint((lat + 1) * lat_step, lon * lon_step);
            glm::vec3 point3 = getPoint((lat + 1) * lat_step, (lon + 1) * lon_step);
            glm::vec3 point4 = getPoint(lat * lat_step, (lon + 1) * lon_step);
            memcpy(sphere + offset, glm::value_ptr(point1), 3 * sizeof(GLfloat));
            offset += 3;
            memcpy(sphere + offset, glm::value_ptr(point4), 3 * sizeof(GLfloat));
            offset += 3;
            memcpy(sphere + offset, glm::value_ptr(point3), 3 * sizeof(GLfloat));
            offset += 3;

            memcpy(sphere + offset, glm::value_ptr(point1), 3 * sizeof(GLfloat));
            offset += 3;
            memcpy(sphere + offset, glm::value_ptr(point3), 3 * sizeof(GLfloat));
            offset += 3;
            memcpy(sphere + offset, glm::value_ptr(point2), 3 * sizeof(GLfloat));
            offset += 3;
        }
    }
    // std::cout<<"offset:" << offset << std::endl;
}
