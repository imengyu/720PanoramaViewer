#pragma once
#include "stdafx.h"
#include <vector>

//网格类
class CCMesh
{
public:
	CCMesh();
	~CCMesh();

	void GenerateBuffer();
	void ReleaseBuffer();

	//渲染Mesh
	void RenderMesh();

	//从obj文件加载
	void LoadFromObj(const wchar_t* path);
	//清空已加载的数据和缓冲区
	void UnLoad();

	GLuint MeshVBO = 0;
	GLuint MeshEBO = 0;

	std::vector<glm::vec3> position;
	std::vector<glm::vec3> normals;
	std::vector<GLint> indices;
	std::vector<glm::vec2> texCoords;
};

