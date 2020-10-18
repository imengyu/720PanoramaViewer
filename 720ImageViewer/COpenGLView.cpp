#include "stdafx.h"
#include "COpenGLView.h"
#include "CCFileManager.h"
#include "CCRenderGlobal.h"
#include "CCamera.h"
#include "CCShader.h"
#include "CApp.h"
#include "StringHlp.h"
#include "resource.h"
#include <stdio.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>

COpenGLView::COpenGLView(COpenGLRenderer *renderer)
{
	this->OpenGLRenderer = renderer;
	this->OpenGLRenderer->View = this;
	this->logger = CApp::Instance->GetLogger();
	memset(SizeText, 0, sizeof(SizeText));
}
COpenGLView::~COpenGLView()
{
}

bool COpenGLView::Init(HINSTANCE hInstance, LPCWSTR Title, int Width, int Height, WNDPROC wndproc)
{
	this->Title = Title;
	this->Width = Width;
	this->Height = Height;
	this->TextPadding = 10.0f / (this->Width / 2.0f);
	this->CustomWndProc = wndproc;

	if (!CreateViewWindow(hInstance))
		return false;

	Rendering = true;
	RenderThreadRunning = true;
	ViewportChanged = true;
	hThreadRender = CreateThread(0, 0, RenderThread, this, 0, 0);
	hThreadMain = CreateThread(0, 0, MainThread, this, 0, 0);
	startTime = GetTickCount();

	inited = true;
	return true;
}

