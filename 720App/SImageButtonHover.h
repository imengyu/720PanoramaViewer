#pragma once
#include "stdafx.h"

#define MESSAGE_CUSTOM_IMAGEBUTTON_MOUSE_ENTER 0X500A

class SImageButtonHover : public SImageButton
{
	SOUI_CLASS_NAME(SImageButtonHover, L"imgbtnhover")
public:
    SImageButtonHover();
    ~SImageButtonHover();

    void SetHoverCheckReceiver(HWND hwnd);
    SStringW& GetMyToolTip();
private:

    void OnMouseHover(WPARAM wParam, CPoint pos);
    void OnMouseLeave();
    void OnMouseLUp(UINT nFlag, CPoint pos);

    bool mouseEnter = false;
    HWND receiveTarget = nullptr;
    SStringW m_strMyTip;

protected:

    SOUI_ATTRS_BEGIN()
        ATTR_STRINGW(L"myTip", m_strMyTip, FALSE)
     SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_LBUTTONUP(OnMouseLUp)
        MSG_WM_RBUTTONUP(OnMouseLUp)
    SOUI_MSG_MAP_END()


};

