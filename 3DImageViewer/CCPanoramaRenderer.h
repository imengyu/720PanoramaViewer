#pragma once
#include "stdafx.h"
#include "CCTexture.h"

class COpenGLRenderer;
class CCPanoramaRenderer
{
public:

	CCPanoramaRenderer(COpenGLRenderer * renderer);

	void Init();
	void Destroy();
	void Render();

	GLint viewLoc = -1;
	GLint projectionLoc = -1;
	GLint modelLoc = -1;
	GLint ourTextrueLoc = -1;
	GLint ourColorLoc = -1;
	GLint useColorLoc = -1;

	GLubyte* glVendor = nullptr;
	GLubyte* glRenderer = nullptr;
	GLubyte* glVersion = nullptr;
	GLubyte* glslVersion = nullptr;

	GLuint shaderProgram = 0;

	int sphereSegmentY = 16;
	int sphereSegmentX = 32;
	int sphereFullSegmentY = 16;
	int sphereFullSegmentX = 32;

	bool renderDebugWireframe = false;
	bool renderPanoramaFull = false;

	CCTexture*panoramaThumbnailTex = nullptr;


private:
	COpenGLRenderer* Renderer = nullptr;

	bool CreateShader();
	void RenderThumbnail(bool wireframe = false);
	void RenderFullChunks();

	//获取球面上的UV坐标
	glm::vec2 GetSphereUVPoint(float u, float v, short i);
	//获取球面上的点
	glm::vec3 GetSpherePoint(float u, float v, float r);
};


