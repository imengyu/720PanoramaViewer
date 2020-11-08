#pragma once
#include "stdafx.h"

class CMainDlg;
class CPanoModeWnd : public SHostWnd
{
public:

	CPanoModeWnd(CMainDlg* mainDlg);
	~CPanoModeWnd();

	void Show(int x, int y);
	void Hide();

private:
	CMainDlg* mainDialog;

protected:

	BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnLostFocus(HWND hWndFocus);
	void OnModeCylinder();
	void OnModeSphere();
	void OnModeAsteroid();
	void OnModeOutball();
	void OnMode360();
	void OnModeOrginal();

	virtual void OnFinalMessage(HWND hWnd);

	void OnTimer(UINT_PTR timerID);

	//souiœ˚œ¢
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_pano_cylinder", OnModeCylinder)
		EVENT_NAME_COMMAND(L"btn_pano_sphere", OnModeSphere)
		EVENT_NAME_COMMAND(L"btn_pano_asteroid", OnModeAsteroid)
		EVENT_NAME_COMMAND(L"btn_pano_outball", OnModeOutball)
		EVENT_NAME_COMMAND(L"btn_pano_360", OnMode360)
		EVENT_NAME_COMMAND(L"btn_pano_orginal", OnModeOrginal)
	EVENT_MAP_END()

	BEGIN_MSG_MAP_EX(CPanoModeWnd)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_KILLFOCUS(OnLostFocus)
		MSG_WM_TIMER(OnTimer)
		CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP()

};

