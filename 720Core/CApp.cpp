#include "CApp.h"
#include "CCursor.h"
#include "CCMeshLoader.h"
#include "CWindowsOpenGLView.h"
#include "CWindowsGameRenderer.h"
#include "StringSplit.h"
#include "PathHelper.h"
#include <shellapi.h>

CApp* CApp::Instance = nullptr;

bool CAppInternal::Init()
{	
	//命令行
	appArgList = CommandLineToArgvW(GetCommandLine(), &appArgCount);
	if (appArgList == NULL)
	{
		MessageBox(NULL, L"Unable to parse command line", L"Error", MB_OK);
		return false;
	}

	//程序路径处理
	Instance = this;
	hInst = GetModuleHandle(NULL);
	GetModuleFileName(NULL, currentPath, MAX_PATH);
	wcscpy_s(currentDir, currentPath);
	PathRemoveFileSpec(currentDir);
	strcpy_s(currentDirA, CStringHlp::UnicodeToAnsi(currentDir).c_str());

	//将程序的运行路径修改到当前程序所在的目录
	SetCurrentDirectory(Path::GetDirectoryName(std::wstring(currentPath)).c_str());

	InitConsole();

	CCPtrPool::InitPool();

	LoggerInternal::InitConst();

	logger = Logger::GetStaticInstance();
	logger->SetLogLevel(LogLevel::LogLevelText);
	logger->SetLogOutPut(LogOutPut::LogOutPutConsolne);
	logger->InitLogConsoleStdHandle();

	settings = new SettingHlpInternal(CStringHlp::FormatString(L"%s\\config\\config.ini", currentDir).c_str());

	CCursor::Init(hInst);
	CCMeshLoader::Init();

	//命令行读取
	return true;
}
int CAppInternal::Run(int* p)
{
	int result = 0;

	//读取命令行文件
	std::wstring filePath;
	bool hasInputFile = GetCommandIsProvideFile(&filePath);

	//单例判断
	if (CheckMutex())
	{
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
	}

	//初始化
	gameRenderer = new CWindowsGameRendererInternal();
	mainView = new CWindowsOpenGLViewInternal(hInst, hMain, gameRenderer);

	if (mainView->Init())
	{
		if (hasInputFile) {
			logger->Log(L"Command input file %hs", filePath.c_str());
			gameRenderer->SetOpenFilePath(filePath.c_str());
			gameRenderer->MarkShouldOpenFile();
		}

		if (runCallback)
			result = runCallback(runCallbackData, mainView, gameRenderer);

		logger->Log(L"Init ok");
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

	*p = result;
	return result;
}

int CAppInternal::GetCommandIsProvideFile(std::wstring* path) {
	if (appArgCount > 1) {
		if (_waccess_s(appArgList[1], 0) == 0) {
			*path = appArgList[1];
			return 1;
		}
	}
	return 0;
}
bool CAppInternal::CheckMutex()
{
	HANDLE  hMutex = NULL;
	bool forceNotSingle = settings->GetSettingBool(L"NoSingleCheck", FALSE);
	hMutex = CreateMutex(NULL, FALSE, L"720Mutex");
	if (hMutex && (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		CloseHandle(hMutex);
		hMutex = NULL;

		return true;
	}
	return false;
}
void CAppInternal::InitConsole()
{
	AllocConsole();
	hConsole = GetConsoleWindow();
	ShowWindow(hConsole, SW_HIDE);
	SetConsoleTitle(APP_TITLE);

	freopen_s(&fileIn, "CONIN$", "r", stdin);
	freopen_s(&fileErr, "CONOUT$", "w", stderr);
	freopen_s(&file, "CONOUT$", "w", stdout);
}
bool CAppInternal::Destroy()
{
	FreeConsole();

	fclose(fileIn);
	fclose(file);
	fclose(fileErr);

	logger->Log(L"Quiting..");

	LoggerInternal::DestroyConst();
	CCursor::Destroy();
	CCMeshLoader::Destroy();
	CCPtrPool::ReleasePool();
	delete settings;
	delete gameRenderer;
	delete mainView;

	return true;
}
void CAppInternal::SetCurrentHWND(HWND hWnd)
{
	hMain = hWnd;
	settings->SetSettingInt(L"LastHWND", (int)hWnd);
}
void CApp::SetAppRunCallback(void*ptr, CAppRunCallback callback)
{
	runCallbackData = ptr;
	runCallback = callback;
}
