// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HelpDlg.h"	


CHelpDlg::CHelpDlg() : SHostDialog(_T("LAYOUT:XML_HELPWND"))
{
}
CHelpDlg::~CHelpDlg()
{

}

BOOL CHelpDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	return TRUE;
}
void CHelpDlg::OnClose()
{
	OnCancel();
}
void CHelpDlg::OnMinimize()
{
	ShowWindow(SW_MINIMIZE);
}
