#pragma once
#include "stdafx.h"
#include <gtc/matrix_transform.hpp>
#include <vector>
#include "CColor.h"

// 初始化摄像机变量
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

// 摄像机类，处理输入并计算相应的欧拉角，矢量和矩阵
class CCamera
{
public:
	// 摄像机位置
	glm::vec3 Position = glm::vec3(0.0f);
	// 旋转欧拉角
	glm::vec3 Rotate = glm::vec3(0.0f);
	//摄像机投影
	CCameraProjection Projection = CCameraProjection::Perspective;
	// 摄像机FOV
	float FiledOfView = DEF_FOV;
	//正交投影摄像机视图垂直方向的大小
	float OrthographicSize = 5.0f;
	//剪裁平面近端
	float ClippingNear = 0.1f;
	//剪裁平面远端
	float ClippingFar = 10.0f;
	//摄像机背景颜色
	CColor Background = CColor::Black;

	CCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 rotate = glm::vec3(0.0f, 0.0f, 0.0f));
	// 返回使用欧拉角和LookAt矩阵计算的view矩阵
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
	// 从更新的CameraEuler的欧拉角计算前向量
	void updateCameraVectors();
};
