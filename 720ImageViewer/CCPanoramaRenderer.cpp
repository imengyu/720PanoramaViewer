#include "CCPanoramaRenderer.h"
#include "CApp.h"
#include "COpenGLRenderer.h"
#include "CGameRenderer.h"
#include "CCMesh.h"
#include "CCMaterial.h"
#include "CCRenderGlobal.h"

CCPanoramaRenderer::CCPanoramaRenderer(COpenGLRenderer* renderer)
{
    Renderer = renderer;
    logger = CApp::Instance->GetLogger();
}

void CCPanoramaRenderer::Init()
{
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);

    LoadBuiltInResources();

    shader = new CCShader(
        CCFileManager::GetResourcePath("shader", "Standard_vertex.glsl").c_str(),
        CCFileManager::GetResourcePath("shader", "Standard_fragment.glsl").c_str());

    CreateMainModel();

    globalRenderInfo = new CCRenderGlobal();

    globalRenderInfo->glVendor = (GLubyte*)glGetString(GL_VENDOR);            //返回负责当前OpenGL实现厂商的名字
    globalRenderInfo->glRenderer = (GLubyte*)glGetString(GL_RENDERER);    //返回一个渲染器标识符，通常是个硬件平台
    globalRenderInfo->glVersion = (GLubyte*)glGetString(GL_VERSION);    //返回当前OpenGL实现的版本号
    globalRenderInfo->glslVersion = (GLubyte*)glGetString(GL_SHADING_LANGUAGE_VERSION);//返回着色预压编译器版本号

    globalRenderInfo->viewLoc = shader->GetUniformLocation("view");
    globalRenderInfo->projectionLoc = shader->GetUniformLocation("projection");
    globalRenderInfo->modelLoc = shader->GetUniformLocation("model");
    globalRenderInfo->ourTextrueLoc = shader->GetUniformLocation("ourTexture");
    globalRenderInfo->useColorLoc = shader->GetUniformLocation("useColor");
    globalRenderInfo->ourColorLoc = shader->GetUniformLocation("ourColor");
    globalRenderInfo->texOffest = shader->GetUniformLocation("texOffest");
    globalRenderInfo->texTilling = shader->GetUniformLocation("texTilling");

}
void CCPanoramaRenderer::Destroy()
{
    CCRenderGlobal::Destroy();

    if (shader != nullptr) {
        delete shader;
        shader = nullptr;
    }
    if (mainModel != nullptr) {
        delete mainModel;
        mainModel = nullptr;
    }
    if (panoramaCheckTex != nullptr) {
        delete panoramaCheckTex;
        panoramaCheckTex = nullptr;
    }
    ReleaseTexPool();
}

