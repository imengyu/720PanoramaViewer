#pragma once
#include "stdafx.h"
#include "COpenGLRenderer.h"
#include "CCamera.h"
#include "CCPanoramaCamera.h"
#include "CCPanoramaRenderer.h"
#include "CCTextureLoadQueue.h"
#include "CCModel.h"
#include "CCFileManager.h"
#include "CCGUInfo.h"
#include "CAppUIWapper.h"
#include "CApp.h"
#include <vector>

//全景模式
enum PanoramaMode : int16_t {
	PanoramaSphere,
	PanoramaCylinder,
	PanoramaAsteroid,
	PanoramaOuterBall,
	PanoramaMercator,
	PanoramaFull360,
	PanoramaFullOrginal,
	PanoramaModeMax,
};

#define GAME_FILE_OK 2
#define GAME_FILE_OPEN_FAILED 3

#define GAME_EVENT_GO_FULLSCREEN 1
#define GAME_EVENT_QUIT_FULLSCREEN 2
#define GAME_EVENT_SHOW_RIGHT_MENU 3
#define GAME_EVENT_FILE_DELETE_BACK 4
#define GAME_EVENT_LOADING_STATUS 5

typedef void(*CGameFileStatusChangedCallback)(void* data, bool isOpen, int status);
typedef void(*CGameSampleEventCallback)(void* data, int eventCode, void* param);

class CWindowsGameRenderer : public COpenGLRenderer
{
public:
	CWindowsGameRenderer() {}
	virtual ~CWindowsGameRenderer() {}

	virtual void SetOpenFilePath(const wchar_t* path) {}
	virtual void DoOpenFile() {}
	virtual void MarkShouldOpenFile() { }
	virtual void MarkCloseFile(bool delete_after_close) { }
	virtual void SetRenderQuitFullScreenButton(bool show) { }
	virtual void AddTextureToQueue(CCTexture* tex, int x, int y, int id) { }
	virtual CCGUInfo* GetGUInfo() { return nullptr; }
	virtual PanoramaMode GetMode() { return PanoramaMode::PanoramaModeMax; }
	virtual void SetMode(PanoramaMode mode) { }
	virtual const wchar_t* GetFileLastError() { return nullptr; }
	virtual const wchar_t* GetCurrentFilePath() { return nullptr; }
	virtual const wchar_t* GetCurrentFileInfoTitle() { return nullptr; }

	virtual bool ZoomIn() { return false; }
	virtual bool ZoomOut() { return false; }
	virtual void ZoomReset() { }
	virtual void ZoomBest() { }
	virtual void OpenFileAs() { }

	virtual void SetFileStatusChangedCallback(CGameFileStatusChangedCallback callback, void*data) {}
	virtual void SetSampleEventCallback(CGameSampleEventCallback callback, void* data) {}
};

#ifdef VR720_EXPORTS

struct FileStatusChangedCallbackData {
	bool isOpen;
	int status;
	void* data;
	CGameFileStatusChangedCallback callback;
};
struct SampleEventCallbackData {
	void* data;
	int eventCode;
	void* param;
	CGameSampleEventCallback callback;
};

class CImageLoader;
class CWindowsGameRendererInternal : public CWindowsGameRenderer
{
public:
	CWindowsGameRendererInternal();
	~CWindowsGameRendererInternal();

	void SetOpenFilePath(const wchar_t* path);
	void DoOpenFile();
	void MarkShouldOpenFile() { should_open_file = true; }
	void MarkCloseFile(bool delete_after_close) { should_close_file = true;  this->delete_after_close = delete_after_close; }
	void SetRenderQuitFullScreenButton(bool show) { renderQuitFullScreenButton = show; }
	void AddTextureToQueue(CCTexture* tex, int x, int y, int id);
	CCGUInfo* GetGUInfo() { return uiInfo; }
	PanoramaMode GetMode() { return mode; }
	void SetMode(PanoramaMode mode) { SwitchMode(mode); }
	const wchar_t* GetFileLastError() { return fileManager->GetLastError(); }
	const wchar_t* GetCurrentFilePath() { return currentOpenFilePath.c_str(); }
	const wchar_t* GetCurrentFileInfoTitle();

