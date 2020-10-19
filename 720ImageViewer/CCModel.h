#pragma once
#include "stdafx.h"

class CCMaterial;
class CCMesh;
//ģ����
class CCModel
{
public:
	CCModel();
	~CCModel();

	//ˢ��ģ������
	void UpdateVectors();
	//��ȡģ�ͱ任����
	glm::mat4 GetMatrix();
	//����ģ��λ�ú���ת
	void Reset();
	//����ģ��
	void Render();

	bool Visible = true;

	//ģ��λ��
	glm::vec3 Positon = glm::vec3(0.0f);
	//ģ����ת
	glm::vec3 Rotation = glm::vec3(0.0f);

	//ģ�Ͳ���
	CCMaterial* Material = nullptr;
	//ģ������
	CCMesh* Mesh = nullptr;

	glm::vec3 Front = glm::vec3(0.0f);
	glm::vec3 Right = glm::vec3(0.0f);
	glm::vec3 Up = glm::vec3(0.0f);
	glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
};

