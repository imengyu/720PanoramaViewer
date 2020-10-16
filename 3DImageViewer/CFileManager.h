#pragma once
#include "stdafx.h"

class CImageLoader;
class COpenGLRenderer;
class CFileManager
{
public:

	CFileManager(COpenGLRenderer *render);

	void OpenFile();
	void CloseFile();
	void DoOpenFile(wchar_t* path);


private:

	COpenGLRenderer* Render = nullptr;
	CImageLoader* CurrentFileLoader = nullptr;
};

