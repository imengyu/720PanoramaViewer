#include "CCPanoramaRenderer.h"
#include "COpenGLRenderer.h"
#include "COpenGLView.h"
#include "CCFileManager.h"
#include "CCAssetsManager.h"
#include "CCMesh.h"
#include "CCMeshLoader.h"
#include "CCMaterial.h"
#include "CCRenderGlobal.h"
#include "CWindowsGameRenderer.h"

CCPanoramaRenderer::CCPanoramaRenderer(CWindowsGameRenderer* renderer)
{
    Renderer = renderer;
    logger = Logger::GetStaticInstance();
}

void CCPanoramaRenderer::Init()
{
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glDisable(GL_DITHER);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);

    LoadBuiltInResources();

    shader = new CCShader(
        CCAssetsManager::GetResourcePath(L"shader", L"Standard_vertex.glsl").c_str(),
        CCAssetsManager::GetResourcePath(L"shader", L"Standard_fragment.glsl").c_str());

    CreateMainModel();

    globalRenderInfo = new CCRenderGlobal();

    globalRenderInfo->glVendor = (GLubyte*)glGetString(GL_VENDOR);            //���ظ���ǰOpenGLʵ�ֳ��̵�����
    globalRenderInfo->glRenderer = (GLubyte*)glGetString(GL_RENDERER);    //����һ����Ⱦ����ʶ����ͨ���Ǹ�Ӳ��ƽ̨
    globalRenderInfo->glVersion = (GLubyte*)glGetString(GL_VERSION);    //���ص�ǰOpenGLʵ�ֵİ汾��
    globalRenderInfo->glslVersion = (GLubyte*)glGetString(GL_SHADING_LANGUAGE_VERSION);//������ɫԤѹ�������汾��

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
    if (mainFlatModel != nullptr) {
        delete mainFlatModel;
        mainFlatModel = nullptr;
    }
    ReleaseTexPool();
}

