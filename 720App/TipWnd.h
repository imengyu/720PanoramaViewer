#pragma once
#include "stdafx.h"

class CTipWnd : public SHostWnd
{
public:
	CTipWnd();
	~CTipWnd();

	void SetTip(const SStringT& strTip);
	void Show(int x, int y);
	void Show();
	void Hide();
protected:

	BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
	virtual void OnFinalMessage(HWND hWnd);
	void OnTimer(UINT_PTR timerID);

	BEGIN_MSG_MAP_EX(CTipWnd)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		CHAIN_MSG_MAP(SHostWnd)
	END_MSG_MAP()

	SWindow* pText = nullptr;
};

