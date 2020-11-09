#include "stdafx.h"
#include "NotifyWnd.h"

#define TIMER_SHOW_TIP  2030
#define TIMER_END_TIP  2031
#define TIMER_FADE_OUT  2032

CNotifyWnd::CNotifyWnd() :SHostWnd(L"LAYOUT:XML_NOTIFYWND")
{
}
CNotifyWnd::~CNotifyWnd()
{
}

BOOL CNotifyWnd::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	pAniExit = SApplication::getSingletonPtr()->LoadAnimation(L"anim:alpha_out");
	pAniIn = SApplication::getSingletonPtr()->LoadAnimation(L"anim:zoom_in");
	SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) | WS_EX_TRANSPARENT);
	return TRUE;
}

void CNotifyWnd::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
void CNotifyWnd::SetTip(const SStringT& strTip, int showTime)
{
	this->showTime = showTime;
	if(pText == nullptr)
		pText = FindChildByName(L"txt_tip");
	pText->SetWindowText(strTip);
}
void CNotifyWnd::Show(int x, int y)
{
	SetWindowPos(HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	SetTimer(TIMER_SHOW_TIP, 20);

	StartAnimation(pAniIn);
}
void CNotifyWnd::Show() { 

	StartAnimation(pAniIn);
	SetTimer(TIMER_SHOW_TIP, 20);
}
void CNotifyWnd::Hide() { 
	StartAnimation(pAniExit);
	SetTimer(TIMER_FADE_OUT, 300);
}

void CNotifyWnd::OnTimer(UINT_PTR timerID)
{
	SetMsgHandled(FALSE);
	if (timerID == TIMER_END_TIP) {
		KillTimer(TIMER_END_TIP);
		Hide();
	}
	else if (timerID == TIMER_SHOW_TIP) {
		KillTimer(TIMER_SHOW_TIP);
		KillTimer(TIMER_END_TIP);
		SetTimer(TIMER_END_TIP, showTime);
		ShowWindow(SW_SHOW);
	}
	else if (timerID == TIMER_FADE_OUT) {
		KillTimer(TIMER_FADE_OUT);
		ShowWindow(SW_HIDE);
		StopHostAnimation();
	}
}
