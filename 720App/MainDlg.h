// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "stdafx.h"
#include <shellapi.h>
#include <Shlwapi.h>
#include <helper/SMenuEx.h>
#include "ShadowWindow.h"
#include "TipWnd.h"
#include "PanoModeWnd.h"
#include "NotifyWnd.h"
#include "AboutDlg.h"
#include "HelpDlg.h"
#include "FeedbackDlg.h"
#include "SettingsDlg.h"
#include "SImageButtonHover.h"
#include "CMainAlbumAdapter.h"
#include "../720Core/720CoreExport.h"
#include "../720Core/CApp.h"
#include "../720Core/messages.h"
#include "../720Core/CWindowsGameRenderer.h"
#include "../720Core/CWindowsOpenGLView.h"
#include "../720Core/SystemHelper.h"

class CMainDlg : public SHostWnd, SOUI::IMessageFilter
{
public:
	CMainDlg();
	~CMainDlg();
	
	void SetView(CWindowsOpenGLView* view);
	void SetGameRenderer(CWindowsGameRenderer* gameRenderer);

	void OnPanoModeChanged(PanoramaMode mode);

	static CMainDlg* Instance;

	BOOL PreTranslateMessage(MSG* pMsg);
private:

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnMainMenu();
	void OnRightMenu();
	void OnListRightMenu();

	void OnSize(UINT nType, CSize size);
	void OnMove(CPoint pos);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
	void OnDestroy();
	void OnShow(BOOL firstShow, int s);
	void OnTimer(UINT_PTR id);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnRenderKey(UINT msg, WPARAM wParam, LPARAM lParam);

	void OnKillFocus(HWND hWnd);
	void OnSetFocus(HWND hWnd);
	void OnDropFile(HDROP hDropInfo);
	void OnCustomMessage(UINT uiMsg);

	void ResizeView(int w, int h);
	void ResizeShadow();
	void ResizeNotifyWnd();
	void UpdateFullScreenState();
	void UpdatePanoButtonState();
	void UpdateToolButtonState();
	void UpdateLoadingState();
	void UpdateLeftImageInfoState();
	void UpdateRightAlbumInfoState();
	void UpdateRightAlbumEmptyViewState();
	void ShowBigTip(SStringW str, int showTime = 2500);
	void DoLoadImageInfo(int id);
	void ResetImageInfo();
	void DoOpenFile(LPCWSTR path);

	void OnLoadSeetings();
	void OnApplySeetings();
	void OnSaveSeetings();

	void OnOpenFile();
	void OnCloseFile();
	void OnImportFile();
	void OnSettings();
	void OnAbout();
	void OnHelp();
	void OnFeedBack();
	void OnFullScreen();
	void OnOpenWith();
	void OnSnapshot();
	void OnZoomIn();
	void OnZoomOut();
	void OnZoomReset();
	void OnDelete();
	void OnShowImageDir();
	void OnShowImageInfo();
	void OnShowImageFileInfo();
	void OnShowAlbum();
	void OnAlbumCheckMode();
	void OnAlbumExitCheckMode();
	void OnAlbumCheckAll();
	void OnAlbumDeleteSelection();
	void OnAlbumDeleteCurrRight();
	void OnAlbumClearNonExistItems();
	void OnAlbumShowImageDirCurrRight();
	bool OnAlbumListSelectionItemChanged(EventArgs* e);
	bool OnAlbumListClick(EventArgs* e);
	bool OnAlbumListContextMenu(EventArgs* e);
	void OnChangePanoMode();
	void OnNext();
	void OnPrev();

	bool IsInAlbumList(const wchar_t* path, CMainAlbumItem** outItem);
	bool AlbumListAdd(const wchar_t* path, CMainAlbumItem**outItem = nullptr);
	static void AlbumCheckItemChanged(void*data, int count);
	void LoadAlbumList();
	void SaveAlbumList();
	void ClearAlbumList();

	void OnGameFileStatusChanged(bool isOpen, int status);

	static void OnGameSampleEventCallback(void* data, int eventCode, void* param);
	static void OnGameFileStatusChangedCallback(void* data, bool isOpen, int status);

