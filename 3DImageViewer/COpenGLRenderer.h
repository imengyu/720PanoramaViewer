#pragma once
#include "stdafx.h"

class COpenGLView;
class COpenGLRenderer
{
protected:
	int Width, Height;

public:
	COpenGLRenderer();
	~COpenGLRenderer();

	virtual bool Init();
	virtual void Render(float FrameTime);
	virtual void RenderUI();
	virtual void Resize(int Width, int Height);
	virtual void Destroy();

	COpenGLView * View;
};

