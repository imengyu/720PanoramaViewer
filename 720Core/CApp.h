#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "SettingHlp.h"

#define APP_TITLE L"720 Panorama Viewer"

class CWindowsOpenGLView;
class CWindowsGameRenderer;

typedef int(*CAppRunCallback)(void* ptr, CWindowsOpenGLView* view, CWindowsGameRenderer* renderer);

class VR720_EXP CApp {

public:
	static CApp* Instance;

	virtual bool Init() { return false; }
	virtual int Run(int*p) { return 0; }
	virtual bool Destroy() { return false; }

	virtual HINSTANCE GetHInstance() { return nullptr; }
	virtual SettingHlp* GetSettings() { return nullptr; }
	virtual LPWSTR GetCurrentDir() { return nullptr; }
	virtual LPCSTR GetCurrentDirA() { return nullptr; }
	virtual LPWSTR GetCurrentPath() { return nullptr; }
	virtual HWND GetConsoleHWND() { return nullptr; }
	virtual Logger* GetLogger() { return nullptr; }

	virtual void SetCurrentHWND(HWND hWnd) {}
	void SetAppRunCallback(void* ptr, CAppRunCallback callback);

protected:

	void* runCallbackData = nullptr;
	CAppRunCallback runCallback = nullptr;
};

#ifdef VR720_EXPORTS 

class CAppInternal : public CApp
{
public:
	
	bool Init();
	int Run(int* p);
	bool Destroy();

	HINSTANCE GetHInstance()
	{
		return hInst;
	}
	SettingHlp* GetSettings()
	{
		return settings;
	}
	LPWSTR GetCurrentDir()
	{
		return currentDir;
	}
	LPCSTR GetCurrentDirA()
	{
		return currentDirA;
	}
	LPWSTR GetCurrentPath()
	{
		return currentPath;
	}
	HWND GetConsoleHWND()
	{
		return hConsole;
	}
	Logger* GetLogger() { return logger; }

	void SetCurrentHWND(HWND hWnd);
private:

	LPWSTR* appArgList = nullptr;
	int appArgCount = 0;
	HINSTANCE hInst = nullptr;
	CWindowsOpenGLView* mainView = nullptr;
	CWindowsGameRenderer* gameRenderer = nullptr;
	SettingHlp* settings = nullptr;
	WCHAR currentDir[MAX_PATH];
	CHAR currentDirA[MAX_PATH];
	WCHAR currentPath[MAX_PATH];
	HWND hConsole = nullptr;
	HWND hMain = nullptr;
	FILE* fileIn = nullptr;
	FILE* file = nullptr;
	FILE* fileErr = nullptr;
	Logger* logger = nullptr;


	bool CheckMutex();
	void InitConsole();

	int GetCommandIsProvideFile(std::wstring* path);
};

#endif

