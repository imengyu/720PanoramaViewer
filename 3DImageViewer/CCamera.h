#pragma once
#include "stdafx.h"
#include <gtc/matrix_transform.hpp>
#include <vector>

// Ϊ��������ƶ������˼��ֵ�ѡ��
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	ROATE_UP,
	ROATE_DOWN,
	ROATE_LEFT,
	ROATE_RIGHT
};
enum Camera_Mode {
	CenterRoate,
	OutRoataround,
	Static,
};


// ��ʼ�����������
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float ROATE_SPEED = 20.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// ������࣬�������벢������Ӧ��ŷ���ǣ�ʸ���;���
class CCamera
{
public:
	// ���������
	glm::vec3 Position = glm::vec3(0.0f);
	// ŷ����
	glm::vec3 Rotate = glm::vec3(0.0f);

	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	Camera_Mode Mode;

	// �ɵ�ѡ��
	float MovementSpeed = SPEED;
	float RoateSpeed = ROATE_SPEED;
	float MouseSensitivity = SENSITIVITY;
	float Zoom = ZOOM;

	CCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 rotate = glm::vec3(0.0f, 0.0f, 0.0f));

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

	void SetMode(Camera_Mode mode);

	void ForceUpdate();
	void Reset();

	float RoateNearMax = 0.2f;
	float RoateFarMax = 3.5f;
	float ZoomSpeed = 0.05f;
	float ZoomMax = 170.0f;
	float ZoomMin = 2.0f;
	float RoateYForWorld = 0.0f;
	float RoateXForWorld = 0.0f;

private:
	// �Ӹ��µ�CameraEuler��ŷ���Ǽ���ǰ����
	void updateCameraVectors();
};
