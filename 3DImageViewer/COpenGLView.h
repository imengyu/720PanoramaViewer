#pragma once
#include "COpenGLRenderer.h"
#include "CCamera.h"
#include <imgui.h>

class COpenGLView
{
protected:
	LPCWSTR Title = L"";
	HWND hWnd = NULL;
	HDC hDC = NULL;
	HGLRC hGLRC = NULL;
	COpenGLRenderer *OpenGLRenderer = NULL;
	HANDLE hThread = NULL;
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
	std::vector<int> DownedKeys;

	bool ViewportChanged = false;


	bool InitGl();
	void InitImgui();
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

	static LRESULT __stdcall WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI RenderThread(LPVOID lpParam);

	void OnSize(int Width, int Height);

	glm::vec2  ViewportPosToGLPos(glm::vec2 pos);
	glm::vec2 GLPosToViewportPos(glm::vec2 pos);

	CCamera Camera = CCamera(glm::vec3(0.0f, 0.0f, 3.0f));

	double LimitFps = 10;

	float GetTime();
	float GetDeltaTime();

	void CloseView();
	void MarkDestroyComplete();
	void WaitDestroyComplete();

	bool ShowInfoOverlay = true;

	COpenGLRenderer * GetRenderer();
};

