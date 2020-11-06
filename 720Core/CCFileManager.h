#pragma once
#include "stdafx.h"
#include "CImageLoader.h"
#include <string>

//加载贴图回调
typedef void (*CCFileManagerOnCloseCallback)(void* data);

//文件打开管理
class COpenGLRenderer;
class CCFileManager
{
public:
	CCFileManager(COpenGLRenderer *render);

	bool DoOpenFile(const wchar_t* path);
	void DeleteCurrentFile();
	void OpenCurrentFileAs();
	void OpenFile();

	void CloseFile();
	std::wstring GetCurrentFileName();

	CImageLoader* CurrentFileLoader = nullptr;
	ImageType CurrenImageType = ImageType::Unknow;

	void SetOnCloseCallback(CCFileManagerOnCloseCallback c, void* data) {
		onCloseCallback = c;
		onCloseCallbackData = data;
	}

	bool ImageRatioNotStandard = false;

	const wchar_t* GetLastError();
private:

	Logger* logger = nullptr;
	std::wstring lastErr;
	COpenGLRenderer* Render = nullptr;

	CCFileManagerOnCloseCallback onCloseCallback = nullptr;
	void*onCloseCallbackData = nullptr;
};

