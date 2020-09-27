#pragma once

class COpenGLView;
class COpenGLTextRender
{
public:
	COpenGLTextRender(COpenGLView*view);
	~COpenGLTextRender();

	bool Init();
	void DrawString(const char* str);
	void DrawStringPos(const char* str, float x, float y);
	void Destroy();

private:
	COpenGLView *View;
	GLuint lists;
};

