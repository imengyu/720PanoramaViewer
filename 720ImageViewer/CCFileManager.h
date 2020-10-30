#pragma once
#include "stdafx.h"
#include "CImageLoader.h"
#include <string>

//������ͼ�ص�
typedef void (*CCFileManagerOnCloseCallback)(void* data);

//�ļ��򿪹���
class COpenGLRenderer;
class CCFileManager
{
public:

	CCFileManager(COpenGLRenderer *render);

#if defined(VR720_WINDOWS)
	bool DoOpenFile(const vchar* path);
	void DeleteCurrentFile();
	void OpenCurrentFileAs();
	void OpenFile();
#else
	bool OpenFile(const vchar* path);
#endif
	void CloseFile();
	vstring GetCurrentFileName();

	CImageLoader* CurrentFileLoader = nullptr;
	ImageType CurrenImageType = ImageType::Unknow;

	void SetOnCloseCallback(CCFileManagerOnCloseCallback c, void* data) {
		onCloseCallback = c;
		onCloseCallbackData = data;
	}

	bool ImageRatioNotStandard = false;

	const vchar* GetLastError();
private:

	Logger* logger = nullptr;
	vstring lastErr;
	COpenGLRenderer* Render = nullptr;

	CCFileManagerOnCloseCallback onCloseCallback = nullptr;
	void*onCloseCallbackData = nullptr;
};

