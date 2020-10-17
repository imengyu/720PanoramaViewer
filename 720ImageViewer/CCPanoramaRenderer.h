#pragma once
#include "stdafx.h"
#include "CCTexture.h"
#include "CCModel.h"
#include "CColor.h"
#include <vector>

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

	GLuint shaderProgram = 0;

	CCModel* mainModel = nullptr;

	int sphereSegmentY = 32;
	int sphereSegmentX = 32;
	int sphereFullSegmentY = 24;
	int sphereFullSegmentX = 48;

	bool renderDebugWireframe = false;
	bool renderPanoramaFull = false;
	bool renderDebugVector = false;

	CCTexture*panoramaThumbnailTex = nullptr;
	std::vector<CCTexture*> panoramaTexPool;

	void ReleaseTexPool();

	void ResetModel();
	void RotateModel(float xoffset, float yoffset);

private:
	COpenGLRenderer* Renderer = nullptr;

	void CreateMainModel();
	void CreateMainModelSphereMesh(CCMesh* mesh);

	bool CreateShader();
	void RenderThumbnail(bool wireframe = false);
	void RenderFullChunks();

	//获取球面上的UV坐标
	glm::vec2 GetSphereUVPoint(float u, float v, short i);
	//获取球面上的点
	glm::vec3 GetSpherePoint(float u, float v, float r);
};


