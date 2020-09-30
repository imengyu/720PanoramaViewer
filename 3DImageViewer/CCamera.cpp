#include "CCamera.h"

CCamera::CCamera(glm::vec3 position, glm::vec3 up, glm::vec3 rotate)
{
	Position = position;
	WorldUp = up;
	Rotate = rotate;
	updateCameraVectors();
}

// 返回使用欧拉角和LookAt矩阵计算的view矩阵
glm::mat4 CCamera::GetViewMatrix()
{
	return glm::lookAt(Position, Position + Front, Up);
}

// 处理从任何类似键盘的输入系统接收的输入，以摄像机定义的ENUM形式接受输入参数（从窗口系统中抽象出来）
void CCamera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = MovementSpeed * deltaTime;

	switch (direction)
	{
	case FORWARD:
		Position += Front * velocity;

		break;
	case BACKWARD:
		Position -= Front * velocity;

		break;
	case LEFT:
		Position -= Right * velocity;

		break;
	case RIGHT:
		Position += Right * velocity;

		break;
	case ROATE_UP:
		Rotate.y += RoateSpeed * deltaTime;
		break;
	case ROATE_DOWN:
		Rotate.y -= RoateSpeed * deltaTime;
		break;
	case ROATE_LEFT:
		Rotate.x -= RoateSpeed * 1.3f * deltaTime;
		break;
	case ROATE_RIGHT:
		Rotate.x += RoateSpeed * 1.3f * deltaTime;
		break;
	default:
		break;
	}

	updateCameraVectors();
}

// 处理从鼠标输入系统接收的输入，预测x和y方向的偏移值
void CCamera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
	switch (Mode)
	{
	case CenterRoate: {
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Rotate.x += xoffset;
		Rotate.y += yoffset;

		// 确保当pitch超出范围时，屏幕不会翻转
		if (constrainPitch)
		{
			if (Rotate.y > 89.0f)
				Rotate.y = 89.0f;
			if (Rotate.y < -89.0f)
				Rotate.y = -89.0f;
		}

		// 使用更新的欧拉角更新3个向量
		updateCameraVectors();
		break;
	}
	case OutRoataround: {
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		RoateXForWorld += xoffset;
		RoateYForWorld += yoffset;

		//计算摄像机在这个球上的坐标
		float distance = glm::distance(Position, glm::vec3(0.0f));

		float w = distance * glm::cos(glm::radians(RoateYForWorld));
		Position.x = w * glm::cos(glm::radians(RoateXForWorld));
		Position.y = distance * glm::sin(glm::radians(RoateYForWorld));
		Position.z = w * glm::sin(glm::radians(RoateXForWorld));

		if (Position.y < 0) Rotate.x = 180.0f - Rotate.x;
		if (Position.z < 0) Rotate.y = 180.0f - Rotate.y;

		updateCameraVectors();
		break;
	}
	case Static:
		break;
	}
}

// 处理从鼠标滚轮事件接收的输入
void CCamera::ProcessMouseScroll(float yoffset)
{
	switch (Mode)
	{
	case CenterRoate: {
		if (Zoom >= ZoomMin && Zoom <= ZoomMax)
			Zoom -= yoffset * ZoomSpeed;
		if (Zoom <= ZoomMin) Zoom = ZoomMin;
		if (Zoom >= ZoomMax) Zoom = ZoomMax;
		break;
	}
	case OutRoataround: {
		Position.z -= yoffset * ZoomSpeed * 0.1f;
		if (Position.z < RoateNearMax) Position.z = RoateNearMax;
		if (Position.z > RoateFarMax) Position.z = RoateFarMax;
		break;
	}
	case Static:
		break;
	}
}

void CCamera::SetPosItion(glm::vec3 position)
{
	Position = position;
	updateCameraVectors();
}

void CCamera::SetRotation(glm::vec3 rotation)
{
	Rotate = rotation;
	updateCameraVectors();
}

void CCamera::SetMode(Camera_Mode mode)
{
	Mode = mode; 
	switch (Mode)
	{
	case CenterRoate:
		Reset();
		break;
	case OutRoataround:
		Reset();
		Position = glm::vec3(0.0f, 0.0f, 3.0f);
		Rotate = glm::vec3(-90.0f, 0.0f, 0.0f);
		RoateYForWorld = 0.0f;
		RoateXForWorld = 0.0f;
		updateCameraVectors();
		break;
	case Static:
		Reset();
		break;
	default:
		break;
	}
}

void CCamera::ForceUpdate() {
	updateCameraVectors();
}

void CCamera::Reset()
{
	Position = glm::vec3(0.0f);
	Up = glm::vec3(0.0f, 1.0f, 0.0f);
	Front = glm::vec3(0.0f, 0.0f, -1.0f);
	Rotate = glm::vec3(YAW, PITCH, 0.0f);
	Zoom = ZOOM;
	updateCameraVectors();
}

// 从更新的CameraEuler的欧拉角计算前向量

void CCamera::updateCameraVectors()
{
	// 计算新的前向量
	glm::vec3 front;
	front.x = cos(glm::radians(Rotate.x)) * cos(glm::radians(Rotate.y));
	front.y = sin(glm::radians(Rotate.y));
	front.z = sin(glm::radians(Rotate.x)) * cos(glm::radians(Rotate.y));
	Front = glm::normalize(front);
	// 再计算右向量和上向量
	Right = glm::normalize(glm::cross(Front, WorldUp));  // 标准化
	Up = glm::normalize(glm::cross(Right, Front));
}
