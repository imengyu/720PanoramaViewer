#include "stdafx.h"
#include "PanoModeWnd.h"
#include "MainDlg.h"

#define TIMER_END_TIP  2032

CPanoModeWnd::CPanoModeWnd(CMainDlg* mainDlg) :SHostWnd(L"LAYOUT:XML_PANO_MODEWND")
{
	this->mainDialog = mainDlg;
}
CPanoModeWnd::~CPanoModeWnd()
{
}

BOOL CPanoModeWnd::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) | WS_EX_TRANSPARENT);
	return TRUE;
}

void CPanoModeWnd::OnLostFocus(HWND hWndFocus)
{
	SetTimer(TIMER_END_TIP, 300);
}
void CPanoModeWnd::OnModeCylinder()
{
	mainDialog->OnPanoModeChanged(PanoramaCylinder);
	Hide();
}
void CPanoModeWnd::OnModeSphere()
{
	mainDialog->OnPanoModeChanged(PanoramaSphere);
	Hide();
}
void CPanoModeWnd::OnModeAsteroid()
{
	mainDialog->OnPanoModeChanged(PanoramaAsteroid);
	Hide();
}
void CPanoModeWnd::OnModeOutball()
{
	mainDialog->OnPanoModeChanged(PanoramaOuterBall);
	Hide();
}
void CPanoModeWnd::OnMode360()
{
	mainDialog->OnPanoModeChanged(PanoramaFull360);
	Hide(); 
}
void CPanoModeWnd::OnModeOrginal()
{
	mainDialog->OnPanoModeChanged(PanoramaFullOrginal);
	Hide();
}
void CPanoModeWnd::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
void CPanoModeWnd::OnTimer(UINT_PTR timerID)
{
	SetMsgHandled(FALSE);
	if (timerID == TIMER_END_TIP) {
		Hide();
		KillTimer(TIMER_END_TIP);
	}
}

void CPanoModeWnd::Show(int x, int y)
{
	SetWindowPos(HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	ShowWindow(SW_SHOW);
}
void CPanoModeWnd::Hide() { ShowWindow(SW_HIDE); }
