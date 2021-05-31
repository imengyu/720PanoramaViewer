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
	int Width = 0, Height = 0;

public:
	COpenGLRenderer()
	{
	}
	virtual ~COpenGLRenderer()
	{
	}

	/**
	 * ��ʼ��
	 * @return ���س�ʼ���Ƿ�ɹ�
	 */
	virtual bool Init()
	{
		return true;
	}
	/**
	 * ��Ⱦʱ����
	 * @param FrameTime ����ʱ��
	 */
	virtual void Render(float FrameTime)
	{
	}
	/**
	 * ��ȾUIʱ����
	 */
	virtual void RenderUI()
	{
	}
	/**
	 * ÿһ֡����ʱ����
	 */
	virtual void Update()
	{

	}
	virtual bool SetProperty(const char* name, const wchar_t* val)
	{
		return false;
	}
	virtual bool GetPropertyBool(const char* name)
	{
		return false;
	}
	virtual bool GetPropertyInt(const char* name)
	{
		return false;
	}
	virtual bool GetPropertyString(const char* name)
	{
		return false;
	}

	/**
	 * ����ͼ���µ�����Сʱ����ô˷���
	 * @param Width �¿��
	 * @param Height �¸߶�
	 */
	virtual void Resize(int Width, int Height)
	{
		this->Width = Width;
		this->Height = Height;
	}
	/**
	 * �ͷ�
	 */
	virtual void Destroy()
	{
	}

	COpenGLView* View = nullptr;
};

