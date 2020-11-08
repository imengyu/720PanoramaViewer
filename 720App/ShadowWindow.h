#pragma once
#include "stdafx.h"

class CShadowWindow : public SHostWnd
{
public:
	CShadowWindow();
	~CShadowWindow();


	BOOL OnInitDialog(HWND hWnd, LPARAM lParam);

protected:

	virtual void OnFinalMessage(HWND hWnd);

	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CShadowWindow)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
};

