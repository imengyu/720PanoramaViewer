#pragma once
#include "stdafx.h"
#include "CCamera.h"

//�����ȫ��ģʽ
enum class CCPanoramaCameraMode {
	CenterRoate,
	OutRoataround,
	Static = 3,
};
//������ƶ�ѡ��
enum class CCameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	ROATE_UP,
	ROATE_DOWN,
	ROATE_LEFT,
	ROATE_RIGHT = 7
};

//ȫ�������
class CCPanoramaCamera : public CCamera
{
public:
	//�����ȫ��ģʽ
	CCPanoramaCameraMode Mode = CCPanoramaCameraMode::Static;

	// �������κ����Ƽ��̵�����ϵͳ���յ����룬������������ENUM��ʽ��������������Ӵ���ϵͳ�г��������
	void ProcessKeyboard(CCameraMovement direction, float deltaTime);
	// �������������ϵͳ���յ����룬Ԥ��x��y�����ƫ��ֵ
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch);
	// �������������¼����յ�����
	void ProcessMouseScroll(float yoffset);
	//����ģʽ
	void SetMode(CCPanoramaCameraMode mode);

	float RoateNearMax = 0.2f;
	float RoateFarMax = 3.5f;
	float ZoomSpeed = 0.05f;
	float FovMax = 170.0f;
	float FovMin = 2.0f;
	float RoateYForWorld = 0.0f;
	float RoateXForWorld = 0.0f;
	float MovementSpeed = DEF_SPEED;
	float RoateSpeed = DEF_ROATE_SPEED;
	float MouseSensitivity = DEF_SENSITIVITY;
};