	bool ZoomIn();
	bool ZoomOut();
	void ZoomReset();
	void ZoomBest();
	void OpenFileAs();

	void SetFileStatusChangedCallback(CGameFileStatusChangedCallback callback, void* data);
	void SetSampleEventCallback(CGameSampleEventCallback callback, void* data);

private:

	Logger* logger;

	std::wstring currentOpenFilePath;
	bool fileOpened = false;

	bool Init() override;
	void Render(float FrameTime) override;
	void Update() override;
	void RenderUI() override;
	void Resize(int Width, int Height) override;
	void Destroy() override;

	char* GetPanoramaModeStr(PanoramaMode mode);

	//全景模式
	PanoramaMode mode = PanoramaMode::PanoramaSphere;
	CCPanoramaCamera*camera = nullptr;
	CCPanoramaRenderer* renderer = nullptr;
	CCFileManager*fileManager = nullptr;
	CAppUIWapper*uiWapper = nullptr;
	CCTextureLoadQueue*texLoadQueue = nullptr;

	void ShowErrorDialog();

	//UI控制

	CCGUInfo* uiInfo = nullptr;

	bool debug_tool_active = false;
	bool show_console = false;
	bool show_status_bar = true;
	bool show_fps = true;
	int debug_tool_active_tab = 1;
	bool main_menu_active = true;
	float ui_update_tick = 0.0f;

	bool glinfo_dialog_active = false;
	bool render_dialog_active = false;
	bool file_opened = false;

	bool renderQuitFullScreenButton = false;

	float current_fps = 0;
	float current_draw_time = 0;

	int zoom_slider_value = 50;

	float bestZoom = 0;
	float bestOrthoSize = 0;

	bool render_init_finish = false;
	bool should_open_file = false;
	bool should_close_file = false;
	bool delete_after_close = false;
	bool destroying = false;
	bool needTestImageAndSplit = false;
	bool firstMouse = true;
	bool lastMoved = false;
	float lastX = 0, lastY = 0, xoffset = 0, yoffset = 0;
	float loopCount = 0;

	void LoadImageInfo();
	void TestSplitImageAndLoadTexture();

	TextureLoadQueueDataResult* LoadChunkTexCallback(TextureLoadQueueInfo* info, CCTexture* texture);
	static TextureLoadQueueDataResult* LoadTexCallback(TextureLoadQueueInfo* info, CCTexture* texture, void* data);
	static void FileCloseCallback(void* data);
	static void CameraFOVChanged(void* data, float fov);
	static void CameraOrthoSizeChanged(void* data, float fov);
	static void CameraRotate(void* data, CCPanoramaCamera* cam);
	static void BeforeQuitCallback(COpenGLView* view);

	void UpdateConsoleState();
	void UpdateLoadingState(bool loading);

	void SwitchMode(PanoramaMode mode);

	float MouseSensitivity = 0.1f;
	float RoateSpeed = 20.0f;
	float MoveSpeed = 0.3f;

	bool SplitFullImage = true;

	static void MouseCallback(COpenGLView* view, float x, float y, int button, int type);
	static void ScrollCallback(COpenGLView* view, float x, float y, int button, int type);
	void KeyMoveCallback(CCameraMovement move);

	SettingHlp* settings = nullptr;

	void LoadSettings();
	void SaveSettings();

	SampleEventCallbackData sampleEventCallbackData;
	FileStatusChangedCallbackData fileStatusChangedCallbackData;

	void CallSampleEventCallback(int code, void* param);
	void CallFileStatusChangedCallback(bool isOpen, int status);
};

#endif
