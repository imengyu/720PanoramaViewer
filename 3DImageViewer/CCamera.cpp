#include "CCamera.h"

// �������Ĺ�����


// ����������

CCamera::CCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

CCamera::CCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
	Position = glm::vec3(posX, posY, posZ);
	WorldUp = glm::vec3(upX, upY, upZ);
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

// ����ʹ��ŷ���Ǻ�LookAt��������view����

glm::mat4 CCamera::GetViewMatrix()
{
	return glm::lookAt(Position, Position + Front, Up);
}

// ������κ����Ƽ��̵�����ϵͳ���յ����룬������������ENUM��ʽ��������������Ӵ���ϵͳ�г��������

void CCamera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = MovementSpeed * deltaTime;
	if (direction == FORWARD)
		Position += Front * velocity;
	if (direction == BACKWARD)
		Position -= Front * velocity;
	if (direction == LEFT)
		Position -= Right * velocity;
	if (direction == RIGHT)
		Position += Right * velocity;
}

// ������������ϵͳ���յ����룬Ԥ��x��y�����ƫ��ֵ

void CCamera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	// ȷ����pitch������Χʱ����Ļ���ᷭת
	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	// ʹ�ø��µ�ŷ���Ǹ���3������
	updateCameraVectors();
}

// ������������¼����յ�����

void CCamera::ProcessMouseScroll(float yoffset)
{
	if (Zoom >= 1.0f && Zoom <= 45.0f)
		Zoom -= yoffset;
	if (Zoom <= 1.0f)
		Zoom = 1.0f;
	if (Zoom >= 45.0f)
		Zoom = 45.0f;
}

void CCamera::SetPosItion(glm::vec3 position)
{
	Position = position;
	updateCameraVectors();
}

void CCamera::SetRotation(glm::vec3 rotation)
{
	Pitch = rotation.x;
	Yaw = rotation.y;
	updateCameraVectors();
}

// �Ӹ��µ�CameraEuler��ŷ���Ǽ���ǰ����

void CCamera::updateCameraVectors()
{
	// �����µ�ǰ����
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	// �ټ�����������������
	Right = glm::normalize(glm::cross(Front, WorldUp));  // ��׼��
	Up = glm::normalize(glm::cross(Right, Front));
}