	void OnThreadMessage(int handlerId);
	void OnToolButtonMouse(SImageButtonHover*ctl, bool enter);

public:
	CWindowsGameRenderer* GetGameRenderer() { return gameRenderer; }

protected:
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		EVENT_NAME_COMMAND(L"btn_import", OnImportFile)
		EVENT_NAME_COMMAND(L"btn_menu", OnMainMenu)
		EVENT_NAME_COMMAND(L"btn_fullscreen", OnFullScreen)
		EVENT_NAME_COMMAND(L"btn_fullscreen_menu", OnFullScreen)
		EVENT_NAME_COMMAND(L"btn_open_main", OnOpenFile)
		EVENT_NAME_COMMAND(L"btn_open_with", OnOpenWith)
		EVENT_NAME_COMMAND(L"btn_zoom_in", OnZoomIn)
		EVENT_NAME_COMMAND(L"btn_zoom_out", OnZoomOut)
		EVENT_NAME_COMMAND(L"btn_info", OnShowImageInfo)
		EVENT_NAME_COMMAND(L"btn_delete", OnDelete)
		EVENT_NAME_COMMAND(L"btn_full_screen", OnFullScreen)
		EVENT_NAME_COMMAND(L"btn_scale_100", OnZoomReset)
		EVENT_NAME_COMMAND(L"btn_pano_mode", OnChangePanoMode)
		EVENT_NAME_COMMAND(L"btn_album_big", OnShowAlbum)
		EVENT_NAME_COMMAND(L"btn_album", OnShowAlbum)
		EVENT_NAME_COMMAND(L"btn_close_album", OnShowAlbum)
		EVENT_NAME_COMMAND(L"btn_left_img", OnPrev)
		EVENT_NAME_COMMAND(L"btn_right_img", OnNext)
		EVENT_NAME_COMMAND(L"link_show_file_dir", OnShowImageDir)
		EVENT_NAME_COMMAND(L"link_show_file_info", OnShowImageFileInfo)
		EVENT_NAME_COMMAND(L"btn_info_close", OnShowImageInfo)
		EVENT_NAME_COMMAND(L"btn_album_check_mode", OnAlbumCheckMode)
		EVENT_NAME_COMMAND(L"btn_album_check_cancel", OnAlbumExitCheckMode)
		EVENT_NAME_COMMAND(L"btn_album_check_all", OnAlbumCheckAll)
		EVENT_NAME_COMMAND(L"btn_album_check_delete", OnAlbumDeleteSelection)
		EVENT_NAME_COMMAND(L"link_delete_selection", OnAlbumDeleteSelection)
	EVENT_MAP_END()

	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SHOWWINDOW(OnShow)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_DROPFILES(OnDropFile)
		MSG_WM_TIMER(OnTimer)
		if (uMsg == WM_KEYDOWN || uMsg == WM_KEYUP)
			OnRenderKey(uMsg, wParam, lParam);
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_KEYUP(OnKeyUp)
		MSG_WM_SIZE(OnSize)
		MSG_WM_MOVE(OnMove)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_SETFOCUS(OnSetFocus)
		if (uMsg == MESSAGE_CUSTOM_IMAGEBUTTON_MOUSE_ENTER)
			OnToolButtonMouse((SImageButtonHover *)wParam, (bool)lParam);
		else if (uMsg == CCTHREAD_MSG_HANDLER_DEF_MESSAGE)
			OnThreadMessage((int)wParam);
		else if (uMsg >= WM_CUSTOM_MIN && uMsg <= WM_CUSTOM_MAX)
			OnCustomMessage(uMsg);
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

	

private:


	const int shadowRadius = 28;
	const int titleBarHeight = 36;
	const int toolBarHeight = 54;
	const int leftWndWidth = 290;
	const int rightWndWidth = 325;

	CWindowsOpenGLView* view = nullptr;
	CWindowsGameRenderer* gameRenderer = nullptr;

	glm::vec2 lastSize = glm::vec2();

	bool ismaxsize = false;
	bool fullscreen = false;
	bool tooltipShow = false;
	bool isFirstShow = true;
	bool shouldShowMax = false;
	bool isShowToolsForFocus = false;
	bool isShowImageInfo = false;
	bool isShowAightAlbum = false;
	bool isFileOpened = false;
	bool isLoading = false;

	SImageButtonHover* toolTipCurrent = nullptr;

	CTipWnd* tipWind = nullptr;
	CPanoModeWnd* panoModeWnd = nullptr;
	CShadowWindow* shadowWindow = nullptr;
	CNotifyWnd* notifyWnd = nullptr;

	SWindow* pWindowError = nullptr;
	SWindow* pWindowWelecome = nullptr;
	SWindow* pWindowMain = nullptr;
	SWindow* pWindowImageInfo = nullptr;
	SWindow* pWindowAlbum = nullptr;
	SWindow* pWindowAlbumEmpty = nullptr;
	SWindow* pWndAlbumTitle = nullptr;
	SWindow* pWndAlbumTitleSelect = nullptr;
	SWindow* pWndCon = nullptr;

	SListView* pLstViewMain = nullptr;
	CMainAlbumAdapter* mainAdapter = nullptr;
	std::vector<CMainAlbumItem*> mainAlbumItems;
	CMainAlbumItem* currentItem = nullptr;
	CMainAlbumItem* currentRightItem = nullptr;
	bool currentItemChanged = false;

	SStatic* pTextFileError = nullptr;
	SStatic* pTextTitle = nullptr;
	SStatic* pTextImageInfoFullPath = nullptr;
	SStatic* pTextImageInfoFileSize = nullptr;
	SStatic* pTextImageInfoResolution = nullptr;
	SStatic* pTextImageInfoShootingDate = nullptr;
	SStatic* pTextImageInfoModifyDate = nullptr;
	SStatic* pTextImageInfoShutterTime = nullptr;
	SStatic* pTextImageInfoExposureBiasValue = nullptr;
	SStatic* pTextImageInfoIsoSensitivity = nullptr;
	SStatic* pTextImageInfoCamera = nullptr;
	SStatic* pTextImageInfoFocalLength = nullptr;
	SStatic* pTextAlbumSelectTitle = nullptr;

	SImageButton* pBtnFullscreenMenu = nullptr;	
	SImageButton* pBtnAlbumCheckAll = nullptr;
	SImageButtonHover* pBtnPanoMode = nullptr;
	SImageButtonHover* pBtnLeftImg = nullptr;
	SImageButtonHover* pBtnRightImg = nullptr;
	SImageButtonHover* pBtnInfo = nullptr;
	SImageButtonHover* pBtnOpenWith = nullptr;
	SImageButtonHover* pBtnDelete = nullptr;

	SStringW strTitle;
	SStringW strTitleLoading = L"加载中";

	CCThreadMessageHandler* uiThreadMessageHandler = nullptr;
	
public:
	//配置
	bool CheckImageRatio = true;
	bool ReverseRotation = false;
};
