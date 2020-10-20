#pragma once
#include "stdafx.h"
#include "resource.h"

class CRegDialog
{
public:
    static void Show(HWND hWnd);

private:
    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

