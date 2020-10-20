#include "CApp.h"
#include "CCursor.h"
#include "CCMeshLoader.h"
#include "COpenGLView.h"
#include "CGameRenderer.h"
#include "CAboutDialog.h"
#include "CHelpDialog.h"
#include "CRegDialog.h"
#include "SystemHelper.h"
#include "StringSplit.h"
#include "StringHlp.h"
#include <shellapi.h>
#include <Shlwapi.h>

CApp* CApp::Instance = nullptr;

LRESULT CALLBACK  CApp::mainWndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
	case WM_CLOSE: {
		Instance->mainView->CloseView();
		break;
	}
	case WM_KILLFOCUS: {
		Instance->mainView->SetToLowerFpsMode();
		break;
	}
	case WM_SETFOCUS: {
		Instance->mainView->QuitLowerFpsMode();
		break;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	case WM_CUSTOM_SHOW_ABOUTBOX: {
		CAboutDialog::Show(hWnd);
		Instance->gameRenderer->NotifyAboutDialogClosed();
		break;
	}
	case WM_CUSTOM_SHOW_HELPBOX: {
		CHelpDialog::Show(hWnd);
		Instance->gameRenderer->NotifyHelpDialogClosed();
		break;
	}
	case WM_CUSTOM_SHOW_REG: {
		CRegDialog::Show(hWnd);
		break;
	}
	case WM_CUSTOM_OPEN_FILE: {
		WCHAR strFilename[MAX_PATH] = { 0 };
		if (SystemHelper::ChooseOneFile(hWnd, NULL, L"请选择一个全景图", L"图片文件\0*.jpg;*.png;*.bmp\0所有文件(*.*)\0*.*\0\0\0",
			strFilename, NULL, strFilename, MAX_PATH)) {
			Instance->gameRenderer->SetOpenFilePath(strFilename);
			Instance->gameRenderer->DoOpenFile();
		}
		break;
	}
	case WM_DROPFILES: {
		HDROP hDropInfo = (HDROP)wParam;
		UINT  nFileCount = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
		TCHAR szFileName[_MAX_PATH] = _T("");
		DWORD dwAttribute;

		if (nFileCount > 0) {
			::DragQueryFile(hDropInfo, 0, szFileName, sizeof(szFileName));
			dwAttribute = ::GetFileAttributes(szFileName);

			if ((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
			{
				Instance->gameRenderer->SetOpenFilePath(szFileName);
				Instance->gameRenderer->DoOpenFile();
			}
		}

		::DragFinish(hDropInfo);
		break;
	}
	case WM_CUSTOM_CMD_OPENFILE: {
		std::wstring path = Instance->settings->GetSettingStr(L"NextOpenFilePath", L"", 512);
		if (_waccess_s(path.c_str(), 0) == 0) {
			Instance->gameRenderer->SetOpenFilePath(path.c_str());
			Instance->gameRenderer->DoOpenFile();
		}
		break;
	}
	default:
		break;
	}
	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

int CApp::Run()
{
	int result = 0;

	Instance = this;
	hInst = GetModuleHandle(NULL);
	GetModuleFileName(NULL, currentPath, MAX_PATH);
	wcscpy_s(currentDir, currentPath);
	PathRemoveFileSpec(currentDir);
	char* currentDirAb = StringHlp::UnicodeToAnsi(currentDir);
	strcpy_s(currentDirA, currentDirAb);
	StringHlp::FreeStringPtr(currentDirAb);

	appArgList = CommandLineToArgvW(GetCommandLine(), &appArgCount);
	if (appArgList == NULL)
	{
		MessageBox(NULL, L"Unable to parse command line", L"Error", MB_OK);
		return -1;
	}

	logger = new LoggerInternal();
	logger->SetLogLevel(LogLevel::LogLevelText);
	logger->SetLogOutPut(LogOutPut::LogOutPutConsolne);

	settings = new SettingHlpInternal(FormatString(L"%s\\config\\config.ini", currentDir).c_str());

	CCursor::Init(hInst);
	CCMeshLoader::Init();

	//命令行读取
	std::wstring filePath;
	bool hasInputFile = false;
	hasInputFile = GetCommandIsProvideFile(&filePath);

	//单例判断
	bool forceNotSingle = settings->GetSettingBool(L"NoSingleCheck", FALSE);
	HANDLE  hMutex = CreateMutex(NULL, FALSE, L"720Mutex");
	if (hMutex && (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		CloseHandle(hMutex);
		hMutex = NULL;
		
		HWND windowHandle = (HWND)settings->GetSettingInt(L"LastHWND", 0);
		if (windowHandle != nullptr) {

			//显示窗口
			if (!IsWindowVisible(windowHandle)) ShowWindow(windowHandle, SW_SHOW);
			if (IsIconic(windowHandle)) ShowWindow(windowHandle, SW_RESTORE);
			SetForegroundWindow(windowHandle);

			//传递文件路径至打开的进程中
			if (hasInputFile) {
				settings->SetSettingStr(L"NextOpenFilePath", filePath.c_str());
				SendMessage(windowHandle, WM_CUSTOM_CMD_OPENFILE, NULL, NULL);
			}
		}

		goto QUIT_AND_DESTROY;
	}

	AllocConsole();
	hConsole = GetConsoleWindow();
	ShowWindow(hConsole, SW_HIDE);
	SetConsoleTitle(APP_TITLE);

	freopen_s(&fileIn, "CONIN$", "r", stdin);
	freopen_s(&fileErr, "CONOUT$", "w", stderr);
	freopen_s(&file, "CONOUT$", "w", stdout);

	logger->InitLogConsoleStdHandle();

	gameRenderer = new CGameRenderer();
	mainView = new COpenGLView(gameRenderer);

	if (mainView->Init(hInst, APP_TITLE, 1024, 768, (WNDPROC)mainWndProc))
	{
		if (hasInputFile) {
			logger->Log(L"Command input file %hs", filePath.c_str());
			gameRenderer->SetOpenFilePath(filePath.c_str());
			gameRenderer->DoOpenFile();
		}

		logger->Log(L"Init ok");

		mainView->Show();

		HWND hWnd = mainView->GetHWND();
		settings->SetSettingInt(L"LastHWND", (int)hWnd);

		//Run loop
		mainView->MessageLoop();
	}
	else
	{
		logger->LogError(L"Init COpenGLView failed !");
		MessageBox(NULL, L"初始化失败！", APP_TITLE, MB_OK | MB_ICONERROR);
		result = -1;
	}

	logger->Log(L"Wait for Destroy..");

	mainView->Destroy();
	mainView->WaitDestroyComplete();

QUIT_AND_DESTROY:
	logger->Log(L"Quiting..");

	FreeConsole();

	fclose(fileIn);
	fclose(file);
	fclose(fileErr);

	CCursor::Destroy();
	CCMeshLoader::Destroy();
	delete settings;
	delete gameRenderer;
	delete mainView;
	delete logger;

    return result;
}

HINSTANCE CApp::GetHInstance()
{
	return hInst;
}
LoggerInternal* CApp::GetLogger()
{
	return logger;
}
SettingHlp* CApp::GetSettings()
{
	return settings;
}
LPWSTR CApp::GetCurrentDir()
{
	return currentDir;
}
LPCSTR CApp::GetCurrentDirA()
{
	return currentDirA;
}
LPWSTR CApp::GetCurrentPath()
{
	return currentPath;
}
HWND CApp::GetConsoleHWND()
{
	return hConsole;
}

int CApp::GetCommandIsProvideFile(std::wstring* path) {
	if (appArgCount > 1) {
		if (_waccess_s(appArgList[1], 0) == 0) {
			*path = appArgList[1];
			return 1;
		}
	}
	return 0;
}
