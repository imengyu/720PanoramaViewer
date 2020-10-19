#pragma once
#include "stdafx.h"
#include "CCTexture.h"
#include "CCModel.h"
#include "CCShader.h"
#include "CColor.h"
#include <vector>

struct ChunkModel {
	CCModel* model;
	int chunkX;
	int chunkY;
	float chunkXv;
	float chunkYv;
	float chunkXvE;
	float chunkYvE;
	glm::vec3 pointCenter;
	glm::vec3 pointA;
	glm::vec3 pointB;
	glm::vec3 pointC;
	glm::vec3 pointD;
	bool loadMarked = false;
};

class CCRenderGlobal;
class CGameRenderer;
class CCPanoramaRenderer
{
public:

	CCPanoramaRenderer(CGameRenderer* renderer);

	void Init();
	void Destroy();
	void Render(float deltaTime);

	CColor wireframeColor = CColor::FromString("#DC143C");
	CColor wireframeColor2 = CColor::FromString("#0000FF");

	Logger* logger = nullptr;

	CCRenderGlobal* globalRenderInfo = nullptr;

	CCShader* shader = nullptr;

	CCModel* testModel = nullptr;
	CCModel* mainModel = nullptr;
	std::vector<ChunkModel*> fullModels;

	glm::mat4 model = glm::mat4(1.0f);

	int sphereSegmentY = 64;
	int sphereSegmentX = 128;
	//int sphereSegmentY = 30;
	//int sphereSegmentX = 15;
	int sphereFullSegmentY = 0;
	int sphereFullSegmentX = 0;

	float panoramaFullViewWidth = 0.2f;
	float panoramaFullViewHeight = 0.1f;
	int panoramaFullSplitW = 0;
	int panoramaFullSplitH = 0;

	int renderPanoramaFullTestIndex = 0;
	float renderPanoramaFullTestTime = 0;

	bool renderDebugWireframe = false;
	bool renderNoPanoramaSmall = false;
	bool renderPanoramaFull = false;
	bool renderPanoramaFullTest = false;
	bool renderPanoramaFullRollTest = false;
	bool renderPanoramaATest = false;
	bool renderDebugVector = false;

	CCTexture* panoramaRedCheckTex = nullptr;
	CCTexture* panoramaCheckTex = nullptr;
	CCTexture*panoramaThumbnailTex = nullptr;
	std::vector<CCTexture*> panoramaTexPool;

	void ReleaseTexPool();
	void ReleaseFullModel();
	void GenerateFullModel(int chunkW, int chunkH);

	void ResetModel();
	void RotateModel(float xoffset, float yoffset);

	void UpdateMainModelTex();
	void UpdateFullChunksVisible();

private:
	CGameRenderer* Renderer = nullptr;

	void CreateMainModel();
	glm::vec3  CreateFullModelSphereMesh(ChunkModel* info, int segXStart, int segYStart, int segXEnd, int segYEnd);
	void CreateMainModelSphereMesh(CCMesh* mesh);

	void LoadBuiltInResources();
	void RenderThumbnail(bool wireframe = false);
	void RenderFullChunks(float deltaTime);

	bool IsInView(glm::vec3 worldPos);

	//获取球面上的UV坐标
	glm::vec2 GetSphereUVPoint(float u, float v, short i);
	//获取球面上的点
	glm::vec3 GetSpherePoint(float u, float v, float r);
};


