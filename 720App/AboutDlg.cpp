// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AboutDlg.h"	


CAboutDlg::CAboutDlg() : SHostDialog(_T("LAYOUT:XML_ABOUTWND"))
{
}
CAboutDlg::~CAboutDlg()
{

}

BOOL CAboutDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	pWndArgeement = FindChildByID(3);
	pWndMain = FindChildByID(2);
	pBtnArgeemen = FindChildByName2<SButton>(L"btn_check_agreement");
	return TRUE;
}
void CAboutDlg::OnClose()
{
	OnCancel();
}


void CAboutDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CAboutDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CAboutDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CAboutDlg::OnCheckAgreement()
{
	if (pWndMain->IsVisible()) {
		pWndMain->SetVisible(false, true);
		pWndArgeement->SetVisible(true, true);
		pBtnArgeemen->SetWindowText(L"返回");
	}
	else {
		pWndMain->SetVisible(true, true);
		pWndArgeement->SetVisible(false, true);
		pBtnArgeemen->SetWindowText(L"查看用户使用协议");
	}

}