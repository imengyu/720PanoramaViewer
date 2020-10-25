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

typedef void(*CCPanoramaCameraFovChangedCallback)(void* data, float fov);

class COpenGLView;
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
	float OrthographicSize = 1.0f;
	//����ƽ�����
	float ClippingNear = 0.1f;
	//����ƽ��Զ��
	float ClippingFar = 1000.0f;
	//�����������ɫ
	CColor Background = CColor::Black;

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	CCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 rotate = glm::vec3(0.0f, 0.0f, 0.0f));
	// ����ʹ��ŷ���Ǻ�LookAt��������view����
	glm::mat4 GetViewMatrix();

	void SetOrthoSize(float o);

	void SetFOVChangedCallback(CCPanoramaCameraFovChangedCallback callback, void* data);
	void SetOrthoSizeChangedCallback(CCPanoramaCameraFovChangedCallback callback, void* data);

	void SetPosItion(glm::vec3 position);
	void SetRotation(glm::vec3 rotation);
	void SetFOV(float fov);

	void ForceUpdate();
	void Reset();

	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	void SetView(COpenGLView* view);

	/**
	 * @brief  					��������ת��Ϊ��������
	 * @brief screenPoint		���������
	 * @brief viewportRange 	�ӿڷ�Χ�� ����ֵ����Ϊ������-����
	 * @brief modelViewMatrix 	ģ����ͼ����
	 * @brief projectMatrix 	ͶӰ����
	 * @brief pPointDepth   	��Ļ�����ȣ������ָ��(Ϊnullptr),����Ȼ������ж�ȡ���ֵ
	 * @return 					��������ϵ
	 * @note ע�⣺�õ�����������ϵ��ʹ��ǰҪ�����������ֵw��
	 *		 ���w��0����Ӧʹ�ô˵㡣
	 * @code
	 *  // sample
	 *  ...
	 *  auto&& worldPoint = Screen2World(...);
	 *  if( !FuzzyIsZero( worldPoint.w ) )
	 *  {
	 *	 	glm::vec3 world3D(worldPoint);
	 *      world3D /= worldPoint;
	 *      /// using world3D
	 *	}
	 *	else
	 *	{
	 *		// error handler
	 *	}
	 */
	glm::vec3 Screen2World(const glm::vec2& screenPoint, glm::mat4& model, float* pPointDepth);
	/**
	 * @brief ��������ϵת��Ϊ��Ļ����ϵ
	 * @brief worldPoint		��������ĵ������
	 * @brief viewportRange 	�ӿڷ�Χ�� ����ֵ����Ϊ������-����
	 * @brief modelViewMatrix 	ģ����ͼ����
	 * @brief projectMatrix 	ͶӰ����
	 * @brief pPointDepth   	��Ļ�����ȣ������ָ��(Ϊnullptr),����Ȼ������ж�ȡ���ֵ
	 * @return 					���������
	 * @note ���صĴ�����������ֵ�����������2D������������㣬����������x,yά����
	 */
	glm::vec3 World2Screen(const glm::vec3& worldPoint, glm::mat4& model);

protected:
	COpenGLView* glView = nullptr;

	// �Ӹ��µ�CameraEuler��ŷ���Ǽ���ǰ����
	void updateCameraVectors();

	CCPanoramaCameraFovChangedCallback fovChangedCallback = nullptr;
	void* fovChangedCallbackData = nullptr;
	CCPanoramaCameraFovChangedCallback orthoSizeChangedCallback = nullptr;
	void* orthoSizeChangedCallbackData = nullptr;
};
