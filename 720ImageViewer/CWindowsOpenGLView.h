#pragma once
#include "COpenGLRenderer.h"
#include "COpenGLView.h"
#include <imgui.h>

class CCamera;
class CCShader;
class CCRenderGlobal;
class CWindowsOpenGLView : public COpenGLView
{
protected:
	LPCWSTR Title = L"";
	HWND hWnd = NULL;
	HDC hDC = NULL;
	HGLRC hGLRC = NULL;
	HANDLE hThreadRender = NULL;
	HANDLE hThreadMain = NULL;
	float currentFps = 0.0f;
	float currentMaxFps = 0.0f;

private:
	DWORD startTime = 0;
	DWORD currentTime = 0;
	DWORD lastTime = 0;
	DWORD time = 0;
	DWORD drawTime = 0;
	DWORD drawTimeReal = 0;
	DWORD drawLastTime = 0;
	double lastSleepTime = 0;

	std::string imguiIniPath;

	bool inited = false;
	char SizeText[250];
	bool Rendering = false;
	bool RenderThreadRunning = false;
	bool Destroying = false;
	WNDPROC CustomWndProc = nullptr;
	HINSTANCE hInstance = nullptr;
	glm::vec2 lastSize;

	int DownedKeys[MAX_KEY_LIST];
	int UpedKeys[MAX_KEY_LIST];

	bool ViewportChanged = false;
	bool UpdateTicked = false;

	Logger* logger = nullptr;

	bool CreateViewWindow(HINSTANCE hInstance);
	bool InitGl();
	void InitImgui();
	void DrawNoCameraOverlay();
	void DrawViewInfoOverlay(bool* p_open);
	void DestroyRender();

public:

	CWindowsOpenGLView(HINSTANCE hInstance, LPCWSTR Title, int Width, int Height, WNDPROC wndproc, COpenGLRenderer* renderer);
	~CWindowsOpenGLView();

	void Show(bool Maximized = false);
	void Active();
	void MessageLoop();
	bool Init();
	void Destroy();
	void Render();
	void RenderUI();

	void SetViewText(const char * text);
	void SetViewText(const wchar_t* text);

	static LRESULT __stdcall WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI RenderThread(LPVOID lpParam);
	static DWORD __stdcall MainThread(LPVOID lpParam);

	void CallMouseCallback(WPARAM wParam, LPARAM lParam, ViewMouseEventType type);
	void OnSize(int Width, int Height);

	float LimitFps = 25.0f;
	float FixedTimeStep = 10.0f;

	float GetTime();
	float GetCurrentFps();
	float GetDrawTime();
	float GetDeltaTime();

	void CloseView();
	void MarkDestroyComplete();
	void WaitDestroyComplete();

	bool ShowInfoOverlay = true;
	bool IsFullScreen = false;

	bool GetKeyPress(int code);
	bool GetKeyDown(int code);
	bool GetKeyUp(int code);

	void Resize(int w, int h, bool moveToCenter);
	void UpdateFullScreenState();
	void SetFullScreen(bool full);

	void SetToLowerFpsMode();
	void QuitLowerFpsMode();

	LRESULT SendWindowsMessage(UINT Msg, WPARAM wParam, LPARAM lParam);

	void MouseCapture();
	void ReleaseCapture();

	HWND GetHWND();

private:

	bool closeActived = false;
	float lastSetFps = 0;

	int AddKeyInKeyList(int* list, int code);
	int IsKeyInKeyListExists(int* list, int code);
	void HandleDownKey(int code);
	void HandleUpKey(int code);
};

