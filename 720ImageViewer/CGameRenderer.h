#pragma once
#include "stdafx.h"
#include "COpenGLRenderer.h"
#include "CCamera.h"
#include "CCPanoramaCamera.h"
#include "CCPanoramaRenderer.h"
#include "CCTextureLoadQueue.h"
#include "CCFileManager.h"
#include "CAppUIWapper.h"
#include "CApp.h"
#include "CCModel.h"
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
	void TestSplitImageAndLoadTexture();
	void NotifyAboutDialogClosed() { about_dialog_showed = false; }
	void NotifyHelpDialogClosed() { help_dialog_showed = false; }

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

	//UI控制

	bool debug_tool_active = false;
	bool show_console = false;
	int debug_tool_active_tab = 1;
	bool main_menu_active = true;

	bool about_dialog_showed = false;
	bool help_dialog_showed = false;
	bool about_dialog_active = false;
	bool glinfo_dialog_active = false;
	bool render_dialog_active = false;
	bool loading_dialog_active = false;

	bool firstMouse = true;
	float lastX = 0, lastY = 0, xoffset = 0, yoffset = 0;

	static TextureLoadQueueDataResult* LoadTexCallback(TextureLoadQueueInfo* info, CCTexture* texture, void* data);
	static void FileCloseCallback(void* data);
	static void CameraFOVChanged(void* data, float fov);
	static void BeforeQuitCallback(COpenGLView* view);

	void SwitchMode(PanoramaMode mode);
	void UpdateConsoleState();

	float MouseSensitivity = 0.1f;
	float RoateSpeed = 20.0f;

	static void MouseCallback(COpenGLView* view, float x, float y, int button, int type);
	static void ScrollCallback(COpenGLView* view, float x, float y, int button, int type);
	void KeyMoveCallback(CCameraMovement move);

	SettingHlp* settings = nullptr;

	void LoadSettings();
	void SaveSettings();

};

