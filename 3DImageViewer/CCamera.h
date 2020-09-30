#pragma once
#include "stdafx.h"
#include <gtc/matrix_transform.hpp>
#include <vector>

// 为摄像机的移动定义了几种的选项
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


// 初始化摄像机变量
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float ROATE_SPEED = 20.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// 摄像机类，处理输入并计算相应的欧拉角，矢量和矩阵
class CCamera
{
public:
	// 摄像机变量
	glm::vec3 Position = glm::vec3(0.0f);
	// 欧拉角
	glm::vec3 Rotate = glm::vec3(0.0f);

	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	Camera_Mode Mode;

	// 可调选项
	float MovementSpeed = SPEED;
	float RoateSpeed = ROATE_SPEED;
	float MouseSensitivity = SENSITIVITY;
	float Zoom = ZOOM;

	CCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 rotate = glm::vec3(0.0f, 0.0f, 0.0f));

	// 返回使用欧拉角和LookAt矩阵计算的view矩阵
	glm::mat4 GetViewMatrix();

	// 处理从任何类似键盘的输入系统接收的输入，以摄像机定义的ENUM形式接受输入参数（从窗口系统中抽象出来）
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	// 处理从鼠标输入系统接收的输入，预测x和y方向的偏移值
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	// 处理从鼠标滚轮事件接收的输入
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
	// 从更新的CameraEuler的欧拉角计算前向量
	void updateCameraVectors();
};
