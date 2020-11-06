#pragma once
#include "stdafx.h"

class COpenGLView;
/**
 * 渲染器抽象类
 */
class VR720_EXP COpenGLRenderer
{
protected:
	//宽度和高度
	int Width = 0, Height = 0;

public:
	COpenGLRenderer()
	{
	}
	virtual ~COpenGLRenderer()
	{
	}

	/**
	 * 初始化
	 * @return 返回初始化是否成功
	 */
	virtual bool Init()
	{
		return true;
	}
	/**
	 * 渲染时调用
	 * @param FrameTime 增量时间
	 */
	virtual void Render(float FrameTime)
	{
	}
	/**
	 * 渲染UI时调用
	 */
	virtual void RenderUI()
	{
	}
	/**
	 * 每一帧更新时调用
	 */
	virtual void Update()
	{

	}
	/**
	 * 当视图重新调整大小时会调用此方法
	 * @param Width 新宽度
	 * @param Height 新高度
	 */
	virtual void Resize(int Width, int Height)
	{
		this->Width = Width;
		this->Height = Height;
	}
	/**
	 * 释放
	 */
	virtual void Destroy()
	{
	}

	COpenGLView* View = nullptr;
};

