#pragma once
#include "stdafx.h"
#include "COpenGLRenderer.h"

class CGameRenderer : public COpenGLRenderer
{
public:
	CGameRenderer();
	~CGameRenderer();

	void SetOpenFilePath(std::string path);
private:

	std::string currentOpenFilePath;

	bool Init() override;
	void Render(float FrameTime) override;
	void RenderUI() override;
	void Resize(int Width, int Height) override;
	void Destroy() override;

	GLfloat* vertices = nullptr;
	GLuint vertex_count = 0;
	GLuint vertex_buffer_object = 0;
	GLuint vertex_array_object = 0;
	GLuint element_buffer_object = 0;;//EBO
	GLuint shaderProgram = 0;
	GLint viewLoc = 0;
	GLint projectionLoc = 0;
	GLint modelLoc = 0;

	void createShader();

	glm::vec3 getPoint(GLfloat u, GLfloat v);
	void createSphere(GLfloat* sphere, GLuint Longitude, GLuint Latitude);
};

