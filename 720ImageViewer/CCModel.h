#pragma once
#ifndef VR720_CCMODEL_H
#define VR720_CCMODEL_H
#include "stdafx.h"
#include "CCSmartPtr.hpp"

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
	glm::mat4 GetMatrix() const;

	/**
	 * ����ģ��λ�ú���ת
	 */
	void Reset();
	/**
	 * ����ģ��
	 */
	void Render() const;

	//ģ���Ƿ�ɼ�
	bool Visible = true;

	//ģ��λ��
	glm::vec3 Position = glm::vec3(0.0f);
	//ģ����ת
	glm::vec3 Rotation = glm::vec3(0.0f);

	//ģ�Ͳ���
	CCSmartPtr<CCMaterial> Material;
	//ģ������
	CCSmartPtr<CCMesh> Mesh;

	glm::vec3 Front = glm::vec3(0.0f);
	glm::vec3 Right = glm::vec3(0.0f);
	glm::vec3 Up = glm::vec3(0.0f);
	glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
};

#endif