bool COpenGLView::CreateViewWindow(HINSTANCE hInstance) {
	WNDCLASSEX WndClassEx;

	memset(&WndClassEx, 0, sizeof(WNDCLASSEX));

	WndClassEx.cbSize = sizeof(WNDCLASSEX);
	WndClassEx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClassEx.lpfnWndProc = WndProc;
	WndClassEx.hInstance = hInstance;
	WndClassEx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP));
	WndClassEx.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP));
	WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClassEx.lpszClassName = L"Win32OpenGLWindow";

	if (RegisterClassEx(&WndClassEx) == 0)
	{
		logger->LogError2(L"RegisterClassEx failed!");
		return false;
	}

	DWORD Style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	hWnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_ACCEPTFILES, WndClassEx.lpszClassName, Title, Style, 0, 0, Width, Height, NULL, NULL, hInstance, NULL);

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

	char*iniFilePath = StringHlp::UnicodeToAnsi(CCFileManager::GetDirResourcePath(L"config", L"imgui.ini").c_str());
	imguiIniPath = iniFilePath;
	delete iniFilePath;

	io.IniFilename = imguiIniPath.c_str();
	static ImVector<ImWchar> myRange;
	ImFontGlyphRangesBuilder myGlyph;

	myGlyph.AddText(u8"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,./<>?;:\"'{}[]|\\+_-=()：；\
*&^%$#@!~`，。《》￥ 关于文件这是一个简易的全景图查看软件染支持多种投影方式可快速打开您浏览程序信息好欢迎使用请先提渲\
示确定设置模式帮助能全屏调试退出显示控制台配使用球面平小行星水晶球单闭当前载入中稍后此案像该球体轴分段失败误错不灰度色或位格\
非常大很抱歉我们暂时");
	myGlyph.BuildRanges(&myRange);

	//io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 16.5f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
	io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 16.5f, NULL, myRange.Data);

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
void COpenGLView::Active()
{
	SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
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
	if (!Destroying) {
		closeActived = true;
		Rendering = false;
		RenderThreadRunning = false;
		Destroying = true;
		DestroyWindow(hWnd);
	}
}
void COpenGLView::DestroyRender()
{
	Rendering = false;

	if (inited) {
		inited = false;

		if (beforeQuitCallback)
			beforeQuitCallback(this);

		if (OpenGLRenderer) {
			OpenGLRenderer->Destroy();
			OpenGLRenderer = nullptr;
		}
		if(Camera) {
			delete Camera;
			Camera = nullptr;
		}

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

		//清空
		if (Camera) 
			glClearColor(Camera->Background.r, Camera->Background.g, Camera->Background.b, Camera->Background.a);

		//绘制
		if (OpenGLRenderer) OpenGLRenderer->Render(currentFps);

		//绘制UI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//绘制界面
		RenderUI();
		if (!Camera) 
			DrawNoCameraOverlay();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}
void COpenGLView::RenderUI()
{
	//绘制信息文字
	if(ShowInfoOverlay) DrawViewInfoOverlay(&ShowInfoOverlay);
	//
	if (OpenGLRenderer) OpenGLRenderer->RenderUI();
}
	
void COpenGLView::DrawNoCameraOverlay() {
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(io.DisplaySize);
	ImGui::SetNextWindowBgAlpha(0.6f);
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
	if (ImGui::Begin("no_camera_rendering_box", 0, window_flags))
	{
		ImGui::Text(u8"No camear rendering");
		ImGui::End();
	}
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
		ImGui::Text("FPS: %0.2f (%d ms) Max : %0.2f (%d ms)", currentFps, drawTime, currentMaxFps, drawTimeReal);
		ImGui::Text("Last Sleep: %d ms", (int)lastSleepTime);
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

void COpenGLView::SetViewText(const char* text)
{
	SetWindowTextA(hWnd, text);
}
void COpenGLView::SetViewText(const wchar_t* text)
{
	SetWindowTextW(hWnd, text);
}

void COpenGLView::CalcMainCameraProjection(CCShader* shader)
{
	if (Camera) {

		//摄像机矩阵变换
		glm::mat4 view = Camera->GetViewMatrix();
		glUniformMatrix4fv(shader->viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		//摄像机投影
		glm::mat4 projection = Camera->Projection == CCameraProjection::Perspective ?
			glm::perspective(glm::radians(Camera->FiledOfView), (float)Width / (float)Height, Camera->ClippingNear, Camera->ClippingFar) :
			glm::ortho(-(float)Width / (float)Height, (float)Width / (float)Height, Camera->OrthographicSize, 0.0f, Camera->ClippingNear, Camera->ClippingFar);
		glUniformMatrix4fv(shader->projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	}
}

LRESULT WINAPI COpenGLView:: WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	COpenGLView *view = (COpenGLView*)GetWindowLongW(hWnd, GWL_USERDATA);
	ImGui_ImplWin32_WndProcHandler(hWnd, uiMsg, wParam, lParam);
	if (view) {
		switch (uiMsg)
		{
		case WM_KEYDOWN: {
			view->HandleDownKey(wParam);
			break;
		}
		case WM_KEYUP: {
			view->HandleUpKey(wParam);
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
				view->ViewportChanged = true;
				break;
			}
			view->ViewportChanged = true;
			break;
		}
		case WM_MOUSEMOVE: { 
			if (view->mouseCallback != nullptr && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyWindowHovered()) {
				short xPos = LOWORD(lParam), yPos = HIWORD(lParam);
				view->mouseCallback(view, (float)xPos, (float)yPos, wParam, ViewMouseEventType::ViewMouseMouseMove);
			}
			break;
		}
		case WM_LBUTTONDOWN: {
			view->CallMouseCallback(wParam, lParam, ViewMouseEventType::ViewMouseMouseDown);
			break;
		}
		case WM_LBUTTONUP: {
			view->CallMouseCallback(MK_LBUTTON, lParam, ViewMouseEventType::ViewMouseMouseUp);
			break;
		}
		case WM_RBUTTONDOWN: {
			view->CallMouseCallback(wParam, lParam, ViewMouseEventType::ViewMouseMouseDown);
			break;
		}
		case WM_RBUTTONUP: {
			view->CallMouseCallback(MK_RBUTTON, lParam, ViewMouseEventType::ViewMouseMouseUp);
			break;
		}
		case WM_MBUTTONDOWN: {
			view->CallMouseCallback(wParam, lParam, ViewMouseEventType::ViewMouseMouseDown);
			break;
		}
		case WM_MBUTTONUP: {
			view->CallMouseCallback(MK_MBUTTON, lParam, ViewMouseEventType::ViewMouseMouseUp);
			break;
		}
		case WM_MOUSEWHEEL: {
			if (view->scrollCallback != nullptr && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyWindowHovered()) {
				short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
				view->scrollCallback(view, 0, (float)zDelta, wParam, ViewMouseEventType::ViewMouseMouseWhell);
			}
			break;
		}
		default:
			break;
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

	view->logger->Log2(L"RenderThread start!");

	//Init Opengl
	if (!view->InitGl()) {
		view->logger->LogError2(L"InitGl failed!");
		return -1;
	}

	view->InitImgui();

	//Init render
	if (view->OpenGLRenderer && !view->OpenGLRenderer->Init()) {
			view->logger->LogError2(L"OpenGLRenderer init failed!");
			return -1;
		}

	//Loop
	while (view->RenderThreadRunning) {

		//limit fps
		view->lastTime = GetTickCount();
		view->lastSleepTime = (1000.0 / view->LimitFps) - (double)(view->drawTime);
		if (view->lastSleepTime > 0) Sleep((DWORD)view->lastSleepTime);

		//Change viewport
		if (view->ViewportChanged) {
			RECT rc; GetClientRect(view->hWnd, &rc);
			view->OnSize(rc.right - rc.left, rc.bottom - rc.top);
			view->ViewportChanged = false;
		}

		view->drawLastTime = GetTickCount();
		view->UpdateTicked = true;

		//Render
		if (view->Rendering) {
			view->Render();
			SwapBuffers(view->hDC);
		}

		//calc fps
		view->currentTime = GetTickCount();
		view->drawTime = view->currentTime - view->lastTime;
		view->drawTimeReal = view->currentTime - view->drawLastTime;
		if (view->drawTime > 0)
			view->currentFps = 1000.0f / view->drawTime;
		if (view->drawTimeReal > 0)
			view->currentMaxFps = 1000.0f / view->drawTimeReal;
		view->time = view->currentTime - view->startTime;


	}

	view->DestroyRender();
	view->MarkDestroyComplete();

	view->logger->Log2(L"RenderThread destroyed!");

	return 0;
}
DWORD WINAPI COpenGLView::MainThread(LPVOID lpParam)
{
	COpenGLView* view = (COpenGLView*)lpParam;

	view->logger->Log2(L"MainThread created!");

	while (view->RenderThreadRunning) {

		if (view->UpdateTicked) {
			if(view->OpenGLRenderer) view->OpenGLRenderer->Update();
			view->UpdateTicked = false;
		} else Sleep(10);
	}

	view->logger->Log2(L"MainThread destroyed!");

	return 0;
}

void COpenGLView::CallMouseCallback(WPARAM wParam, LPARAM lParam, ViewMouseEventType type) {
	
	if (mouseCallback != nullptr && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyWindowHovered()) {
		WORD xPos = LOWORD(lParam), yPos = HIWORD(lParam);
		mouseCallback(this, (float)xPos, (float)yPos, wParam, type);
	}
}
void COpenGLView::OnSize(int Width, int Height)
{
	if (Width != 0 && Height != 0) 
	{
		this->Width = Width;
		this->Height = Height;

		sprintf_s(SizeText, "%dx%d - %s", Width, Height, (char*)glGetString(GL_RENDERER));

		logger->Log2(L"Resize view : %dx%d", Width, Height);

		if (OpenGLRenderer) OpenGLRenderer->Resize(Width, Height);
	}
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
	if(currentFps != 0)
		return 1.0f / currentFps;
	return 0.01f;
}
void COpenGLView::CloseView() {
	if (!closeActived) {
		CloseWindow(hWnd);
		::SendMessage(hWnd, WM_CLOSE, 0, 0);
	}
}
void COpenGLView::SetCamera(CCamera* camera)
{
	Camera = camera;
}

int destroyWaitCount = 0;

void COpenGLView::MarkDestroyComplete()
{
	Destroying = false;
}
void COpenGLView::WaitDestroyComplete()
{
	while (Destroying && destroyWaitCount < 111) {
		Sleep(20);
		destroyWaitCount++;
	}
}

void COpenGLView::SetBeforeQuitCallback(BeforeQuitCallback beforeQuitCallback)
{
	this->beforeQuitCallback = beforeQuitCallback;
}
void COpenGLView::SetMouseCallback(ViewMouseCallback mouseCallback)
{
	this->mouseCallback = mouseCallback;
}
void COpenGLView::SetScrollCallback(ViewMouseCallback mouseCallback)
{
	this->scrollCallback = mouseCallback;
}

int COpenGLView::AddKeyInKeyList(int *list, int code) {
	for (int i = 0; i < MAX_KEY_LIST; i++) {
		if (list[i] == 0)
		{
			list[i] = code;
			return i;
		}
	}	
	return -1;
}
int COpenGLView::IsKeyInKeyListExists(int* list, int code) {
	for (int i = 0; i < MAX_KEY_LIST; i++) {
		if (list[i] == code)
			return i;
	}
	return -1;
}
void COpenGLView::HandleDownKey(int code) {
	
	int upIndex = IsKeyInKeyListExists(UpedKeys, code);
	if (upIndex > -1) UpedKeys[upIndex] = 0;

	int downIndex = IsKeyInKeyListExists(DownedKeys, code);
	if (downIndex == -1) AddKeyInKeyList(DownedKeys, code);

}
void COpenGLView::HandleUpKey(int code) {
	int upIndex = IsKeyInKeyListExists(UpedKeys, code);
	if (upIndex == -1) AddKeyInKeyList(UpedKeys, code);

	int downIndex = IsKeyInKeyListExists(DownedKeys, code);
	if (downIndex > -1) DownedKeys[downIndex] = 0;

}

bool COpenGLView::GetKeyPress(int code) {
	return IsKeyInKeyListExists(DownedKeys, code) > -1;
}
bool COpenGLView::GetKeyDown(int code) {
	int up = IsKeyInKeyListExists(DownedKeys, code);
	if (up > -1) {
		DownedKeys[up] = 0;
		return true;
	}
	return  false;
}
bool COpenGLView::GetKeyUp(int code) {
	int up =  IsKeyInKeyListExists(UpedKeys, code);
	if (up > -1) {
		UpedKeys[up] = 0;
		return true;
	}
	return  false;
}
void COpenGLView::Resize(int w, int h, bool moveToCenter) {
	int newx = moveToCenter ?(GetSystemMetrics(SM_CXSCREEN) - w) / 2 : 0,
		newy = moveToCenter ? (GetSystemMetrics(SM_CYSCREEN) - h) / 2 - 30 : 0;
	SetWindowPos(hWnd, NULL, newx, newy, w, h, SWP_NOZORDER | (moveToCenter ? 0 : SWP_NOMOVE));
}
void COpenGLView::UpdateFullScreenState() {
	if (IsFullScreen) {
		SetWindowLong(hWnd, GWL_STYLE, WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP);
		ShowWindow(hWnd, SW_MAXIMIZE);
	}
	else {
		SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		ShowWindow(hWnd, SW_RESTORE);
	}
}
void COpenGLView::SetFullScreen(bool full) {
	if (IsFullScreen != full) {
		IsFullScreen = full;
		UpdateFullScreenState();
	}
}
bool COpenGLView::GetIsFullScreen() {
	return IsFullScreen;
}

void COpenGLView::SetToLowerFpsMode() {
	lastSetFps = LimitFps;
	LimitFps = 2.0f;
}
void COpenGLView::QuitLowerFpsMode() {
	if (lastSetFps > 0) {
		LimitFps = lastSetFps;
		lastSetFps = 0;
	}
}

LRESULT COpenGLView::SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam) {
	return ::SendMessage(hWnd, Msg, wParam, lParam);
}

void COpenGLView::MouseCapture() { SetCapture(hWnd); }
void COpenGLView::ReleaseCapture() { ::ReleaseCapture(); }

COpenGLRenderer * COpenGLView::GetRenderer()
{
	return OpenGLRenderer;
}
HWND COpenGLView::GetHWND()
{
	return hWnd;
}
