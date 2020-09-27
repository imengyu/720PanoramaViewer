#include "stdafx.h"
#include "COpenGLView.h"
#include <stdio.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>

extern LoggerInternal* logger;

COpenGLView::COpenGLView(COpenGLRenderer *renderer)
{
	this->OpenGLRenderer = renderer;
	this->OpenGLRenderer->View = this;
	memset(SizeText, 0, sizeof(SizeText));
}
COpenGLView::~COpenGLView()
{
}

bool COpenGLView::Init(HINSTANCE hInstance, LPCWSTR Title, int Width, int Height, WNDPROC wndproc)
{
	Title = Title;
	TextPadding = 10.0f / (this->Width / 2.0f);
	CustomWndProc = wndproc;

	WNDCLASSEX WndClassEx;

	memset(&WndClassEx, 0, sizeof(WNDCLASSEX));

	WndClassEx.cbSize = sizeof(WNDCLASSEX);
	WndClassEx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClassEx.lpfnWndProc = WndProc;
	WndClassEx.hInstance = hInstance;
	WndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClassEx.lpszClassName = L"Win32OpenGLWindow";

	if (RegisterClassEx(&WndClassEx) == 0)
	{
		logger->LogError2(L"RegisterClassEx failed!");
		return false;
	}

	DWORD Style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	hWnd = CreateWindowEx(WS_EX_APPWINDOW, WndClassEx.lpszClassName, Title, Style, 0, 0, Width, Height, NULL, NULL, hInstance, NULL);

	if (hWnd == NULL)
	{
		logger->LogError2(L"CreateWindowEx failed!");
		return false;
	}

	SetWindowLongW(hWnd, GWL_USERDATA, (LONG)this);

	hDC = GetDC(hWnd);
	if (hDC == NULL)
	{
		logger->LogError2(L"GetDC failed!");
		return false;
	}

	PIXELFORMATDESCRIPTOR pfd;

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int PixelFormat = ChoosePixelFormat(hDC, &pfd);

	if (PixelFormat == 0)
	{
		logger->LogError2(L"ChoosePixelFormat failed!");
		return false;
	}

	if (SetPixelFormat(hDC, PixelFormat, &pfd) == FALSE)
	{
		logger->LogError2(L"SetPixelFormat failed!");
		return false;
	}

	Rendering = true;
	RenderThreadRunning = true;
	ViewportChanged = true;
	hThread = CreateThread(0, 0, RenderThread, this, 0, 0);
	startTime = GetTickCount();

	inited = true;

	return true;
}

bool COpenGLView::InitGl() {
	hGLRC = wglCreateContext(hDC);
	if (hGLRC == NULL)
	{
		logger->LogError2(L"wglCreateContext failed!");
		return false;
	}

	if (wglMakeCurrent(hDC, hGLRC) == FALSE)
	{
		logger->LogError2(L"wglMakeCurrent failed!");
		return false;
	}

	if (glewInit() != GLEW_OK)
	{
		logger->LogError2(L"glewInit failed!");
		return false;
	}

	if (WGLEW_EXT_swap_control)
	{
		wglSwapIntervalEXT(0);
	}

	sprintf_s(SizeText, "%dx%d - %s", Width, Height, (char*)glGetString(GL_RENDERER));

	return true;
}
void COpenGLView::InitImgui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//设置颜色风格
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplOpenGL3_Init();
}

