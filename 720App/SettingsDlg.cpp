// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SettingsDlg.h"	


CSettingsDlg::CSettingsDlg() : SHostDialog(_T("LAYOUT:XML_SETWND"))
{
}
CSettingsDlg::~CSettingsDlg()
{

}

BOOL CSettingsDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	return TRUE;
}
void CSettingsDlg::OnClose()
{
	OnCancel();
}
