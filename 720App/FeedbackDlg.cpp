// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FeedbackDlg.h"	


CFeedbackDlg::CFeedbackDlg() : SHostDialog(_T("LAYOUT:XML_FEED_BACKWND"))
{
}
CFeedbackDlg::~CFeedbackDlg()
{

}

BOOL CFeedbackDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	return TRUE;
}
void CFeedbackDlg::OnClose()
{
	OnCancel();
}
