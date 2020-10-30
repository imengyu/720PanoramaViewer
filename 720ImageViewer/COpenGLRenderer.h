#pragma once
#include "stdafx.h"


class COpenGLView;
/**
 * ��Ⱦ��������
 */
class COpenGLRenderer
{
protected:
	//��Ⱥ͸߶�
	int Width, Height;

public:
	COpenGLRenderer();
	~COpenGLRenderer();

	/**
	 * ��ʼ��
	 * @return ���س�ʼ���Ƿ�ɹ�
	 */
	virtual bool Init();
	/**
	 * ��Ⱦʱ����
	 * @param FrameTime ����ʱ��
	 */
	virtual void Render(float FrameTime);
	/**
	 * ��ȾUIʱ����
	 */
	virtual void RenderUI();
	/**
	 * ÿһ֡����ʱ����
	 */
	virtual void Update();
	/**
	 * ����ͼ���µ�����Сʱ����ô˷���
	 * @param Width �¿��
	 * @param Height �¸߶�
	 */
	virtual void Resize(int Width, int Height);
	/**
	 * �ͷ�
	 */
	virtual void Destroy();

	COpenGLView* View;
};

