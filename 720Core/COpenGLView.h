#pragma once
#include "COpenGLRenderer.h"

typedef void(*ViewMouseCallback)(COpenGLView* view, float xpos, float ypos, int button, int type);
typedef void(*BeforeQuitCallback)(COpenGLView* view);

const int MAX_KEY_LIST = 8;

enum ViewMouseEventType {
	ViewMouseMouseDown,
	ViewMouseMouseUp,
	ViewMouseMouseMove,
	ViewMouseMouseWhell,
};

class CCamera;
class CCShader;
class CCRenderGlobal;
//OpenGL 视图抽象类
class COpenGLView
{
public:
	COpenGLView(COpenGLRenderer* renderer) {
		OpenGLRenderer = renderer;
		OpenGLRenderer->View = this;
		memset(DownedKeys, 0, sizeof(DownedKeys));
		memset(UpedKeys, 0, sizeof(UpedKeys));
	}
	virtual ~COpenGLView() {

	}

	//视图高度
	int Width = 800;
	//视图宽度
	int Height = 600;

	/**
	 * 初始化
	 * @return 返回是否成功
	 */
	virtual bool Init() { return false; }
	/**
	 * 释放
	 */
	virtual void Destroy() {}

	//获取或设置窗口是否全屏
	bool IsFullScreen = false;

	/**
	 * 更新窗口是否全屏的状态
	 */
	virtual void UpdateFullScreenState() {}
	/**
	 * 设置窗口是否全屏
	 * @param full 是否全屏
	 */
	virtual void SetFullScreen(bool full) {}
	/**
	 * 获取窗口是否全屏
	 * @return
	 */
	virtual bool GetIsFullScreen() { return IsFullScreen; }

	/**
	 * 显示窗口
	 * @param Maximized 是否最大化
	 */
	virtual void Show(bool Maximized = false) {}
	//激活窗口
	virtual void Active() {}
	//运行窗口消息循环（仅Windows）
	virtual void MessageLoop() {}
	/**
	 * 调整窗口大小
	 * @param w 宽度
	 * @param h 高度
	 * @param moveToCenter 是否移动到屏幕中央
	 */
	virtual void Resize(int w, int h, bool moveToCenter) {}

	//关闭窗口
	virtual void CloseView() {}
	//等待渲染线程退出
	virtual void WaitDestroyComplete() {}

	//设置为低FPS模式
	virtual void SetToLowerFpsMode() {}
	//退出为低FPS模式
	virtual void QuitLowerFpsMode() {}

	//开始窗口鼠标捕捉
	virtual void MouseCapture() {}
	//释放窗口鼠标捕捉
	virtual void ReleaseCapture() {}

	/**
	 * 设置当前窗口的文字
	 * @param text 文字
	 */
	virtual void SetViewText(const wchar_t* text) {}

	/**
	 * 设置当窗口关闭前回调
	 */
	void SetBeforeQuitCallback(BeforeQuitCallback beforeQuitCallback) { this->beforeQuitCallback = beforeQuitCallback; }
	/**
	 * 设置鼠标事件回调
	 */
	void SetMouseCallback(ViewMouseCallback mouseCallback) { this->mouseCallback = mouseCallback; }
	/**
	 * 设置鼠标滚动事件回调
	 */
	void SetScrollCallback(ViewMouseCallback scrollCallback) { this->scrollCallback = scrollCallback; }

	/**
	 * 发送Windows消息至窗口
	 */
	virtual LRESULT SendWindowsMessage(UINT Msg, WPARAM wParam, LPARAM lParam) { return 0; }

	//摄像机
	//**********************