void CCPanoramaRenderer::Render(float deltaTime)
{
    CCRenderGlobal::SetInstance(globalRenderInfo);
    CCTexture::UnUse();
    shader->Use();

    //���������
    Renderer->View->CalcMainCameraProjection(shader);

    //ģ��λ�ú;���ӳ��
    model = mainModel->GetMatrix();
    glUniformMatrix4fv(globalRenderInfo->modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);

    //��������
    glUniform1i(globalRenderInfo->useColorLoc, 0);
    if (renderOn) {
        //�����������ͼ
        if (!renderNoPanoramaSmall) RenderThumbnail();

        //��������ʽ����ȫ����
        if (renderPanoramaFull)
            RenderFullChunks(deltaTime);

        if (renderPanoramaFlat)
            RenderFlat();
    }

    //���Ʋ���
    if (renderPanoramaFullTest) {
        glPolygonMode(GL_FRONT, GL_LINE);
        glPolygonMode(GL_BACK, GL_LINE);
        glUniform1i(globalRenderInfo->useColorLoc, 1);
        glUniform3f(globalRenderInfo->ourColorLoc, wireframeColor2.r, wireframeColor2.g, wireframeColor2.b);
        glColor3f(wireframeColor2.r, wireframeColor2.g, wireframeColor2.b);
        RenderFullChunks(deltaTime);
    }

    //���Ƶ����߿�
    if (renderDebugWireframe) {

        glPolygonMode(GL_FRONT, GL_LINE);
        glPolygonMode(GL_BACK, GL_LINE);
        glUniform1i(globalRenderInfo->useColorLoc, 1);
        glUniform3f(globalRenderInfo->ourColorLoc, wireframeColor.r, wireframeColor.g, wireframeColor.b);

        if (renderPanoramaFlat)
            RenderFlat();
        else
            RenderThumbnail();
    }
    //������������
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

    CreateMainModelSphereMesh(mainModel->Mesh.GetPtr());

    mainFlatModel = new CCModel();
    mainFlatModel->Mesh = new CCMesh();
    mainFlatModel->Mesh->DrawType = GL_DYNAMIC_DRAW;
    mainFlatModel->Material = new CCMaterial(panoramaCheckTex);
    mainFlatModel->Material->tilling = glm::vec2(50.0f, 25.0f);

    CreateMainModelFlatMesh(mainFlatModel->Mesh.GetPtr());
}
void CCPanoramaRenderer::CreateMainModelFlatMesh(CCMesh* mesh) const {
    mesh->normals.clear();
    mesh->positions.clear();
    mesh->texCoords.clear();
    mesh->indices.clear();

    float ustep = 1.0f / (float)sphereSegmentX, vstep = 1.0f / (float)sphereSegmentY;
    float u, v = 0;

    for (int j = 0; j <= sphereSegmentY; j++) {
        v += vstep;
        u = 0;
        for (int i = 0; i <= sphereSegmentX; i++) {
            u += ustep;
            mesh->positions.emplace_back(glm::vec3(0.5f - u, (0.5f - v) / 2.0f, 0.0f));
            mesh->texCoords.emplace_back(glm::vec2(1.0f - u, v));
        }
    }


    int vertices_line_count = sphereSegmentX + 1;
    for (int j = 0, c = sphereSegmentY; j < c; j++) {
        int line_start_pos = (j)*vertices_line_count;
        for (int i = 0; i < sphereSegmentX; i++) {

            mesh->indices.emplace_back(CCFace(line_start_pos + i, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i + vertices_line_count + 1, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i + vertices_line_count, 0, -1));

            mesh->indices.emplace_back(CCFace(line_start_pos + i + vertices_line_count + 1, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i + 1, 0, -1));
        }
    }

    //����������
    mesh->GenerateBuffer();
}
void CCPanoramaRenderer::CreateMainModelSphereMesh(CCMesh* mesh) const {

    float r = 1.0f;
    float ustep = 1.0f / (float)sphereSegmentX, vstep = 1.0f / (float)sphereSegmentY;
    float u, v = 0;

    //����
    //=======================================================

    for (int j = 0; j <= sphereSegmentY; j++) {
        v += vstep;
        u = 0;
        for (int i = 0; i <= sphereSegmentX; i++) {
            u += ustep;
            mesh->positions.push_back(GetSpherePoint(u, v, r));
            mesh->texCoords.emplace_back(glm::vec2(1.0f - u, v));
        }
    }

    //��������
    //=======================================================

    int vertices_line_count = sphereSegmentX + 1;
    int line_start_pos = vertices_line_count;

    for (int i = 0; i < sphereSegmentX; i++) {
        mesh->indices.emplace_back(CCFace(line_start_pos + i + 1, 0, -1));
        mesh->indices.emplace_back(CCFace(line_start_pos + i, 0, -1));
        mesh->indices.emplace_back(CCFace(i, 0, -1));
    }
    for (int j = 0; j < sphereSegmentY - 1; j++) {
        line_start_pos = (j + 1) * vertices_line_count;
        for (int i = 0; i < sphereSegmentX; i++) {

            mesh->indices.emplace_back(CCFace(line_start_pos + i, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i + vertices_line_count + 1, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i + vertices_line_count, 0, -1));

            mesh->indices.emplace_back(CCFace(line_start_pos + i + vertices_line_count + 1, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i + 1, 0, -1));
        }
    }

    line_start_pos = vertices_line_count * sphereSegmentY;
    for (int i = 0; i < sphereSegmentX; i++) {
        mesh->indices.emplace_back(CCFace(line_start_pos + i, 0, -1));
        mesh->indices.emplace_back(CCFace(line_start_pos + i + 1, 0, -1));
        mesh->indices.emplace_back(CCFace(line_start_pos + i + vertices_line_count, 0, -1));
    }

    //����������
    mesh->GenerateBuffer();
}
glm::vec3 CCPanoramaRenderer::CreateFullModelSphereMesh(ChunkModel* info, int segXStart, int segYStart, int segX, int segY) const {

    CCMesh* mesh = info->model->Mesh.GetPtr();

    float r = 0.99f;
    float ustep = 1.0f / (float)sphereFullSegmentX, vstep = 1.0f / (float)sphereFullSegmentY;
    float u, v, cu, cv = 0;

    int segXEnd = segXStart + segX;
    int segYEnd = segYStart + segY;

    float u_start = (float)segXStart * ustep, v_start = (float)segYStart * vstep;
    float custep = 1.0f / (float)(segXEnd - segXStart), cvstep = 1.0f / (float)(segYEnd - segYStart);
    v = v_start;

    int skip = 0;

    for (int j = segYStart; j <= segYEnd; j++) {
        v += vstep;
        cv += cvstep;
        if (j <= 2 || j >= sphereFullSegmentY - 4) {
            skip++;
            continue;
        }
        u = u_start;
        cu = 1.0f;
        for (int i = segXStart; i <= segXEnd; i++) {
            u += ustep;
            cu -= custep;
            mesh->positions.push_back(GetSpherePoint(u, v, r));
            mesh->texCoords.emplace_back(glm::vec2(cu, cv));
        }
    }

    int vertices_line_count = segXEnd - segXStart + 1;
    for (int j = 0, c = segYEnd - segYStart - skip; j < c; j++) {
        int line_start_pos = (j)*vertices_line_count;
        for (int i = 0; i < segXEnd - segXStart; i++) {

            mesh->indices.emplace_back(CCFace(line_start_pos + i, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i + vertices_line_count + 1, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i + vertices_line_count, 0, -1));

            mesh->indices.emplace_back(CCFace(line_start_pos + i + vertices_line_count + 1, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i, 0, -1));
            mesh->indices.emplace_back(CCFace(line_start_pos + i + 1, 0, -1));
        }
    }

    //����������
    mesh->GenerateBuffer();

    float center_u = (u_start + ustep * ((float)(segXEnd - segXStart) / 2.0f)),
        center_v = (v_start + vstep * ((float)(segYEnd - segYStart) / 2.0f)),
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
    if (!panoramaCheckTex->Load(CCAssetsManager::GetResourcePath(L"textures", L"checker.jpg").c_str()))
        panoramaCheckTex = nullptr;

    panoramaRedCheckTex = new CCTexture();
    panoramaRedCheckTex->Load(CCAssetsManager::GetResourcePath(L"textures", L"red_checker.jpg").c_str());


    uiLogoTex = new CCTexture();
    uiLogoTex->Load(CCAssetsManager::GetResourcePath(L"textures", L"logo.png").c_str());

    uiFailedTex = new CCTexture();
    uiFailedTex->Load(CCAssetsManager::GetResourcePath(L"textures", L"icon_image_error.jpg").c_str());

    uiOpenButtonTex = new CCTexture();
    uiOpenButtonTex->Load(CCAssetsManager::GetResourcePath(L"textures", L"open_file.jpg").c_str());

    uiTitleTex = new CCTexture();
    uiTitleTex->Load(CCAssetsManager::GetResourcePath(L"textures", L"title.png").c_str());

}
void CCPanoramaRenderer::ReleaseTexPool() {
    renderPanoramaFull = false;
    if (!panoramaTexPool.empty()) {
        for (auto& it : panoramaTexPool)
            it.ForceRelease();
        panoramaTexPool.clear();
    }
    panoramaThumbnailTex = nullptr;
}

