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

#define GAME_FILE_OK 1
#define GAME_FILE_CLOSE_BUT_WILL_OPEN_NEXT 2
#define GAME_FILE_OPEN_FAILED 3

#define GAME_EVENT_GO_FULLSCREEN 1
#define GAME_EVENT_QUIT_FULLSCREEN 2
#define GAME_EVENT_SHOW_RIGHT_MENU 3
#define GAME_EVENT_FILE_DELETE_BACK 4
#define GAME_EVENT_LOADING_STATUS 5
#define GAME_EVENT_IMAGE_INFO_LOADED 6
#define GAME_EVENT_IMAGE_INFO_LOADED2 7
#define GAME_EVENT_CAPTURE_FINISH 8
#define GAME_EVENT_SHADER_FILE_MISSING 9

#define GAME_CUR_CURB 10
#define GAME_CUR_DEF 11
#define GAME_KEY_NEXT 12
#define GAME_KEY_PREV 13
#define GAME_KEY_DEL 14

#define GAME_EVENT_SHADER_NOT_SUPPORT 20

typedef void(*CGameFileStatusChangedCallback)(void* data, bool isOpen, int status);
typedef void(*CGameSampleEventCallback)(void* data, int eventCode, void* param);

class CWindowsGameRenderer : public COpenGLRenderer
{
public:
	CWindowsGameRenderer() {}
	virtual ~CWindowsGameRenderer() {}

	virtual bool SetStatCapture(const wchar_t* path) { return false; }
	virtual bool SetOpenFilePath(const wchar_t* path) { return false; }
	virtual const wchar_t* GetOpenFilePath() { return nullptr; }
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
	virtual const wchar_t* GetCurrentFileInfo(int c) { return nullptr; }
	virtual const wchar_t* GetCurrentFileInfoTitle() { return nullptr; }
	virtual const wchar_t* GetCurrentFileLoadingPrecent() { return nullptr; }

	virtual bool ZoomIn() { return false; }
	virtual bool ZoomOut() { return false; }
	virtual void ZoomReset() { }
	virtual void ZoomBest() { }

	virtual void CallSampleEventCallback(int code, void* param) {}
	virtual void SetFileStatusChangedCallback(CGameFileStatusChangedCallback callback, void*data) {}
	virtual void SetSampleEventCallback(CGameSampleEventCallback callback, void* data) {}
};

#define GAME_IMAGE_INFO_TYPE 2
#define GAME_IMAGE_INFO_DATE 3
#define GAME_IMAGE_INFO_SHOOTING_DATE 4
#define GAME_IMAGE_INFO_SHUTTER_TIME 5
#define GAME_IMAGE_INFO_EXPOSURE_BIAS_VALUE 6
#define GAME_IMAGE_INFO_ISO_SENSITIVITY 7
#define GAME_IMAGE_INFO_FILE_SIZE 8
#define GAME_IMAGE_INFO_RESOLUTION 9
#define GAME_IMAGE_INFO_CAMERA 10
#define GAME_IMAGE_INFO_FOCAL_LENGTH 11
 
#ifdef VR720_EXPORTS

#include "easyexif.h"

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

	bool SetOpenFilePath(const wchar_t* path);
	bool SetStatCapture(const wchar_t* path) {
		should_capture = true;
		nextCaptureSavePath = path;
		return true;
	}
	const wchar_t* GetOpenFilePath();
	void DoOpenFile();
	void MarkShouldOpenFile() { should_open_file = true; }
	void MarkCloseFile(bool delete_after_close) { should_close_file = true;  this->delete_after_close = delete_after_close; }
	void SetRenderQuitFullScreenButton(bool show) { renderQuitFullScreenButton = show; }
	void AddTextureToQueue(CCTexture* tex, int x, int y, int id);
	PanoramaMode GetMode() { return mode; }
	void SetMode(PanoramaMode mode) { SwitchMode(mode); }
	const wchar_t* GetFileLastError() { return fileManager->GetLastError(); }
	const wchar_t* GetCurrentFilePath() { return currentOpenFilePath.c_str(); }
	const wchar_t* GetCurrentFileInfo(int c);
	const wchar_t* GetCurrentFileInfoTitle();
	const wchar_t* GetCurrentFileLoadingPrecent();

	bool SetProperty(const char* name, const wchar_t* val) override;
	bool GetPropertyBool(const char* name) override;

	bool ZoomIn();
	bool ZoomOut();
	void ZoomReset();
	void ZoomBest();

	void CaptureFromGLSurface();

	void SetFileStatusChangedCallback(CGameFileStatusChangedCallback callback, void* data);
	void SetSampleEventCallback(CGameSampleEventCallback callback, void* data);

private:

	Logger* logger = nullptr;
	std::wstring currentOpenFilePath;

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

	bool glinfo_dialog_active = false;
	bool render_dialog_active = false;
	bool debug_tool_active = false;
	bool show_console = false;
	int debug_tool_active_tab = 1;
	bool main_menu_active = true;
	float ui_update_tick = 0.0f;

	float current_fps = 0;
	float current_draw_time = 0;

	bool renderQuitFullScreenButton = false;

	bool currentImageOpened = false;
	std::wstring currentImageInfoTitle;
	std::wstring currentImageChangeDate;
	std::wstring currentImageFileSize;
	std::wstring currentImageResolutionSize;
	ImageType currentImageType = ImageType::Unknow;
	int currentImageAllChunks = 0;
	int currentImageLoadedChunks = 0;
	int currentImageLoadChunks = 0;
	bool currentImageLoading = false;
	bool currentImageHasExifData = false;
	bool currentImageExifDataLoading = false;

	std::wstring currentImageInfoResolution;
	std::wstring currentImageInfoCamera;
	std::wstring currentImageInfoFocalLength;
	std::wstring currentImageInfoISOSpeedRatings;
	std::wstring currentImageInfoExposureBiasValue;
	std::wstring currentImageInfoShutterSpeedValue;
	std::wstring currentImageInfoDateTime;

	easyexif::EXIFInfo currentImageExifInfo;

	float bestZoom = 0;
	float bestOrthoSize = 0;

	bool render_init_finish = false;
	bool should_open_file = false;
	bool should_close_file = false;
	bool should_capture = false;
	std::wstring nextCaptureSavePath;
	bool delete_after_close = false;
	bool destroying = false;
	bool needTestImageAndSplit = false;
	bool firstMouse = true;
	bool lastMoved = false;
	float lastX = 0, lastY = 0, xoffset = 0, yoffset = 0;
	float loopCount = 0;

	void LoadImageInfo();
	void TestSplitImageAndLoadTexture();
	void LoadImageExifInfo();
	static DWORD WINAPI LoadImageInfoThread(LPVOID lpParam);

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

	SampleEventCallbackData sampleEventCallbackData = { 0 };
	FileStatusChangedCallbackData fileStatusChangedCallbackData = { 0 };

	void CallFileStatusChangedCallback(bool isOpen, int status);
	void CallSampleEventCallback(int code, void* param);

	//配置
	bool CheckImageRatio = true;
	bool ReverseRotation = false;

};

#endif
