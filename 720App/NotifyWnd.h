#pragma once
#include "stdafx.h"

class CNotifyWnd : public SHostWnd
{
public:
	CNotifyWnd();
	~CNotifyWnd();

	void SetTip(const SStringT& strTip, int showTime);
	void Show(int x, int y);
	void Show();
	void Hide();
protected:

	BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
	virtual void OnFinalMessage(HWND hWnd);
	void OnTimer(UINT_PTR timerID);

	BEGIN_MSG_MAP_EX(CNotifyWnd)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		CHAIN_MSG_MAP(SHostWnd)
	END_MSG_MAP()

	SWindow* pText = nullptr;
	IAnimation* pAniExit = nullptr;
	IAnimation* pAniIn = nullptr;
	int showTime = 2500;
};

