#include "stdafx.h"
#include "TipWnd.h"

#define TIMER_END_TIP  2031

CTipWnd::CTipWnd() :SHostWnd(L"LAYOUT:XML_TIPWND")
{
}
CTipWnd::~CTipWnd()
{
}

BOOL CTipWnd::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) | WS_EX_TRANSPARENT);
	return TRUE;
}

void CTipWnd::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
void CTipWnd::SetTip(const SStringT& strTip)
{
	if(pText == nullptr)
		pText = FindChildByName(L"txt_tip");
	pText->SetWindowText(strTip);
}
void CTipWnd::Show(int x, int y)
{
	SetWindowPos(HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}
void CTipWnd::Show() { ShowWindow(SW_SHOW); }
void CTipWnd::Hide() { ShowWindow(SW_HIDE); }

void CTipWnd::OnTimer(UINT_PTR timerID)
{
	SetMsgHandled(FALSE);
	if (timerID == TIMER_END_TIP)
	{
		SNativeWnd::DestroyWindow();
	}
}
