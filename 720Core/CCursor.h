#pragma once
#include "stdafx.h"
#include "COpenGLView.h"
#include "resource.h"

class VR720_EXP CCursor
{
public:
	static CCursor* Default;
	static CCursor* Arrow;
	static CCursor* Grab;
	static CCursor* Hand;

	static void Init();
	static void Destroy();

	static void SetViewCursur(COpenGLView* view, CCursor* cursor);
	static void SetHWNDCursur(HWND view, CCursor* cursor);

	CCursor(LPCWSTR type);
	CCursor(HINSTANCE hInst, LPCWSTR type);
	CCursor(char*texturePath);
	~CCursor();

private:

	HCURSOR hCursor = NULL;

};

