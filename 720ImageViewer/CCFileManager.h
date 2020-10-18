#pragma once
#include "stdafx.h"
#include "CImageLoader.h"

//������ͼ�ص�
typedef void (*CCFileManagerOnCloseCallback)(void* data);

class COpenGLRenderer;
class CCFileManager
{
public:

	CCFileManager(COpenGLRenderer *render);

	void OpenFile();
	void CloseFile();
	bool DoOpenFile(const wchar_t* path);

	static const std::wstring GetResourcePath(const wchar_t* typeName, const wchar_t* name);
	static const std::wstring GetDirResourcePath(const wchar_t* dirName, const wchar_t* name);
	static const std::string GetResourcePath(const char* typeName, const char* name);
	static const std::string GetDirResourcePath(const char* dirName, const char* name);

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