void CCPanoramaRenderer::Render()
{
    CCRenderGlobal::SetInstance(globalRenderInfo);
    CCTexture::UnUse();
    shader->Use();

    //摄像机矩阵
    Renderer->View->CalcMainCameraProjection(shader);

    //模型位置和矩阵映射
    glUniformMatrix4fv(globalRenderInfo->modelLoc, 1, GL_FALSE, glm::value_ptr(mainModel->GetMatrix()));

    //绘制外层缩略图
    glUniform1i(globalRenderInfo->useColorLoc, 0);
    RenderThumbnail();

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

    for (int j = 0; j <= sphereSegmentY; j++, v += vstep) {
        u = 0;
        for (int i = 0; i <= sphereSegmentX; i++, u += ustep) {
            mesh->position.push_back(GetSpherePoint(u, v, r));
            mesh->texCoords.push_back(glm::vec2(u, v));
        }
    }

    //顶点索引
    //=======================================================

    int all_vertices_count = mesh->position.size();
    int vertices_line_count = sphereSegmentX + 1;
    int line_start_pos = vertices_line_count;

    for (int i = 0; i < sphereSegmentX ; i++) {
        mesh->indices.push_back(line_start_pos + i + 1);
        mesh->indices.push_back(line_start_pos + i);
        mesh->indices.push_back(i);
    } 
    for (int j = 0; j < sphereSegmentY - 1; j++) {
        line_start_pos = (j + 1)  * vertices_line_count;
        for (int i = 0; i < sphereSegmentX; i++) {

            mesh->indices.push_back(line_start_pos + i);
            mesh->indices.push_back(line_start_pos + i + vertices_line_count + 1);
            mesh->indices.push_back(line_start_pos + i + vertices_line_count);
            
            mesh->indices.push_back(line_start_pos + i + vertices_line_count + 1);
            mesh->indices.push_back(line_start_pos + i);
            mesh->indices.push_back(line_start_pos + i + 1);
        }
    }
    
    line_start_pos = vertices_line_count * sphereSegmentY;
    for (int i = 0; i < sphereSegmentX; i++) {
        mesh->indices.push_back(line_start_pos + i);
        mesh->indices.push_back(line_start_pos + i + 1);
        mesh->indices.push_back(line_start_pos + i + vertices_line_count);
    }  

    //创建缓冲区
    mesh->GenerateBuffer();
}
void CCPanoramaRenderer::CreateMainModel() {
    mainModel = new CCModel();
    mainModel->Mesh = new CCMesh();
    mainModel->Material = new CCMaterial(panoramaCheckTex);
    mainModel->Material->tilling = glm::vec2(50.0f, 25.0f);

    CreateMainModelSphereMesh(mainModel->Mesh);
}
void CCPanoramaRenderer::CreateFullModelSphereMesh(CCMesh* mesh, int chunkW, int chunkH, int currentChuntX, int currentChuntY) {

    float r = 0.99f;
    float ustep = 1.0f / sphereFullSegmentX, vstep = 1.0f / sphereFullSegmentY;
    float u = 0, v = 0, cu = 0, cv = 0;

    int segXStart = currentChuntX / chunkW;
    int segYStart = currentChuntY / chunkH;
    int segXEnd = segXStart + sphereFullSegmentX / chunkW;
    int segYEnd = segXStart + sphereFullSegmentY / chunkW;

    float u_start = (float)currentChuntX / (float)chunkW;
    u = u_start;
    v = (float)currentChuntY / (float)chunkH;

    for (int j = segYStart; j <= segYEnd; j++, v += vstep, cv += vstep) {
        u = u_start;
        cu = 0;
        for (int i = segXStart; i <= segXEnd; i++, u += ustep, cu += vstep) {
            mesh->position.push_back(GetSpherePoint(u, v, r));
            mesh->texCoords.push_back(glm::vec2(cu, cv));
        }
    }

    int all_vertices_count = mesh->position.size();
    int vertices_line_count = (segYEnd - segYStart) + 1;
    int line_start_pos = vertices_line_count;

    if (segYStart == 0) {
        for (int i = segXStart; i < segXEnd; i++) {
            mesh->indices.push_back(line_start_pos + i + 1);
            mesh->indices.push_back(line_start_pos + i);
            mesh->indices.push_back(i);
        }
    }

    for (int j = segYStart; j < segYEnd - 1; j++) {
        line_start_pos = (j + 1 - segYStart) * vertices_line_count;
        for (int i = segXStart; i < segXEnd; i++) {

            mesh->indices.push_back(line_start_pos + i);
            mesh->indices.push_back(line_start_pos + i + vertices_line_count + 1);
            mesh->indices.push_back(line_start_pos + i + vertices_line_count);

            mesh->indices.push_back(line_start_pos + i + vertices_line_count + 1);
            mesh->indices.push_back(line_start_pos + i);
            mesh->indices.push_back(line_start_pos + i + 1);
        }
    }

    if (segYEnd == 0) {
        line_start_pos = vertices_line_count * (segYEnd - segYStart) ;
        for (int i = segXStart; i < segXEnd; i++) {
            mesh->indices.push_back(line_start_pos + i);
            mesh->indices.push_back(line_start_pos + i + 1);
            mesh->indices.push_back(line_start_pos + i + vertices_line_count);
        }
    }

    //创建缓冲区
    mesh->GenerateBuffer();
}
void CCPanoramaRenderer::LoadBuiltInResources() {
    panoramaCheckTex = new CCTexture();
    if (!panoramaCheckTex->Load(CCFileManager::GetResourcePath(L"textures", L"checker.jpg").c_str())) {
        delete panoramaCheckTex;
        panoramaCheckTex = nullptr;
    }
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

void CCPanoramaRenderer::ReleaseFullModel()
{
    if (fullModels.size() > 0) {
        std::vector<ChunkModel*>::iterator it;
        for (it = fullModels.begin(); it != fullModels.end(); it++) {
            ChunkModel* m = *it;
            delete m->model;
            delete m;
        }
        fullModels.clear();
    }
}
void CCPanoramaRenderer::GenerateFullModel(int chunkW, int chunkH)
{
    for (int i = 0; i < chunkW; i++) {
        for (int j = 0; j < chunkH; j++) {
            ChunkModel* model = new ChunkModel();
            model->model = new CCModel();
            model->model->Mesh = new CCMesh();
            model->chunkX = i;
            model->chunkY = j;
            model->chunkXv = (float)i / (float)chunkW;
            model->chunkYv = (float)j / (float)chunkH;
            CreateFullModelSphereMesh(model->model->Mesh, chunkW, chunkH, i, j);
            fullModels.push_back(model);
        }
    }
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

void CCPanoramaRenderer::UpdateMainModelTex()
{
    if (panoramaThumbnailTex) {
        mainModel->Material->diffuse = panoramaThumbnailTex;
        mainModel->Material->tilling = glm::vec2(1.0f);
    }
    else {
        mainModel->Material->diffuse = panoramaCheckTex;
        mainModel->Material->tilling = glm::vec2(50.0f);
    }
}

void CCPanoramaRenderer::RenderThumbnail(bool wireframe)
{
    glPolygonMode(GL_FRONT, wireframe ? GL_LINE : GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);

    //绘制
    mainModel->Render();
}
void CCPanoramaRenderer::RenderFullChunks()
{
    auto rotate = mainModel->Rotation;
    float x = glm::abs(rotate.x) / 360.0f;
    float y = 90.0f + rotate.y / 90.0f;

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
