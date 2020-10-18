#pragma once
#include "stdafx.h"
#include <gtc/matrix_transform.hpp>
#include <vector>
#include "CColor.h"

// ��ʼ�����������
const float DEF_YAW = -90.0f;
const float DEF_PITCH = 0.0f;
const float DEF_SPEED = 2.5f;
const float DEF_ROATE_SPEED = 20.0f;
const float DEF_SENSITIVITY = 0.1f;
const float DEF_FOV = 45.0f;

enum class CCameraProjection {
	Perspective,
	Orthographic
};

// ������࣬�������벢������Ӧ��ŷ���ǣ�ʸ���;���
class CCamera
{
public:
	// �����λ��
	glm::vec3 Position = glm::vec3(0.0f);
	// ��תŷ����
	glm::vec3 Rotate = glm::vec3(0.0f);
	//�����ͶӰ
	CCameraProjection Projection = CCameraProjection::Perspective;
	// �����FOV
	float FiledOfView = DEF_FOV;
	//����ͶӰ�������ͼ��ֱ����Ĵ�С
	float OrthographicSize = 5.0f;
	//����ƽ�����
	float ClippingNear = 0.1f;
	//����ƽ��Զ��
	float ClippingFar = 1000.0f;
	//�����������ɫ
	CColor Background = CColor::Black;

	CCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 rotate = glm::vec3(0.0f, 0.0f, 0.0f));
	// ����ʹ��ŷ���Ǻ�LookAt��������view����
	glm::mat4 GetViewMatrix();

	void SetPosItion(glm::vec3 position);
	void SetRotation(glm::vec3 rotation);

	void ForceUpdate();
	void Reset();

	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

protected:
	// �Ӹ��µ�CameraEuler��ŷ���Ǽ���ǰ����
	void updateCameraVectors();
};