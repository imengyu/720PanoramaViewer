#include "stdafx.h"
#include "SImageButtonHover.h"

SImageButtonHover::SImageButtonHover()
{
}
SImageButtonHover::~SImageButtonHover()
{
}

void SImageButtonHover::SetHoverCheckReceiver(HWND hwnd)
{
	receiveTarget = hwnd;
}
SStringW& SImageButtonHover::GetMyToolTip()
{
	return m_strMyTip;
}
void SImageButtonHover::OnMouseHover(WPARAM wParam, CPoint pos)
{
	if (!mouseEnter) {
		mouseEnter = true;
		SendMessage(receiveTarget, MESSAGE_CUSTOM_IMAGEBUTTON_MOUSE_ENTER, (WPARAM)this, (LPARAM)true);
	}
	SWindow::OnMouseHover(wParam, pos);
}
void SImageButtonHover::OnMouseLeave()
{
	mouseEnter = false;
	SendMessage(receiveTarget, MESSAGE_CUSTOM_IMAGEBUTTON_MOUSE_ENTER, (WPARAM)this, (LPARAM)false);
	SWindow::OnMouseLeave();
}
void SImageButtonHover::OnMouseLUp(UINT nFlags, CPoint pos)
{
	SendMessage(receiveTarget, MESSAGE_CUSTOM_IMAGEBUTTON_MOUSE_ENTER, (WPARAM)this, (LPARAM)false);
	SWindow::OnLButtonUp(nFlags, pos);
}
