#include "stdafx.h"
#include "ShadowWindow.h"

CShadowWindow::CShadowWindow() : SHostWnd(L"LAYOUT:XML_SHADOWWND")
{
}
CShadowWindow::~CShadowWindow()
{
}

void CShadowWindow::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

BOOL CShadowWindow::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) | WS_EX_TRANSPARENT);
	return TRUE;
}