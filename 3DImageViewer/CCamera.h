#pragma once
#include "stdafx.h"
#include <gtc/matrix_transform.hpp>

#include <vector>

// 为摄像机的移动定义了几种的选项
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// 初始化摄像机变量
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// 摄像机类，处理输入并计算相应的欧拉角，矢量和矩阵
class CCamera
{
public:
	// 摄像机变量
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// 欧拉角
	float Yaw;
	float Pitch;
	// 可调选项
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// 向量构造器
	CCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	// 含标量的构造器
	CCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

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
private:
	// 从更新的CameraEuler的欧拉角计算前向量
	void updateCameraVectors();
};
