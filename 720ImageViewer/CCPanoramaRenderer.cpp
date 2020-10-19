#include "CCPanoramaRenderer.h"
#include "CApp.h"
#include "COpenGLRenderer.h"
#include "CGameRenderer.h"
#include "CGameRenderer.h"
#include "CCMesh.h"
#include "CCMeshLoader.h"
#include "CCMaterial.h"
#include "CCRenderGlobal.h"

CCPanoramaRenderer::CCPanoramaRenderer(CGameRenderer* renderer)
{
    Renderer = renderer;
    logger = CApp::Instance->GetLogger();
}

void CCPanoramaRenderer::Init()
{
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

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
    if (panoramaRedCheckTex != nullptr) {
        delete panoramaRedCheckTex;
        panoramaRedCheckTex = nullptr;
    }
    ReleaseTexPool();
}

void CCPanoramaRenderer::Render(float deltaTime)
{
    CCRenderGlobal::SetInstance(globalRenderInfo);
    CCTexture::UnUse();
    shader->Use();

    //摄像机矩阵
    Renderer->View->CalcMainCameraProjection(shader);

    //模型位置和矩阵映射
    model = mainModel->GetMatrix();
    glUniformMatrix4fv(globalRenderInfo->modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //绘制外层缩略图
    glUniform1i(globalRenderInfo->useColorLoc, 0);
    if(!renderNoPanoramaSmall) RenderThumbnail();

    //绘制区块式完整全景球
    if (renderPanoramaFull)
        RenderFullChunks(deltaTime);

    //绘制测试
    if (renderPanoramaFullTest) {
        glPolygonMode(GL_FRONT, GL_LINE);
        glPolygonMode(GL_BACK, GL_LINE);
        glUniform1i(globalRenderInfo->useColorLoc, 1);
        glUniform3f(globalRenderInfo->ourColorLoc, wireframeColor2.r, wireframeColor2.g, wireframeColor2.b);
        glColor3f(wireframeColor2.r, wireframeColor2.g, wireframeColor2.b);
        RenderFullChunks(deltaTime);
    }
    if (renderPanoramaATest && testModel) 
        testModel->Render();

    //绘制调试线框
    if (renderDebugWireframe) {
        glUniform1i(globalRenderInfo->useColorLoc, 1);
        glUniform3f(globalRenderInfo->ourColorLoc, wireframeColor.r, wireframeColor.g, wireframeColor.b);
        glColor3f(wireframeColor.r, wireframeColor.g, wireframeColor.b);
        RenderThumbnail(true);
    }
    //绘制向量标线
    if (renderDebugVector) {

        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(globalRenderInfo->useColorLoc, 1);

      
        glUniform3f(globalRenderInfo->ourColorLoc, 0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.05f, 0.0f);
        glEnd();

        glUniform3f(globalRenderInfo->ourColorLoc, 0.0f, 0.0f, 1.0f);

        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.05f);
        glEnd();

        glUniform3f(globalRenderInfo->ourColorLoc, 1.0f, 0.0f, 0.0f);

        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.05f, 0.0f, 0.0f);
        glEnd();
    }
}

