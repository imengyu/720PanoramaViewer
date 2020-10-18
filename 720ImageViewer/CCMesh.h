#pragma once
#include "stdafx.h"
#include <vector>

//������
class CCMesh
{
public:
	CCMesh();
	~CCMesh();

	void GenerateBuffer();
	void ReleaseBuffer();

	//��ȾMesh
	void RenderMesh();

	//��obj�ļ�����
	void LoadFromObj(const wchar_t* path);
	//����Ѽ��ص����ݺͻ�����
	void UnLoad();

	GLuint MeshVBO = 0;
	GLuint MeshEBO = 0;

	std::vector<glm::vec3> position;
	std::vector<glm::vec3> normals;
	std::vector<GLint> indices;
	std::vector<glm::vec2> texCoords;
};

