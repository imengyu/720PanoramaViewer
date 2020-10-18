#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "SettingHlp.h"

#define APP_TITLE L"720 Image Viewer"

class COpenGLView;
class CGameRenderer;
class CApp
{
public:
	
	int Run();

	HINSTANCE GetHInstance();
	LoggerInternal* GetLogger();
	SettingHlp* GetSettings();
	LPWSTR GetCurrentDir();
	LPCSTR GetCurrentDirA();
	LPWSTR GetCurrentPath();
	HWND GetConsoleHWND();

	static CApp* Instance;
private:

	LPWSTR* appArgList = nullptr;
	int appArgCount = 0;
	HINSTANCE hInst = nullptr;
	LoggerInternal *logger = nullptr;
	COpenGLView* mainView = nullptr;
	CGameRenderer* gameRenderer = nullptr;
	SettingHlp* settings = nullptr;
	WCHAR currentDir[MAX_PATH];
	CHAR currentDirA[MAX_PATH];
	WCHAR currentPath[MAX_PATH];
	HWND hConsole;
	FILE* fileIn;
	FILE* file;
	FILE* fileErr;

	int GetCommandIsProvideFile(std::wstring* path);

	static LRESULT CALLBACK mainWndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
};