	/**
	 * 计算当前主摄像机的矩阵映射
	 * @param shader 使用的程序
	 */
	void CalcMainCameraProjection(CCShader* shader) const {
		CalcCameraProjection(Camera, shader);
	}
	/**
	 * 计算无摄像机时的矩阵映射
	 * @param shader 使用的程序
	 */
	void CalcNoMainCameraProjection(CCShader* shader) const {
		CalcCameraProjection(Camera, shader);
	}
	/**
	 * 计算当前主摄像机的矩阵映射
	 * @param shader 使用的程序
	 */
	static void CalcCameraProjection(CCamera* camera, CCShader* shader);
	/**
	* 计算无摄像机时的矩阵映射
	* @param shader 使用的程序
	*/
	static void CalcNoCameraProjection(CCamera* camera, CCShader* shader);

	//当前主摄像机
	CCamera* Camera = nullptr;

	/**
	 * 设置当前主摄像机
	 * @param camera 摄像机
	 */
	virtual void SetCamera(CCamera* camera) {
		SetCamera(this, camera);
	}

	//时间
	//**********************

	/**
	 * 获取当前程序绘制总时间
	 * @return
	 */
	virtual float GetTime() { return 0; }
	/**
	 * 获取当前FPS
	 * @return
	 */
	virtual float GetCurrentFps() { return 0; }
	/**
	 * 获取绘制时间
	 * @return
	 */
	virtual float GetDrawTime() { return 0; }
	/**
	 * 获取增量时间
	 * @return
	 */
	virtual float GetDeltaTime() { return 0; }

	//按键
	//**********************

	/**
	 * 获取是否有键按下
	 * @param code 按键键值
	 * @return
	 */
	virtual bool GetKeyPress(int code) {
		return IsKeyInKeyListExists(DownedKeys, code) > -1;
	}
	/**
	 * 获取是否有键正在按下
	 * @param code 按键键值
	 * @return
	 */
	virtual bool GetKeyDown(int code) {
		int up = IsKeyInKeyListExists(DownedKeys, code);
		if (up > -1) {
			DownedKeys[up] = 0;
			return true;
		}
		return  false;
	}
	/**
	 * 获取是否有键放开
	 * @param code 按键键值
	 * @return
	 */
	virtual bool GetKeyUp(int code) {
		int up = IsKeyInKeyListExists(UpedKeys, code);
		if (up > -1) {
			UpedKeys[up] = 0;
			return true;
		}
		return  false;
	}

	/**
	 * 获取当前渲染器
	 * @return
	 */
	COpenGLRenderer* GetRenderer() {
		return OpenGLRenderer;
	}

protected:

	COpenGLRenderer* OpenGLRenderer = NULL;

	ViewMouseCallback scrollCallback = nullptr;
	ViewMouseCallback mouseCallback = nullptr;
	BeforeQuitCallback beforeQuitCallback = nullptr;

	//按键处理
	//*******************************

	int DownedKeys[MAX_KEY_LIST];
	int UpedKeys[MAX_KEY_LIST];

	int AddKeyInKeyList(int* list, int code) {
		for (int i = 0; i < MAX_KEY_LIST; i++) {
			if (list[i] == 0)
			{
				list[i] = code;
				return i;
			}
		}
		return -1;
	}
	int IsKeyInKeyListExists(int* list, int code) {
		for (int i = 0; i < MAX_KEY_LIST; i++) {
			if (list[i] == code)
				return i;
		}
		return -1;
	}
	void HandleDownKey(int code) {

		int upIndex = IsKeyInKeyListExists(UpedKeys, code);
		if (upIndex > -1) UpedKeys[upIndex] = 0;

		int downIndex = IsKeyInKeyListExists(DownedKeys, code);
		if (downIndex == -1) AddKeyInKeyList(DownedKeys, code);

	}
	void HandleUpKey(int code) {
		int upIndex = IsKeyInKeyListExists(UpedKeys, code);
		if (upIndex == -1) AddKeyInKeyList(UpedKeys, code);

		int downIndex = IsKeyInKeyListExists(DownedKeys, code);
		if (downIndex > -1) DownedKeys[downIndex] = 0;

	}

	static void SetCamera(COpenGLView* view, CCamera* camera);
};

