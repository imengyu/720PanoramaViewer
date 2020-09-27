// 3DImageViewer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "COpenGLView.h"
#include "CGameRenderer.h"

#define APP_TITLE L"3D Image Viewer"

LoggerInternal* logger = nullptr;
COpenGLView* mainView = nullptr;
CGameRenderer * gameRenderer = nullptr;
HINSTANCE hInst = NULL;

LRESULT CALLBACK wndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

int getCommandIsProvideFile(int argc, char* argv[], std::string *path) {
	if (argc > 1) {
		if (_access(argv[1], 0) == 0) {
			*path = argv[1];
			return 1;
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	int result = 0;
	hInst = GetModuleHandle(NULL);

	logger = new LoggerInternal();
	logger->SetLogLevel(LogLevel::LogLevelText);
	logger->SetLogOutPut(LogOutPut::LogOutPutConsolne);

	gameRenderer = new CGameRenderer();
	mainView = new COpenGLView(gameRenderer);

	if (mainView->Init(hInst, APP_TITLE, 800, 600, wndProc))
	{
		//Test command for file
		std::string *filePath = new std::string();
		if (getCommandIsProvideFile(argc, argv, filePath)) {
			logger->Log(L"Command input file %hs", filePath->c_str());
			gameRenderer->SetOpenFilePath(*filePath);
		}
		delete filePath;

		logger->Log(L"Init ok");

		//Run loop
		mainView->Show();
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

	logger->Log(L"Quiting..");

	delete gameRenderer;
	delete mainView;
	delete logger;

	return result;
}
