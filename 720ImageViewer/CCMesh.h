#pragma once
#include "stdafx.h"
#include <vector>

//Õ¯∏Ò¿‡
class CCMesh
{
public:
	CCMesh();
	~CCMesh();

	void GenerateBuffer();
	void ReleaseBuffer();

	void RenderMesh();

	GLuint MeshVBO = 0;
	GLuint MeshEBO = 0;

	std::vector<glm::vec3> vertices;
	std::vector<GLint> indices;
	std::vector<glm::vec2> uv;
};

