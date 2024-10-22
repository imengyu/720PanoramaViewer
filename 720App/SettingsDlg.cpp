// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SettingsDlg.h"	
#include "MainDlg.h"

CSettingsDlg::CSettingsDlg() : SHostDialog(_T("LAYOUT:XML_SETWND"))
{
	m_bMsgHandled = false;
}
CSettingsDlg::~CSettingsDlg()
{

}

BOOL CSettingsDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	pCheckImageRatio = FindChildByName2<SCheckBox>("check_check_image_ratio");
	pCheckReverseRotation = FindChildByName2<SCheckBox>("check_reverse_control");
	pCheckConsole = FindChildByName2<SCheckBox>("check_check_console");
	pCheckDebugtool = FindChildByName2<SCheckBox>("check_check_debugtool");
	pCheckInfoov = FindChildByName2<SCheckBox>("check_check_infoov");

	pCheckImageRatio->SetCheck(CMainDlg::Instance->CheckImageRatio);
	pCheckReverseRotation->SetCheck(CMainDlg::Instance->ReverseRotation);
	pCheckConsole->SetCheck(CMainDlg::Instance->GetGameRenderer()->GetPropertyBool("ShowConsole"));
	pCheckDebugtool->SetCheck(CMainDlg::Instance->GetGameRenderer()->GetPropertyBool("DebugTool"));
	pCheckInfoov->SetCheck(CMainDlg::Instance->GetGameRenderer()->GetPropertyBool("ShowInfoOverlay"));
	return TRUE;
}
void CSettingsDlg::OnClose()
{
	CMainDlg::Instance->CheckImageRatio = pCheckImageRatio->IsChecked();
	CMainDlg::Instance->ReverseRotation = pCheckReverseRotation->IsChecked();
	CMainDlg::Instance->GetGameRenderer()->SetProperty("ShowConsole", pCheckConsole->IsChecked() ? L"true" : L"false");
	CMainDlg::Instance->GetGameRenderer()->SetProperty("DebugTool", pCheckDebugtool->IsChecked() ? L"true" : L"false");
	CMainDlg::Instance->GetGameRenderer()->SetProperty("ShowInfoOverlay", pCheckInfoov->IsChecked() ? L"true" : L"false");
	OnOK();
}
