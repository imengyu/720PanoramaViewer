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
	bool loadMarked = false;
	bool loadStarted = false;
	bool loadFinished = false;
};

class CCRenderGlobal;
class COpenGLRenderer;
class CCPanoramaRenderer
{
public:

	CCPanoramaRenderer(COpenGLRenderer * renderer);

	void Init();
	void Destroy();
	void Render();

	CColor wireframeColor = CColor::FromString("#DC143C");

	Logger* logger = nullptr;

	CCRenderGlobal* globalRenderInfo = nullptr;

	CCShader* shader = nullptr;

	CCModel* mainModel = nullptr;
	std::vector<ChunkModel*> fullModels;

	int sphereSegmentY = 64;
	int sphereSegmentX = 128;
	//int sphereSegmentY = 50;
	//int sphereSegmentX = 40;
	int sphereFullSegmentY = 24;
	int sphereFullSegmentX = 48;

	bool renderDebugWireframe = false;
	bool renderPanoramaFull = false;
	bool renderDebugVector = false;

	CCTexture* panoramaCheckTex = nullptr;
	CCTexture*panoramaThumbnailTex = nullptr;
	std::vector<CCTexture*> panoramaTexPool;

	void ReleaseTexPool();
	void ReleaseFullModel();
	void GenerateFullModel(int chunkW, int chunkH);

	void ResetModel();
	void RotateModel(float xoffset, float yoffset);

	void UpdateMainModelTex();


private:
	COpenGLRenderer* Renderer = nullptr;

	void CreateMainModel();
	void CreateFullModelSphereMesh(CCMesh* mesh, int chunkW, int chunkH, int currentChuntX, int currentChuntY);
	void CreateMainModelSphereMesh(CCMesh* mesh);

	void LoadBuiltInResources();
	void RenderThumbnail(bool wireframe = false);
	void RenderFullChunks();

	//获取球面上的UV坐标
	glm::vec2 GetSphereUVPoint(float u, float v, short i);
	//获取球面上的点
	glm::vec3 GetSpherePoint(float u, float v, float r);
};


