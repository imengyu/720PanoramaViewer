#pragma once
#include "stdafx.h"
#include "CImageLoader.h"

//¼ÓÔØÌùÍ¼»Øµ÷
typedef void (*CCFileManagerOnCloseCallback)(void* data);

class COpenGLRenderer;
class CCFileManager
{
public:

	CCFileManager(COpenGLRenderer *render);

	void OpenFile();
	void CloseFile();
	bool DoOpenFile(const wchar_t* path);

	CImageLoader* CurrentFileLoader = nullptr;
	ImageType CurrenImageType = ImageType::Unknow;

	void SetOnCloseCallback(CCFileManagerOnCloseCallback c, void* data) {
		onCloseCallback = c;
		onCloseCallbackData = data;
	}

	const wchar_t* GetLastError();
private:

	Logger* logger = nullptr;
	std::wstring lastErr;
	COpenGLRenderer* Render = nullptr;

	CCFileManagerOnCloseCallback onCloseCallback = nullptr;
	void*onCloseCallbackData = nullptr;
};

