#pragma once
#include "stdafx.h"
#include <gtc/matrix_transform.hpp>

#include <vector>

// Ϊ��������ƶ������˼��ֵ�ѡ��
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// ��ʼ�����������
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// ������࣬�������벢������Ӧ��ŷ���ǣ�ʸ���;���
class CCamera
{
public:
	// ���������
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// ŷ����
	float Yaw;
	float Pitch;
	// �ɵ�ѡ��
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// ����������
	CCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	// �������Ĺ�����
	CCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	// ����ʹ��ŷ���Ǻ�LookAt��������view����
	glm::mat4 GetViewMatrix();

	// ������κ����Ƽ��̵�����ϵͳ���յ����룬������������ENUM��ʽ��������������Ӵ���ϵͳ�г��������
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	// ������������ϵͳ���յ����룬Ԥ��x��y�����ƫ��ֵ
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	// ������������¼����յ�����
	void ProcessMouseScroll(float yoffset);

	void SetPosItion(glm::vec3 position);

	void SetRotation(glm::vec3 rotation);
private:
	// �Ӹ��µ�CameraEuler��ŷ���Ǽ���ǰ����
	void updateCameraVectors();
};
