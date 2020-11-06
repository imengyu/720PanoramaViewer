#include "CHelpDialog.h"
#include "CApp.h"
#include "resource.h"

void CHelpDialog::Show(HWND hWnd)
{
    DialogBox(CApp::Instance->GetHInstance(), MAKEINTRESOURCE(IDD_HELP), hWnd, About);
}

INT_PTR CHelpDialog::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG: {
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(CApp::Instance->GetHInstance(), MAKEINTRESOURCE(IDI_HELP)));
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(CApp::Instance->GetHInstance(), MAKEINTRESOURCE(IDI_HELP)));
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