//����ȫ��ģ��

void CCPanoramaRenderer::ReleaseFullModel()
{
    if (!fullModels.empty()) {
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

    int segX = (int)floor((float)sphereFullSegmentX / (float)chunkW);
    int segY = (int)floor((float)sphereFullSegmentY / (float)chunkH);

    float chunkWf = 1.0f / (float)chunkW, chunkHf = 1.0f / (float)chunkH;
    for (int i = 0; i < chunkW; i++) {
        for (int j = 0; j < chunkH; j++) {
            auto* pChunkModel = new ChunkModel();
            pChunkModel->model = new CCModel();
            pChunkModel->model->Visible = false;
            pChunkModel->model->Mesh = new CCMesh();
            pChunkModel->model->Material = new CCMaterial(panoramaRedCheckTex);
            pChunkModel->model->Material->tilling = glm::vec2(50.0f);
            pChunkModel->chunkX = chunkW - i - 1;
            pChunkModel->chunkY = j;
            pChunkModel->chunkXv = (float)i / (float)chunkW;
            pChunkModel->chunkYv = (float)j / (float)chunkH;
            pChunkModel->chunkXv = pChunkModel->chunkXv + chunkWf;
            pChunkModel->chunkYv = pChunkModel->chunkYv + chunkHf;
            pChunkModel->pointCenter = CreateFullModelSphereMesh(pChunkModel,
                i * segX, j * segY,
                segX + ((i == chunkW - 1 && chunkW % 2 != 0) ? 1 : 0), segY);
            fullModels.push_back(pChunkModel);
        }
    }
}

//ģ�Ϳ���

void CCPanoramaRenderer::ResetModel() const
{
    mainModel->Reset();
    mainFlatModel->Material->offest.x = 0.0f;
}
void CCPanoramaRenderer::RotateModel(float xoffset, float yoffset)
{
    mainModel->Rotation.y += xoffset;
    mainModel->Rotation.z -= yoffset;
    mainModel->UpdateVectors();

    UpdateFullChunksVisible();
}
void CCPanoramaRenderer::RotateModelForce(float y, float z)
{
    mainModel->Rotation.y += y;
    mainModel->Rotation.z += z;
    mainModel->UpdateVectors();

    UpdateFullChunksVisible();
}
void CCPanoramaRenderer::MoveModel(float xoffset, float yoffset) const
{
    if (renderPanoramaFlatXLoop) {
        float v = mainFlatModel->Material->offest.x + xoffset;
        if (v < 0) v += 1.0f;
        else if (v > 1.0f) v -= 1.0f;
        mainFlatModel->Material->offest.x = v;
    }
    else {
        mainModel->Position.x -= xoffset * FlatModelMoveRato;
        if (mainModel->Position.x < FlatModelMin.x) mainModel->Position.x = FlatModelMin.x;
        if (mainModel->Position.x > FlatModelMax.x) mainModel->Position.x = FlatModelMax.x;
    }

    mainModel->Position.y -= yoffset * FlatModelMoveRato;
    if (mainModel->Position.y < FlatModelMin.y) mainModel->Position.y = FlatModelMin.y;
    if (mainModel->Position.y > FlatModelMax.y) mainModel->Position.y = FlatModelMax.y;
}
void CCPanoramaRenderer::MoveModelForce(float x, float y) const
{
    if (renderPanoramaFlatXLoop) {
        float v = mainFlatModel->Material->offest.x + x;
        if (v < 0) v += 1.0f;
        else if (v > 1.0f) v -= 1.0f;
        mainFlatModel->Material->offest.x = v;
    }
    else {
        mainModel->Position.x -= x * FlatModelMoveRato;
        if (mainModel->Position.x < FlatModelMin.x) mainModel->Position.x = FlatModelMin.x;
        if (mainModel->Position.x > FlatModelMax.x) mainModel->Position.x = FlatModelMax.x;
    }

    mainModel->Position.y += y * FlatModelMoveRato;
    if (mainModel->Position.y < FlatModelMin.y) mainModel->Position.y = FlatModelMin.y;
    if (mainModel->Position.y > FlatModelMax.y) mainModel->Position.y = FlatModelMax.y;
}
void CCPanoramaRenderer::UpdateMercatorControl() {
    //PrecalcMercator();

    CCMesh* mesh = mainFlatModel->Mesh.GetPtr();
    mesh->texCoords.clear();

    float ustep = 1.0f / (float)sphereSegmentX, vstep = 1.0f / (float)sphereSegmentY;
    float u, v = 0;

    for (int j = 0; j <= sphereSegmentY; j++) {
        v += vstep;
        u = 0;
        for (int i = 0; i <= sphereSegmentX; i++) {
            u += ustep;
            mesh->texCoords.push_back(GetMercatorUVPoint(1.0f - u, v));
        }
    }

    mesh->ReBufferData();
}
void CCPanoramaRenderer::ResetMercatorControl() const {
    CCMesh* mesh = mainFlatModel->Mesh.GetPtr();
    mesh->texCoords.clear();

    float ustep = 1.0f / (float)sphereSegmentX, vstep = 1.0f / (float)sphereSegmentY;
    float u, v = 0;

    for (int j = 0; j <= sphereSegmentY; j++) {
        v += vstep;
        u = 0;
        for (int i = 0; i <= sphereSegmentX; i++) {
            u += ustep;
            mesh->texCoords.emplace_back(1.0 - u, v);
        }
    }
    mesh->ReBufferData();
}

//��Ⱦ

void CCPanoramaRenderer::RenderThumbnail() const
{
    mainModel->Render();
}
void CCPanoramaRenderer::RenderFullChunks(float deltaTime)
{
    if (renderPanoramaFullTest && !renderPanoramaFullRollTest) {
        renderPanoramaFullTestTime += deltaTime;
        if (renderPanoramaFullTestAutoLoop) {
            if (renderPanoramaFullTestTime > 1) {
                renderPanoramaFullTestTime = 0;
                if (renderPanoramaFullTestIndex < (int)fullModels.size() - 1)renderPanoramaFullTestIndex++;
                else renderPanoramaFullTestIndex = 0;
            }
        }
        fullModels[renderPanoramaFullTestIndex]->model->Render();
    }
    else {
        for (auto m : fullModels) {
            if (m->model->Visible)  //��Ⱦ����
                m->model->Render();
        }
    }
}
void CCPanoramaRenderer::RenderFlat() const {
    mainFlatModel->Render();
}

//����

void CCPanoramaRenderer::UpdateMainModelTex() const
{
    if (!panoramaThumbnailTex.IsNullptr()) {
        mainModel->Material->diffuse = panoramaThumbnailTex;
        mainModel->Material->tilling = glm::vec2(1.0f);
        mainFlatModel->Material->diffuse = panoramaThumbnailTex;
        mainFlatModel->Material->tilling = glm::vec2(1.0f);
    }
    else {
        mainModel->Material->diffuse = panoramaCheckTex;
        mainModel->Material->tilling = glm::vec2(50.0f);
        mainFlatModel->Material->diffuse = panoramaCheckTex;
        mainFlatModel->Material->tilling = glm::vec2(50.0f);
    }
}
void CCPanoramaRenderer::UpdateFullChunksVisible() {
    if (renderPanoramaFull || renderPanoramaFullTest) {
        float fov = Renderer->View->Camera->FiledOfView;
        for (auto m : fullModels) {
            if (fov > 50)
                m->model->Visible = IsInView(m->pointCenter);
            else
                m->model->Visible = IsInView(m->pointA) || IsInView(m->pointB) || IsInView(m->pointC) || IsInView(m->pointD);
            if (m->model->Visible) {
                if (!m->loadMarked && !renderPanoramaFullTest) {//������ͼ
                    m->loadMarked = true;

                    logger->Log(L"Star load chunk %d, %d", m->chunkX, m->chunkY);

                    auto* tex = new CCTexture();
                    tex->wrapS = GL_MIRRORED_REPEAT;
                    tex->wrapT = GL_MIRRORED_REPEAT;
                    m->model->Material->diffuse = tex;
                    m->model->Material->tilling = glm::vec2(1.0f, 1.0f);
                    Renderer->AddTextureToQueue(tex, m->chunkX, m->chunkY, m->chunkY * m->chunkX + m->chunkX);//MainTex
                    panoramaTexPool.emplace_back(tex);
                }
            }
        }
    }
}
void CCPanoramaRenderer::UpdateFlatModelMinMax(float orthoSize) {
    FlatModelMin = glm::vec2(-((1.0f - orthoSize) / 2.0f), -((1.0f - orthoSize) / 4.0f));
    FlatModelMax = glm::vec2((1.0f - orthoSize) / 2.0f, (1.0f - orthoSize) / 4.0f);
    FlatModelMoveRato = orthoSize / 1.0f;
    MoveModelForce(0, 0);
}

bool CCPanoramaRenderer::IsInView(glm::vec3 worldPos)
{
    CCamera* cam = Renderer->View->Camera;
    glm::vec3 viewPos = cam->World2Screen(worldPos, model);

    //glm::vec3 dir = glm::normalize(worldPos - cam->Position);
    //float dot = glm::dot(cam->Front, dir);     //�ж������Ƿ������ǰ��  

    return viewPos.x >= 0 && viewPos.x <= (float)Renderer->View->Width && viewPos.y >= 0 &&
        viewPos.y <= (float)Renderer->View->Height;
}
glm::vec2 CCPanoramaRenderer::GetSphereUVPoint(float u, float v, short i) {
    return glm::vec2(u, v);
}
glm::vec3 CCPanoramaRenderer::GetSpherePoint(float u, float v, float r)
{
    constexpr auto PI = glm::pi<float>();
    float x = r * glm::sin(PI * v) * glm::sin(PI * u * 2);
    float y = r * glm::cos(PI * v);
    float z = r * glm::sin(PI * v) * glm::cos(PI * u * 2);
    return glm::vec3(x, y, z);
}
glm::vec2 CCPanoramaRenderer::GetMercatorUVPoint(float u, float v) const
{
    constexpr auto PI = glm::pi<float>();

    float ��0 = MercatorControlPoint0.x * PI;
    float  �� = u * PI;
    float  �� = v * PI;

    float y = glm::atanh(glm::sin(��));
    return glm::vec2(�� - ��0, y);

    /*
    float ��0 = MercatorControlPoint0.x * PI;
    float  �� = u * PI;
    float  �� = v * PI;
    float  ��p = Mercator_��p;
    float  ��p = Mercator_��p;
    float A = glm::sin(��p) * glm::sin(��) - glm::cos(��p) * glm::cos(��) * glm::sin(�� - ��0);

    float x = glm::atan(
        (glm::tan(��) * glm::cos(��p) + glm::sin(��p) - glm::sin(�� - ��0)) /
        (glm::cos(�� - ��0))
    );
    float y = glm::atan(A);
    return glm::vec2(x, y);
    */
}
void CCPanoramaRenderer::PrecalcMercator() {
    constexpr auto PI = glm::pi<float>();

    float ��0 = MercatorControlPoint0.x * PI;
    float ��1 = MercatorControlPoint1.x * PI;
    float ��2 = MercatorControlPoint2.x * PI;
    float ��1 = MercatorControlPoint1.y * PI;
    float ��2 = MercatorControlPoint2.y * PI;

    Mercator_��p = glm::atan(
        (glm::cos(��1) * glm::sin(��2) * glm::cos(��1) - glm::sin(��1) * glm::cos(��2) * glm::cos(��2)) /
        (glm::sin(��1) * glm::cos(��2) * glm::sin(��2) - glm::cos(��1) * glm::sin(��2) * glm::sin(��1))
    );
    Mercator_��p = glm::atan(
        -((glm::cos(Mercator_��p - ��1)) / glm::tan(��1))
    );
}
