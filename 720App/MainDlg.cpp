// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	

#define TIMER_TIP_SHOW  2031
#define TIMER_TIP_CLOSE  2033

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
}
CMainDlg::~CMainDlg()
{

}
BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	shadowWindow = new CShadowWindow();
	shadowWindow->Create(m_hWnd);
	shadowWindow->SendMessage(WM_INITDIALOG);

	tipWind = new CTipWnd();
	tipWind->Create(m_hWnd);
	tipWind->SendMessage(WM_INITDIALOG);

	panoModeWnd = new CPanoModeWnd(this);
	panoModeWnd->Create(m_hWnd);
	panoModeWnd->SendMessage(WM_INITDIALOG);

	CApp* app = AppGetAppInstance();

	//MessageHandlers
	CCThreadMessageCenter* messageCenter = app->GetMessageCenter();
	uiThreadMessageHandler = messageCenter->CreateThreadMessageHandler(2);
	uiThreadMessageHandler->isUiThread = true;
	uiThreadMessageHandler->targetWindow = m_hWnd;
	messageCenter->RigisterThreadMessageHandler(2, uiThreadMessageHandler);

	//Settings
	SettingHlp* settings = app->GetSettings();
	fullscreen = settings->GetSettingBool(L"FullScreen", false);
	if (fullscreen) UpdateFullScreenState();
	else {
		bool max = settings->GetSettingBool(L"Maximize", false);
		if (!max) {
			int w = settings->GetSettingInt(L"Width", 1024);
			int h = settings->GetSettingInt(L"Height", 768);
			int cw = GetSystemMetrics(SM_CXSCREEN);
			int ch = GetSystemMetrics(SM_CYSCREEN);
			if (w <= 800) w = 1024;
			if (h <= 600) h = 768;

			SetWindowPos(0, (cw - w) / 2, (ch - h) / 2, w, h, SWP_NOACTIVATE | SWP_NOZORDER);
		}
		else OnMaximize();
	}

	ResizeShadow();

	//Eles
	pWindowWelecome = FindChildByID(1);
	pWindowMain = FindChildByID(2);
	pTextFileError = FindChildByName2<SStatic>("text_file_error");
	pBtnPanoMode = FindChildByName2<SImageButton>("btn_pano_mode");
	((SImageButtonHover*)pBtnPanoMode)->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_scale_100")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_scale_org")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_zoom_out")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_zoom_in")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_left_img")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_right_img")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_info")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_open_with")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_delete")->SetHoverCheckReceiver(m_hWnd);
	FindChildByName2<SImageButtonHover>("btn_album")->SetHoverCheckReceiver(m_hWnd);

	return TRUE;
}
void CMainDlg::OnShow(BOOL show, int s)
{
	if(shadowWindow)
		shadowWindow->ShowWindow(show ? SW_SHOW : SW_HIDE);
}
void CMainDlg::OnTimer(UINT_PTR id)
{
	switch (id)
	{
	case TIMER_TIP_SHOW:
		KillTimer(TIMER_TIP_SHOW);
		if (tooltipShow) {
			CRect rect = toolTipCurrent->GetWindowRect();
			ClientToScreen(rect);
			CRect rectTip = tipWind->GetWindowRect();
			tipWind->SetWindowPos(HWND_TOP,
				rect.left - (rectTip.Width() - rect.Width()) / 2,
				rect.top - rectTip.Height() + 10,
				0, 0, SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSIZE);
		}
		break;
	case TIMER_TIP_CLOSE:
		KillTimer(TIMER_TIP_CLOSE);
		if(!tooltipShow)
			tipWind->Hide();
		break;
	default:
		SHostWnd::OnTimer(id);
		break;
	}
}
void CMainDlg::OnClose()
{
	CRect rect = GetWindowRect();
	SettingHlp* settings = AppGetAppInstance()->GetSettings();
	settings->SetSettingBool(L"FullScreen", fullscreen);
	settings->SetSettingInt(L"Width", rect.Width());
	settings->SetSettingInt(L"Height", rect.Height());
	settings->SetSettingBool(L"Maximize", ismaxsize);

	if (view)
		view->SetEnabled(false);
	if (tipWind)
		tipWind->DestroyWindow();
	if (shadowWindow)
		shadowWindow->DestroyWindow();
	if (panoModeWnd)
		panoModeWnd->DestroyWindow();

	SNativeWnd::DestroyWindow();
}
void CMainDlg::OnDestroy()
{
	if (uiThreadMessageHandler) {
		AppGetAppInstance()->GetMessageCenter()->UnRigisterThreadMessageHandler(2);
		uiThreadMessageHandler = nullptr;
	}
	if (view) {
		view->Destroy();
		view = nullptr;
	}

	PostQuitMessage(0);
}
void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}
void CMainDlg::OnMainMenu()
{
	SWindow* pBtn = FindChildByName(L"btn_menu");
	CRect rect = pBtn->GetWindowRect();
	ClientToScreen(rect);

	SMenuEx menu;
	if (menu.LoadMenu(_T("SMenuEx:menu_main")))
	{
		int id = menu.TrackPopupMenu(TPM_RETURNCMD, rect.left - 7, rect.top + rect.Height() - 7, m_hWnd);
		switch (id)
		{
		case 1: OnOpenFile();  break;
		case 2: OnCloseFile(); break;
		case 3: OnImportFile(); break;
		case 4: OnOpenWith(); break;
		case 5: OnSnapshot(); break;
		case 6: OnSettings(); break;
		case 7: OnFeedBack(); break;
		case 8: OnAbout(); break;
		case 9: OnClose(); break;
		case 10: OnHelp(); break;
		case 11: OnShowImageInfo(); break;
		default: break;
		}
	}
}
void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	SWindow* pBtnMax = FindChildByName(L"btn_max");
	SWindow* pBtnRestore = FindChildByName(L"btn_restore");
	if (!pBtnMax || !pBtnRestore) return;

	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}

	ismaxsize = nType == SIZE_MAXIMIZED;

	//阴影窗口
	if (shadowWindow) {
		shadowWindow->ShowWindow(
			(nType != SIZE_MINIMIZED && nType != SIZE_MAXIMIZED && nType != SIZE_MAXHIDE) ? SW_SHOW : SW_HIDE
		);
		ResizeShadow();
	}

	//渲染窗口
	if (view) {
		ResizeView(size.cx, size.cy);
		::SendMessage(view->GetHWND(), WM_SIZE, nType, NULL);
	}
}
void CMainDlg::OnMove(CPoint pos)
{	
	//阴影窗口
	if (shadowWindow) {
		shadowWindow->SetWindowPos(0, pos.x - shadowRadius, pos.y - shadowRadius, 0,0, SWP_NOSIZE | 
			SWP_NOREDRAW | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void CMainDlg::OnKillFocus(HWND hWnd)
{
	if (view) view->SetToLowerFpsMode();
}
void CMainDlg::OnSetFocus(HWND hWnd)
{
	if (shadowWindow)
		::SetWindowPos(shadowWindow->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	if (view) 
		view->QuitLowerFpsMode();
}
void CMainDlg::OnDropFiles(HDROP hDropInfo)
{
	UINT  nFileCount = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	TCHAR szFileName[_MAX_PATH] = _T("");
	DWORD dwAttribute;

	if (nFileCount > 0) {
		::DragQueryFile(hDropInfo, 0, szFileName, sizeof(szFileName));
		dwAttribute = ::GetFileAttributes(szFileName);

		if ((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
		{
			gameRenderer->SetOpenFilePath(szFileName);
			gameRenderer->MarkShouldOpenFile();
		}
	}

	::DragFinish(hDropInfo);
}
void CMainDlg::OnCustomMessage(UINT uiMsg)
{
	switch (uiMsg)
	{
	case WM_CUSTOM_SHOW_ABOUTBOX: {
		//CAboutDialog::Show(m_hWnd);
		gameRenderer->NotifyAboutDialogClosed();
		break;
	}
	case WM_CUSTOM_SHOW_HELPBOX: {
		//CHelpDialog::Show(m_hWnd);
		gameRenderer->NotifyHelpDialogClosed();
		break;
	}
	case WM_CUSTOM_OPEN_FILE: {
		
		break;
	}
	case WM_CUSTOM_CMD_OPENFILE: {
		std::wstring path = AppGetAppInstance()->GetSettings()->GetSettingStr(L"NextOpenFilePath", L"", 512);
		if (_waccess_s(path.c_str(), 0) == 0) {
			gameRenderer->SetOpenFilePath(path.c_str());
			gameRenderer->MarkShouldOpenFile();
		}
		break;
	}
	case WM_CUSTOM_DEL_FILE: {

	}
	default:
		break;
	}
}
void CMainDlg::SetView(CWindowsOpenGLView* view) {
	this->view = view;
	CRect rect = GetClientRect();
	ResizeView(rect.Width(), rect.Height());
}
void CMainDlg::SetGameRenderer(CWindowsGameRenderer* gameRenderer) {
	this->gameRenderer = gameRenderer;
	gameRenderer->SetFileStatusChangedCallback(OnGameFileStatusChangedCallback, this);

	UpdatePanoButtonState();

}

void CMainDlg::ResizeView(int w, int h) {
	if (view) {
		if(fullscreen) ::MoveWindow(view->GetHWND(), 1, 1, w - 2, h - 2, FALSE);
		else ::MoveWindow(view->GetHWND(), 1, titleBarHeight, w - 2, h - toolBarHeight - titleBarHeight, FALSE);
	}
}
void CMainDlg::ResizeShadow() {
	CRect rect = GetWindowRect();
	shadowWindow->SetWindowPos(0,0,0, 
		rect.Width() + shadowRadius * 2, rect.Height() + shadowRadius * 2, 
		SWP_NOREDRAW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}
void CMainDlg::UpdateFullScreenState() {
	CRect rect = GetWindowRect();
	if (fullscreen) {
		lastSize = glm::vec2(rect.Width(), rect.Height());
		ShowWindow(SW_MAXIMIZE);
		rect = GetWindowRect();
		ResizeView(rect.Width(), rect.Height());
	}
	else {
		ShowWindow(SW_RESTORE);
		if (lastSize.x >= 500 && lastSize.y >= 400) {
			SetWindowPos(0, 0, 0, (int)lastSize.x, (int)lastSize.y, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
			CenterWindow();
			ResizeView((int)lastSize.x, (int)lastSize.y);
		}
	}

}
void CMainDlg::UpdatePanoButtonState() {
	switch (gameRenderer->GetMode())
	{
	case PanoramaMode::PanoramaCylinder:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_cylinder");
		break;
	case PanoramaMode::PanoramaAsteroid:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_asteroid");
		break;
	case PanoramaMode::PanoramaFull360:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_360");
		break;
	case PanoramaMode::PanoramaOuterBall:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_outball");
		break;
	case PanoramaMode::PanoramaSphere:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_sphere");
		break;
	default:
	case PanoramaMode::PanoramaFullOrginal:
		pBtnPanoMode->SetAttribute(L"skin", L"skin_btn_pano_orginal");
		break;
	}
}

void CMainDlg::OnOpenFile() {
	WCHAR strFilename[MAX_PATH] = { 0 };
	if (SystemHelper::ChooseOneFile(m_hWnd, NULL, L"请选择一个全景图", L"图片文件\0*.jpg;*.png;*.bmp\0所有文件(*.*)\0*.*\0\0\0",
		strFilename, NULL, strFilename, MAX_PATH)) {
		gameRenderer->SetOpenFilePath(strFilename);
		gameRenderer->MarkShouldOpenFile();
	}
}
void CMainDlg::OnCloseFile() { gameRenderer->MarkCloseFile(false); }
void CMainDlg::OnImportFile() {

}
void CMainDlg::OnFullScreen() {
	fullscreen = true;
	UpdateFullScreenState();
}
void CMainDlg::OnOpenWith() {
	gameRenderer->OpenFileAs();
}
void CMainDlg::OnSnapshot() {

}
void CMainDlg::OnZoomIn() { gameRenderer->ZoomIn(); }
void CMainDlg::OnZoomOut() { gameRenderer->ZoomOut(); }
void CMainDlg::OnZoomReset() { gameRenderer->ZoomReset(); }
void CMainDlg::OnZoomBest() { gameRenderer->ZoomBest(); }
void CMainDlg::OnDelete() {
	if (MessageBox(m_hWnd, L"你是否真的要删除该图像？\n删除后不能恢复！", L"警告", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
		gameRenderer->MarkCloseFile(true);
}
void CMainDlg::OnShowImageInfo() {

}
void CMainDlg::OnShowAlbum()
{

}
void CMainDlg::OnChangePanoMode()
{
	if (panoModeWnd->IsWindowVisible())
		panoModeWnd->Hide();
	else {
		CRect rect = pBtnPanoMode->GetWindowRect();
		ClientToScreen(rect);
		CRect rectTip = panoModeWnd->GetWindowRect();
		panoModeWnd->Show(
			rect.left - (rectTip.Width() - rect.Width()) / 2,
			rect.top - rectTip.Height() + 10
		);
	}
}
void CMainDlg::OnPanoModeChanged(PanoramaMode mode) {
	gameRenderer->SetMode(mode);
	UpdatePanoButtonState();
}
void CMainDlg::OnToolButtonMouse(SImageButtonHover* ctl, bool enter)
{
	if (enter) {
		tooltipShow = TRUE;
		toolTipCurrent = ctl;
		tipWind->SetTip(ctl->GetMyToolTip());
		SetTimer(TIMER_TIP_SHOW, 50);
	}
	else {
		tooltipShow = FALSE;
		SetTimer(TIMER_TIP_CLOSE, 100);
	}
}


void CMainDlg::OnSettings() {

}
void CMainDlg::OnAbout() {

}
void CMainDlg::OnHelp() {

}
void CMainDlg::OnFeedBack() {

}

void CMainDlg::OnGameFileStatusChangedCallback(void* data, bool isOpen, int status)
{
	auto* thisPtr = (CMainDlg*)data;
	thisPtr->pWindowWelecome->SetVisible(!isOpen, TRUE);
	thisPtr->pWindowMain->SetVisible(isOpen, TRUE);
	thisPtr->view->SetVisible(isOpen);

	if (!isOpen) {
		if (status == GAME_FILE_OPEN_FAILED) {
			thisPtr->pWindowWelecome->SetVisible(false);
			thisPtr->pWindowError->SetVisible(true);
			thisPtr->pTextFileError->SetWindowText(thisPtr->gameRenderer->GetFileLastError());
		}
	}
}

void CMainDlg::OnThreadMessage(int handlerId)
{
	if (handlerId == 2)
		uiThreadMessageHandler->RunNext();
}


