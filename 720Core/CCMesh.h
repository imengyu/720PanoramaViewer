#pragma once
#include "stdafx.h"
#include <vector>

//����Ϣ
class CCFace
{
public:
	CCFace(unsigned int vertex_index, unsigned int normal_index = 0, unsigned int texcoord_index = -1);

	unsigned int vertex_index;
	unsigned int normal_index;
	unsigned int texcoord_index;
};

//��������
class CCMesh
{
public:
	CCMesh();
	~CCMesh();

	/**
	 * �������񻺳���
	 */
	void GenerateBuffer();
	/**
	 * ���½����ݴ��뻺����
	 */
	void ReBufferData();
	/**
	 * �ͷ����񻺳���
	 */
	void ReleaseBuffer();

	/**
	 * ��ȾMesh
	 */
	void RenderMesh() const;
	/**
	 * ��obj�ļ�����
	 * @param path obj�ļ�·��
	 */
	void LoadFromObj(const wchar_t* path);
	/**
	 * ����Ѽ��ص����ݺͻ�����
	 */
	void UnLoad();

	GLuint MeshVBO = 0;
	GLenum DrawType = GL_STATIC_DRAW;

	//��������
	std::vector<glm::vec3> positions;
	//������������
	std::vector<glm::vec3> normals;
	//��������
	std::vector<CCFace> indices;
	//uv����
	std::vector<glm::vec2> texCoords;
};