void CCPanoramaRenderer::CreateMainModel() {
    mainModel = new CCModel();
    mainModel->Mesh = new CCMesh();
    mainModel->Material = new CCMaterial(panoramaCheckTex);
    mainModel->Material->tilling = glm::vec2(50.0f, 25.0f);

    CreateMainModelSphereMesh(mainModel->Mesh);
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
            mesh->positions.push_back(GetSpherePoint(u, v, r));
            mesh->texCoords.push_back(glm::vec2(u, v));
        }
    }

    //顶点索引
    //=======================================================

    int all_vertices_count = mesh->positions.size();
    int vertices_line_count = sphereSegmentX + 1;
    int line_start_pos = vertices_line_count;

    for (int i = 0; i < sphereSegmentX; i++) {
        mesh->indices.push_back(CCFace(line_start_pos + i + 1, 0, -1));
        mesh->indices.push_back(CCFace(line_start_pos + i, 0, -1));
        mesh->indices.push_back(CCFace(i, 0, -1));
    }
    for (int j = 0; j < sphereSegmentY - 1; j++) {
        line_start_pos = (j + 1) * vertices_line_count;
        for (int i = 0; i < sphereSegmentX; i++) {

            mesh->indices.push_back(CCFace(line_start_pos + i, 0, -1));
            mesh->indices.push_back(CCFace(line_start_pos + i + vertices_line_count + 1, 0, -1));
            mesh->indices.push_back(CCFace(line_start_pos + i + vertices_line_count, 0, -1));

            mesh->indices.push_back(CCFace(line_start_pos + i + vertices_line_count + 1, 0, -1));
            mesh->indices.push_back(CCFace(line_start_pos + i, 0, -1));
            mesh->indices.push_back(CCFace(line_start_pos + i + 1, 0, -1));
        }
    }

    line_start_pos = vertices_line_count * sphereSegmentY;
    for (int i = 0; i < sphereSegmentX; i++) {
        mesh->indices.push_back(CCFace(line_start_pos + i, 0, -1));
        mesh->indices.push_back(CCFace(line_start_pos + i + 1, 0, -1));
        mesh->indices.push_back(CCFace(line_start_pos + i + vertices_line_count, 0, -1));
    }

    //创建缓冲区
    mesh->GenerateBuffer();
}
glm::vec3 CCPanoramaRenderer::CreateFullModelSphereMesh(ChunkModel*info, int segXStart, int segYStart, int segX, int segY) {

    CCMesh* mesh = info->model->Mesh;

    float r = 0.98f;
    float ustep = 1.0f / sphereFullSegmentX, vstep = 1.0f / sphereFullSegmentY;
    float u = 0, v = 0, cu = 0, cv = 0;

    int segXEnd = segXStart + segX;
    int segYEnd = segYStart + segY;

    float u_start = segXStart * ustep, v_start = segYStart * vstep;
    float custep = 1.0f / (segXEnd - segXStart), cvstep = 1.0f / (segYEnd - segYStart);
    u = u_start;
    v = v_start;

    int skip = 0;

    for (int j = segYStart; j <= segYEnd; j++, v += vstep, cv += cvstep) {
        if (j <= 2 || j >= sphereFullSegmentY - 2) {
            skip++;
            continue;
        }
        u = u_start;
        cu = 0;
        for (int i = segXStart; i <= segXEnd; i++, u += ustep, cu += custep) {
            mesh->positions.push_back(GetSpherePoint(u, v, r));
            mesh->texCoords.push_back(glm::vec2(cu, cv));
        }
    }

    int vertices_line_count = segXEnd - segXStart + 1;
    for (int j = 0, c = segYEnd - segYStart - skip; j < c; j++) {
        int line_start_pos = (j) * vertices_line_count;
        for (int i = 0; i < segXEnd - segXStart; i++) {

            mesh->indices.push_back(CCFace(line_start_pos + i, 0, -1));
            mesh->indices.push_back(CCFace(line_start_pos + i + vertices_line_count + 1, 0, -1));
            mesh->indices.push_back(CCFace(line_start_pos + i + vertices_line_count, 0, -1));

            mesh->indices.push_back(CCFace(line_start_pos + i + vertices_line_count + 1, 0, -1));
            mesh->indices.push_back(CCFace(line_start_pos + i, 0, -1));
            mesh->indices.push_back(CCFace(line_start_pos + i + 1, 0, -1));
        }
    }

    //创建缓冲区
    mesh->GenerateBuffer();

    float center_u = (u_start + ustep * ((segXEnd - segXStart) / 2.0f)),
        center_v = (v_start + vstep * ((segYEnd - segYStart) / 2.0f)),
        center_u_2 = (center_u - u_start) / 10.0f * 9.0f,
        center_v_2 = (center_v - v_start) / 10.0f * 9.0f;

    info->pointA = GetSpherePoint(center_u - center_u_2, center_v - center_v_2, r);
    info->pointB = GetSpherePoint(center_u + center_u_2, center_v - center_v_2, r);
    info->pointC = GetSpherePoint(center_u - center_u_2, center_v + center_v_2, r);
    info->pointD = GetSpherePoint(center_u + center_u_2, center_v + center_v_2, r);

    return GetSpherePoint(center_u, center_v, r);
}
void CCPanoramaRenderer::LoadBuiltInResources() {
    panoramaCheckTex = new CCTexture();
    if (!panoramaCheckTex->Load(CCFileManager::GetResourcePath(L"textures", L"checker.jpg").c_str())) {
        delete panoramaCheckTex;
        panoramaCheckTex = nullptr;
    }
    panoramaRedCheckTex = new CCTexture();
    panoramaRedCheckTex->Load(CCFileManager::GetResourcePath(L"textures", L"red_checker.jpg").c_str());

    testModel = new CCModel();
    testModel->Mesh = new CCMesh();

    CCMeshLoader::GetMeshLoaderByType(MeshTypeObj)->Load(
        CCFileManager::GetResourcePath(L"prefabs", L"teapot.obj").c_str(),
        testModel->Mesh);

    testModel->Material = new CCMaterial(panoramaRedCheckTex);
    testModel->Material->tilling = glm::vec2(50.0f);
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

//完整全景模型

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
    panoramaFullSplitW = chunkW;
    panoramaFullSplitH = chunkH;

    int segX = (int)ceil((float)sphereFullSegmentX / (float)chunkW);
    int segY = (int)ceil((float)sphereFullSegmentY / (float)chunkH);

    float chunkWf = 1.0f / chunkW, chunkHf = 1.0f / chunkH;
    for (int i = 0; i < chunkW; i++) {
        for (int j = 0; j < chunkH; j++) {
            ChunkModel* model = new ChunkModel();
            model->model = new CCModel();
            model->model->Mesh = new CCMesh();
            model->model->Material = new CCMaterial(panoramaRedCheckTex);
            model->model->Material->tilling = glm::vec2(50.0f);
            model->chunkX = i;
            model->chunkY = j;
            model->chunkXv = (float)i / (float)chunkW;
            model->chunkYv = (float)j / (float)chunkH;
            model->chunkXv = model->chunkXv + chunkWf;
            model->chunkYv = model->chunkYv + chunkHf;
            model->pointCenter = CreateFullModelSphereMesh(model, i * segX, j * segY, segX, segY);
            fullModels.push_back(model);
        }
    }
}

