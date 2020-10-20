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
enum PanoramaMode {
	PanoramaSphere,
	PanoramaAsteroid,
	PanoramaOuterBall,
	PanoramaCylinder,
	PanoramaMercator,
	PanoramaFull360,
};

class CImageLoader;
class CGameRenderer : public COpenGLRenderer
{
public:
	CGameRenderer();
	~CGameRenderer();

	void SetOpenFilePath(const wchar_t* path);
	void DoOpenFile();
	void MarkShouldOpenFile() { should_open_file = true; }
	void MarkCloseFile(bool delete_after_close) {
		should_close_file = true; 
		this->delete_after_close = delete_after_close;
	}
	void NotifyAboutDialogClosed() { about_dialog_showed = false; }
	void NotifyHelpDialogClosed() { help_dialog_showed = false; }
	void AddTextureToQueue(CCTexture* tex, int x, int y, int id);
	CCGUInfo* GetGUInfo() { return uiInfo; }

private:

	LoggerInternal* logger;

	std::wstring currentOpenFilePath;
	bool fileOpened = false;

	bool Init() override;
	void Render(float FrameTime) override;
	void Update() override;
	void RenderUI() override;
	void Resize(int Width, int Height) override;
	void Destroy() override;

	//全景模式
	PanoramaMode mode = PanoramaMode::PanoramaSphere;
	CCPanoramaCamera*camera = nullptr;
	CCPanoramaRenderer* renderer = nullptr;
	CCFileManager*fileManager = nullptr;
	CAppUIWapper*uiWapper = nullptr;
	CCTextureLoadQueue*texLoadQueue = nullptr;

	char* GetCurrentPanoramaModeStr();

	//UI控制

	CCGUInfo* uiInfo = nullptr;

	bool debug_tool_active = false;
	bool show_console = false;
	bool show_status_bar = true;
	bool show_fps = true;
	int debug_tool_active_tab = 1;
	bool main_menu_active = true;
	float ui_update_tick = 0.0f;


	bool about_dialog_showed = false;
	bool help_dialog_showed = false;
	bool about_dialog_active = false;
	bool glinfo_dialog_active = false;
	bool render_dialog_active = false;
	bool loading_dialog_active = false;
	bool welecome_dialog_active = true;
	bool reg_dialog_showed = false;
	bool file_opened = false;

	float current_fps = 0;
	DWORD current_draw_time = 0;

	int zoom_slider_value = 50;

	bool should_open_file = false;
	bool should_close_file = false;
	bool delete_after_close = false;
	bool destroying = false;
	bool needTestImageAndSplit = false;
	bool firstMouse = true;
	float lastX = 0, lastY = 0, xoffset = 0, yoffset = 0;
	float loopCount = 0;

	void LoadImageInfo();
	void TestSplitImageAndLoadTexture();

	TextureLoadQueueDataResult* LoadChunkTexCallback(TextureLoadQueueInfo* info, CCTexture* texture);
	static TextureLoadQueueDataResult* LoadTexCallback(TextureLoadQueueInfo* info, CCTexture* texture, void* data);
	static void FileCloseCallback(void* data);
	static void CameraFOVChanged(void* data, float fov);
	static void CameraRotate(void* data, CCPanoramaCamera* cam);
	static void BeforeQuitCallback(COpenGLView* view);

	void SwitchMode(PanoramaMode mode);
	void UpdateConsoleState();

	void LoadAndChechRegister();

	float MouseSensitivity = 0.1f;
	float RoateSpeed = 20.0f;

	bool SplitFullImage = true;

	static void MouseCallback(COpenGLView* view, float x, float y, int button, int type);
	static void ScrollCallback(COpenGLView* view, float x, float y, int button, int type);
	void KeyMoveCallback(CCameraMovement move);

	SettingHlp* settings = nullptr;

	void LoadSettings();
	void SaveSettings();

};

