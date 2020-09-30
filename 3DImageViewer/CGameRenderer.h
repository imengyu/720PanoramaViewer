#pragma once
#include "stdafx.h"
#include "COpenGLRenderer.h"
#include "CCamera.h"
#include "CApp.h"
#include <vector>

enum PanoramaMode {
	Sphere,
	Flat,
	Asteroid,
	OuterBall,
	Cylinder,
};
class CImageLoader;
class CGameRenderer : public COpenGLRenderer
{
public:
	CGameRenderer();
	~CGameRenderer();

	void SetOpenFilePath(const wchar_t* path);
	void DoOpenFile();
	void NotifyAboutDialogClosed() { about_dialog_showed = false; }
	void NotifyHelpDialogClosed() { help_dialog_showed = false; }

private:

	LoggerInternal* logger;

	std::wstring currentOpenFilePath;
	bool fileOpened = false;


	bool Init() override;
	void ResetCamera();
	void HandleFlatZoom(float yoffset);
	void Render(float FrameTime) override;
	void Update() override;
	void RenderUI() override;
	void Resize(int Width, int Height) override;
	void Destroy() override;

	void renderPanel(bool wireframe);
	void renderSphere(bool wireframe);
	void renderABox();

	glm::vec2 getUVPoint(float u, float v, short i);
	glm::vec3 getPoint(float u, float v);
	glm::vec2 getMercatorPoint(float u, float v);

	void fasterSwitchTextureToChunk(int i, int j);

	GLuint texture_one = 0;
	GLuint texture_logo = 0;
	GLuint texture_chunks[64][64];

	int currentLoadTextureChunkCount = -1;
	int currentLoadTextureChunkIndexX = -1;
	int currentLoadTextureChunkIndexY = -1;
	bool currentLoadTextureReqNext = false;
	bool currentLoadTextureReqLoadToGL = false;

	GLuint shaderProgram = 0;

	glm::vec2 texture_chunk_size;
	glm::vec2 texture_logo_size;

	//131072


	//65536

	GLint viewLoc = -1;
	GLint projectionLoc = -1;
	GLint modelLoc = -1;
	GLint ourTextrueLoc = -1;
	GLint ourColorLoc = -1;
	GLint useColorLoc = -1;

	bool debug_tool_active = false;
	bool show_console = false;
	int debug_tool_active_tab = 1;
	bool main_menu_active = true;
	char* message_dialog_message = nullptr;

	bool about_dialog_showed = false;
	bool help_dialog_showed = false;
	bool about_dialog_active = false;
	bool glinfo_dialog_active = false;
	bool render_dialog_active = false;
	bool message_dialog_active = false;
	bool loading_dialog_active = false;


	bool PolygonMode = true;
	bool DrawVector = true;
	bool CameraPerspective = true;

	float CameraPerspectiveBottom = -1.0f;
	float CameraPerspectiveTop = 1.0f;

	bool firstMouse = true;
	float lastX = 0, lastY = 0, xoffset = 0, yoffset = 0;

	bool ReqFileLoadTicked = false;
	bool ReqTexLoadTicked = false;
	bool ReqLogoTexLoad = true;

	void resetModel();

	bool createShader();
	void createPanoramaTexture();
	void createPanoramaTextureChunk();
	void loadPanoramaTextureChunk();
	void loadPanoramaTexture();
	void destroyPanoramaTextures();

	void loadLogoTexture();

	void showMessageDialog(const WCHAR* str);
	void showMessageDialog(const char* str);

	void openFile();
	void closeFile();
	void destroyFileLoader();
	void doOpenFile();

	int preferImageXSeg = 50;
	int preferImageYSeg = 30;
	int preferImageChunkSize = 2048;

	bool  currentFileLoading = false;
	CImageLoader* currentFileLoader = nullptr;
	std::wstring currentFilePath;
	bool currentImageUseChunkLoad = false;
	int currentImageXSeg = 50;
	int currentImageYSeg = 30;

	int currentImageDepth = 0;
	BYTE* currentImageData = nullptr;
	BYTE* currentImageChunkData = nullptr;
	size_t currentImageChunkDataSize = 0;
	int currentImageChunkOnceLoad = 0;
	int currentImageW = 0;
	int currentImageH = 0;

	void finishImageChunkLoad();
	BYTE* getImageChunkData(int x, int y, int chunkW, int chunkH);

	PanoramaMode mode = PanoramaMode::Sphere;

	glm::vec3 Positon = glm::vec3(0.0f);
	glm::vec3 Rotation = glm::vec3(0.0f);
	glm::vec3 Front = glm::vec3(0.0f);
	glm::vec3 Right = glm::vec3(0.0f);
	glm::vec3 Up = glm::vec3(0.0f);
	glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec2 UVOffest = glm::vec2(0.0f);
	float FlatZoom = 1.0f;
	float FlatZoomMin = 1.0f;
	float FlatZoomMax = 4.0f;
	float FlatZoomSpeed = 0.01f;

	void UpdateModelVectors();
	void UpdateConsoleState();
	void SwitchMode(PanoramaMode mode);

	float MouseSensitivity = 0.1f;
	float RoateSpeed = 20.0f;

	static void MouseCallback(COpenGLView* view, float x, float y, int button, int type);
	static void ScrollCallback(COpenGLView* view, float x, float y, int button, int type);

	void KeyMoveCallback(Camera_Movement move);

	SettingHlp* settings = nullptr;

	void LoadSettings();
	void SaveSettings();

	GLubyte* glVendor = nullptr;
	GLubyte* glRenderer = nullptr;
	GLubyte* glVersion = nullptr;
	GLubyte* glslVersion = nullptr;

	BYTE* logoImageData = nullptr;
};