void COpenGLView::Show(bool Maximized)
{
	RECT dRect, wRect, cRect;

	GetWindowRect(GetDesktopWindow(), &dRect);
	GetWindowRect(hWnd, &wRect);
	GetClientRect(hWnd, &cRect);

	wRect.right += Width - cRect.right;
	wRect.bottom += Height - cRect.bottom;
	wRect.right -= wRect.left;
	wRect.bottom -= wRect.top;
	wRect.left = dRect.right / 2 - wRect.right / 2;
	wRect.top = dRect.bottom / 2 - wRect.bottom / 2;

	MoveWindow(hWnd, wRect.left, wRect.top, wRect.right, wRect.bottom, FALSE);
	
	ShowWindow(hWnd, Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
}
void COpenGLView::MessageLoop()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
void COpenGLView::Destroy()
{
	Rendering = false;
	RenderThreadRunning = false;
	Destroying = true;
	DestroyWindow(hWnd);
}
void COpenGLView::DestroyRender()
{
	if (inited) {
		inited = false;

		if (OpenGLRenderer) OpenGLRenderer->Destroy();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		wglDeleteContext(hGLRC);
	}
}
void COpenGLView::Render() {
	if (Rendering)
	{
		//绘制
		glClear(GL_COLOR_BUFFER_BIT);

		//绘制UI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		RenderUI();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (Rendering && OpenGLRenderer) OpenGLRenderer->Render(currentFps);
	}
}
void COpenGLView::RenderUI()
{
	//绘制信息文字
	if(ShowInfoOverlay)
		DrawViewInfoOverlay(&ShowInfoOverlay);
	//
	if (OpenGLRenderer) OpenGLRenderer->RenderUI();
}
	
void COpenGLView::DrawViewInfoOverlay(bool* p_open)
{
	const float DISTANCE = 10.0f;
	static int corner = 0;
	ImGuiIO& io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	if (corner != -1)
	{
		window_flags |= ImGuiWindowFlags_NoMove;
		ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - (DISTANCE * 2) : (DISTANCE * 2));
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	}
	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	if (ImGui::Begin("overlay", p_open, window_flags))
	{
		ImGui::Text("Render info \n(right-click to change position)");
		ImGui::Separator();
		ImGui::Text("FPS: %f", currentFps);
		ImGui::Text(SizeText);
		if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
		else
			ImGui::Text("Mouse Position: <invalid>");

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
			if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
			if (p_open && ImGui::MenuItem("Close")) *p_open = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI COpenGLView:: WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	COpenGLView *view = (COpenGLView*)GetWindowLongW(hWnd, GWL_USERDATA);
	ImGui_ImplWin32_WndProcHandler(hWnd, uiMsg, wParam, lParam);
	if (view) {
		switch (uiMsg)
		{
		case WM_KEYDOWN: {

			break;
		}
		case WM_KEYUP: {

			break;
		}
		case WM_SHOWWINDOW: {
			view->Rendering = wParam == TRUE;
			break;
		}
		case WM_CLOSE: {
			view->Destroy();
			break;
		}
		case WM_SIZE: {
			switch (wParam)
			{
			case SIZE_MAXHIDE:
			case SIZE_MINIMIZED:
				view->Rendering = false;
				break;
			case SIZE_MAXIMIZED:
			case SIZE_MAXSHOW:
			case SIZE_RESTORED:
				view->Rendering = true;
				break;
			}
			view->ViewportChanged = true;
			break;
		}
		}
		if (view->CustomWndProc)
			return view->CustomWndProc(hWnd, uiMsg, wParam, lParam);
	}
	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}
DWORD WINAPI COpenGLView::RenderThread(LPVOID lpParam)
{
	COpenGLView* view = (COpenGLView*)lpParam;
	double deltaTime = 0;;

	logger->Log2(L"RenderThread start!");

	//Init Opengl
	if (!view->InitGl()) {
		logger->LogError2(L"InitGl failed!");
		return -1;
	}

	view->InitImgui();

	//Init render
	if (view->OpenGLRenderer)
		if (!view->OpenGLRenderer->Init()) {
			logger->LogError2(L"OpenGLRenderer init failed!");
			return -1;
		}

	while (view->RenderThreadRunning) {

		//Change viewport
		if (view->ViewportChanged) {
			RECT rc; GetClientRect(view->hWnd, &rc);
			view->OnSize(rc.right - rc.left, rc.bottom - rc.top);
			view->ViewportChanged = false;
		}

		//limit fps
		view->lastTime = GetTickCount();
		deltaTime = (1000.0 / view->LimitFps) - (view->drawTime);
		while (deltaTime > 0) {
			if (deltaTime > 20) {
				Sleep(20);
				deltaTime -= 20;
			} else if (deltaTime > 10) {
				Sleep(10);
				deltaTime -= 10;
			} else if (deltaTime > 5) {
				Sleep(5);
				deltaTime-= 5;
			} else {
				Sleep(1);
				deltaTime--;
			}
		}

		//Render
		if (view->Rendering) {
			view->Render();
			SwapBuffers(view->hDC);
		}

		//calc fps
		view->currentTime = GetTickCount();
		view->drawTime = view->currentTime - view->lastTime;
		if (view->drawTime > 0)
			view->currentFps = 1000.0f / view->drawTime;
		view->time = view->currentTime - view->startTime;
	}

	view->DestroyRender();
	view->MarkDestroyComplete();

	logger->Log2(L"RenderThread destroyed!");

	return 0;
}

void COpenGLView::OnSize(int Width, int Height)
{
	this->Width = Width;
	this->Height = Height;

	sprintf_s(SizeText, "%dx%d - %s", Width, Height, (char*)glGetString(GL_RENDERER));

	if (OpenGLRenderer) OpenGLRenderer->Resize(Width, Height);
}

glm::vec2 COpenGLView::ViewportPosToGLPos(glm::vec2 pos) {
	return glm::vec2(pos.x / ((float)Width * 2) - 1, -(pos.y / ((float)Height * 2) - 1));
}
glm::vec2 COpenGLView::GLPosToViewportPos(glm::vec2 pos)
{
	return glm::vec2(pos.x +- 1 * (float)Width * 2, -(pos.y + 1 * (float)Height * 2));
}
float COpenGLView::GetTime()
{
	return (float)currentTime / 1000.0f;
}
float COpenGLView::GetDeltaTime() {
	return 1.0f / currentFps;
}
void COpenGLView::CloseView() {
	CloseWindow(hWnd);
	SendMessageW(hWnd, WM_CLOSE, 0, 0);
}

void COpenGLView::MarkDestroyComplete()
{
	Destroying = false;
}
void COpenGLView::WaitDestroyComplete()
{
	while (Destroying)
		Sleep(20);
}

COpenGLRenderer * COpenGLView::GetRenderer()
{
	return OpenGLRenderer;
}
