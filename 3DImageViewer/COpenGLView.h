#pragma once
#include "COpenGLRenderer.h"
#include "CCamera.h"
#include <imgui.h>

typedef void(*ViewMouseCallback)(COpenGLView* view, float xpos, float ypos, int button, int type);

const int MAX_KEY_LIST = 8;

enum ViewMouseEventType {
	ViewMouseMouseDown,
	ViewMouseMouseUp,
	ViewMouseMouseMove,
	ViewMouseMouseWhell,
};

class COpenGLView
{
protected:
	LPCWSTR Title = L"";
	HWND hWnd = NULL;
	HDC hDC = NULL;
	HGLRC hGLRC = NULL;
	COpenGLRenderer *OpenGLRenderer = NULL;
	HANDLE hThreadRender = NULL;
	HANDLE hThreadMain = NULL;
	float currentFps = 0.0f;

private:
	DWORD startTime = 0;
	DWORD currentTime = 0;
	DWORD lastTime = 0;
	DWORD time = 0;
	DWORD drawTime = 0;
	bool inited = false;
	char SizeText[250];
	float TextPadding = 0;
	bool Rendering = false;
	bool RenderThreadRunning = false;
	bool Destroying = false;
	WNDPROC CustomWndProc = nullptr;

	int DownedKeys[MAX_KEY_LIST];
	int UpedKeys[MAX_KEY_LIST];

	bool ViewportChanged = false;
	bool UpdateTicked = false;

	LoggerInternal* logger = nullptr;

	ViewMouseCallback scrollCallback = nullptr;
	ViewMouseCallback mouseCallback = nullptr;

	bool CreateViewWindow(HINSTANCE hInstance);
	bool InitGl();
	void InitImgui();
	void DrawNoCameraOverlay();
	void DrawViewInfoOverlay(bool* p_open);
	void DestroyRender();
public:
	COpenGLView(COpenGLRenderer *renderer);
	~COpenGLView();

	int Width = 800, Height = 600;

	bool Init(HINSTANCE hInstance, LPCWSTR Title, int Width, int Height, WNDPROC wndproc);
	void Show(bool Maximized = false);
	void MessageLoop();
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

	glm::vec2  ViewportPosToGLPos(glm::vec2 pos);
	glm::vec2 GLPosToViewportPos(glm::vec2 pos);

	CCamera* Camera = nullptr; 

	float LimitFps = 25.0f;
	float FixedTimeStep = 10.0f;

	float GetTime();
	float GetDeltaTime();

	void SetCamera(CCamera* Camera);
	void SetCameraLoc(GLint viewLoc, GLint projectionLoc);

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
	bool GetIsFullScreen();

	void SetToLowerFpsMode();
	void QuitLowerFpsMode();

	LRESULT SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam);

	void MouseCapture();
	void ReleaseCapture();

	COpenGLRenderer * GetRenderer();

	HWND GetHWND();

	void SetMouseCallback(ViewMouseCallback mouseCallback);
	void SetScrollCallback(ViewMouseCallback mouseCallback);
private:

	bool closeActived = false;
	float lastSetFps = 0;
	GLint viewLoc = -1;
	GLint projectionLoc = -1;

	int AddKeyInKeyList(int* list, int code);
	int IsKeyInKeyListExists(int* list, int code);
	void HandleDownKey(int code);
	void HandleUpKey(int code);


};

