#pragma once
#include "stdafx.h"
#include "resource.h"

class CAboutDialog
{
public:
    static void Show(HWND hWnd);



private:

    static void LoadReged(HWND hWnd);
    static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

