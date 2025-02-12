// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "stdafx.h"

class CSettingsDlg : public SHostDialog
{
public:
	CSettingsDlg();
	~CSettingsDlg();
	
private:

	BOOL OnInitDialog(HWND hWnd, LPARAM lParam);

	void OnClose();

protected:

	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
	EVENT_MAP_END()

	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CSettingsDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

private:
	SCheckBox* pCheckImageRatio = nullptr;
	SCheckBox* pCheckReverseRotation = nullptr;
	SCheckBox* pCheckConsole = nullptr;
	SCheckBox* pCheckDebugtool = nullptr;
	SCheckBox* pCheckInfoov = nullptr;

};
