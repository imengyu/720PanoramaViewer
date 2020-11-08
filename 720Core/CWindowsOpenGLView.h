#pragma once
#include "COpenGLRenderer.h"
#include "COpenGLView.h"

class CWindowsOpenGLView : public COpenGLView {

public:
	CWindowsOpenGLView(COpenGLRenderer* renderer) : COpenGLView(renderer) {}
	virtual ~CWindowsOpenGLView() {}

	virtual void Show(bool Maximized = false) {}
	virtual void Active() {}
	virtual void MessageLoop() {}
	virtual bool Init() { return false; }
	virtual void Destroy() {}
	virtual void Render() {}
	virtual void RenderUI() {}

	virtual void CallMouseCallback(WPARAM wParam, LPARAM lParam, ViewMouseEventType type) {}
	virtual void OnSize(int Width, int Height) {}

	virtual float GetTime() { return 0; }
	virtual float GetCurrentFps() { return 0; }
	virtual float GetDrawTime() { return 0; }
	virtual float GetDeltaTime() { return 0; }

	virtual void CloseView() {}
	virtual void MarkDestroyComplete() {}
	virtual void WaitDestroyComplete() {}

	virtual void Resize(int w, int h, bool moveToCenter) {}
	virtual void UpdateFullScreenState() {}
	virtual void SetFullScreen(bool full) {}

	virtual void SetEnabled(bool en) {}

	virtual void SetToLowerFpsMode() {}
	virtual void QuitLowerFpsMode() {}

	virtual LRESULT SendWindowsMessage(UINT Msg, WPARAM wParam, LPARAM lParam) { return 0; }
	virtual void SetWindowsMessageTarget(HWND hWnd) {}

	virtual void MouseCapture() {}
	virtual void ReleaseCapture() {}

	virtual void SetVisible(bool visible) { }
	virtual HWND GetHWND() { return 0; }

	//Settings
	//*************************************

	float LimitFps = 25.0f;
	float FixedTimeStep = 10.0f;

	bool ShowInfoOverlay = true;
	bool IsFullScreen = false;
};

#ifdef VR720_EXPORTS

#include <imgui.h>

class CCamera;
class CCShader;
class CCRenderGlobal;
class CWindowsOpenGLViewInternal : public CWindowsOpenGLView
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
	HWND parentWindow = nullptr;
	glm::vec2 lastSize;
	bool isChildWindow = false;

	bool ViewportChanged = false;
	bool UpdateTicked = false;

	bool RenderingEnabled = true;

	Logger* logger = nullptr;

	bool CreateViewWindow(HINSTANCE hInstance);
	bool InitGl();
	void InitImgui();
	void DrawNoCameraOverlay();
	void DrawViewInfoOverlay(bool* p_open);
	void DestroyRender();

public:

	CWindowsOpenGLViewInternal(HINSTANCE hInstance, LPCWSTR Title, int Width, int Height, WNDPROC wndproc, COpenGLRenderer* renderer);
	CWindowsOpenGLViewInternal(HINSTANCE hInstance, HWND parentHwnd, COpenGLRenderer* renderer);
	~CWindowsOpenGLViewInternal();

	void Show(bool Maximized = false);
	void Active();
	void MessageLoop();
	bool Init();
	void Destroy();
	void Render();
	void RenderUI();

	void CallMouseCallback(WPARAM wParam, LPARAM lParam, ViewMouseEventType type);
	void OnSize(int Width, int Height);

	float GetTime();
	float GetCurrentFps();
	float GetDrawTime();
	float GetDeltaTime();

	void CloseView();
	void MarkDestroyComplete();
	void WaitDestroyComplete();

	void Resize(int w, int h, bool moveToCenter);
	void UpdateFullScreenState();
	void SetFullScreen(bool full);

	void SetEnabled(bool en);

	void SetToLowerFpsMode();
	void QuitLowerFpsMode();

	LRESULT SendWindowsMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	void SetWindowsMessageTarget(HWND hWnd);

	void MouseCapture();
	void ReleaseCapture();

	HWND GetHWND();
	void SetVisible(bool visible);

private:

	static LRESULT __stdcall WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI RenderThread(LPVOID lpParam);
	static DWORD __stdcall MainThread(LPVOID lpParam);

	HWND windowsMessageTarget = NULL;
	bool closeActived = false;
	float lastSetFps = 0;
};

#endif