//模型控制

void CCPanoramaRenderer::ResetModel()
{
    mainModel->Reset();
}
void CCPanoramaRenderer::RotateModel(float xoffset, float yoffset)
{
    mainModel->Rotation.y += xoffset;
    mainModel->Rotation.z -= yoffset;
    mainModel->UpdateVectors();

    UpdateFullChunksVisible();
}

//渲染

void CCPanoramaRenderer::RenderThumbnail(bool wireframe)
{
    glPolygonMode(GL_FRONT, wireframe ? GL_LINE : GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);

    //绘制
    mainModel->Render();
}
void CCPanoramaRenderer::RenderFullChunks(float deltaTime)
{
    if (renderPanoramaFullTest && !renderPanoramaFullRollTest) {
        renderPanoramaFullTestTime += deltaTime;
        if (renderPanoramaFullTestTime > 1) {
            renderPanoramaFullTestTime = 0;
            if (renderPanoramaFullTestIndex < (int)fullModels.size() - 1)renderPanoramaFullTestIndex++;
            else renderPanoramaFullTestIndex = 0;
        }
        fullModels[renderPanoramaFullTestIndex]->model->Render();
    }
    else {
        for (int i = 0, c = fullModels.size(); i < c; i++) {
            ChunkModel* m = fullModels[i];
            if (m->model->Visible)  //渲染区块
                m->model->Render();
        }
    }
}

//更新

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
void CCPanoramaRenderer::UpdateFullChunksVisible() {
    float fov = Renderer->View->Camera->FiledOfView;
    for (auto it = fullModels.begin(); it != fullModels.end(); it++) {
        ChunkModel* m = *it;
        if (fov > 50)
            m->model->Visible = IsInView(m->pointCenter);
        else
            m->model->Visible = IsInView(m->pointA) || IsInView(m->pointB) || IsInView(m->pointC) || IsInView(m->pointD);
        if (m->model->Visible) {
            if (!m->loadMarked && !renderPanoramaFullTest) {//加载贴图
                m->loadMarked = true;

                logger->Log(L"Star load chunk %d, %d", m->chunkX, m->chunkY);

                CCTexture* tex = new CCTexture();
                tex->wrapS = GL_MIRRORED_REPEAT;
                tex->wrapT = GL_MIRRORED_REPEAT;
                m->model->Material->diffuse = tex;
                m->model->Material->tilling = glm::vec2(1.0f, 1.0f);
                Renderer->AddTextureToQueue(tex, m->chunkX, m->chunkY, m->chunkY * m->chunkX + m->chunkX);//MainTex
                panoramaTexPool.push_back(tex);
            }
        }
    }
}

bool CCPanoramaRenderer::IsInView(glm::vec3 worldPos)
{
    CCamera* cam = Renderer->View->Camera;
    glm::vec3 viewPos = cam->World2Screen(worldPos, model);

    //glm::vec3 dir = glm::normalize(worldPos - cam->Position);
    //float dot = glm::dot(cam->Front, dir);     //判断物体是否在相机前面  

    if (/*dot > 0 && */viewPos.x >= 0 && viewPos.x <= Renderer->View->Width && viewPos.y >= 0 && viewPos.y <= Renderer->View->Height)
        return true;
    else
        return false;
}
glm::vec2 CCPanoramaRenderer::GetSphereUVPoint(float u, float v, short i) {
    return glm::vec2(u, v);
}
glm::vec3 CCPanoramaRenderer::GetSpherePoint(float u, float v, float r)
{
    constexpr float PI = glm::pi<float>();
    float x = r * glm::sin(PI * v) * glm::sin(PI * u * 2);
    float y = r * glm::cos(PI * v);
    float z = r * glm::sin(PI * v) * glm::cos(PI * u * 2);
    return glm::vec3(x, y, z);
}
