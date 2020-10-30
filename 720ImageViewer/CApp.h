#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "SettingHlp.h"

#define APP_TITLE L"720 Image Viewer"

class CWindowsOpenGLView;
class CWindowsGameRenderer;
class CApp
{
public:
	
	int Run();

	HINSTANCE GetHInstance();
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
	CWindowsOpenGLView* mainView = nullptr;
	CWindowsGameRenderer* gameRenderer = nullptr;
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

