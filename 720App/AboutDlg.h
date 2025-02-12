// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "stdafx.h"

class CAboutDlg : public SHostDialog
{
public:
	CAboutDlg();
	~CAboutDlg();
	
private:

	BOOL OnInitDialog(HWND hWnd, LPARAM lParam);

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnCheckAgreement();

private:

	SWindow* pWndArgeement = nullptr;
	SWindow* pWndMain = nullptr;
	SButton* pBtnArgeemen = nullptr;

protected:

	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		EVENT_NAME_COMMAND(L"btn_check_agreement", OnCheckAgreement)
		
	EVENT_MAP_END()

	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CAboutDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
};
