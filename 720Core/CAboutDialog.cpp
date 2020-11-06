#include "CAboutDialog.h"
#include "CRegDialog.h"
#include "CApp.h"
#include "resource.h"

void CAboutDialog::Show(HWND hWnd)
{
    DialogBox(CApp::Instance->GetHInstance(), MAKEINTRESOURCE(IDD_ABOUT), hWnd, About);
}
void CAboutDialog::LoadReged(HWND hDlg)
{
    bool reged = CApp::Instance->GetSettings()->GetSettingBool(L"registered", false);
    if (reged) {
        ShowWindow(GetDlgItem(hDlg, IDC_REGIST), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_REG_TO), SW_SHOW);
    }
    else {
        ShowWindow(GetDlgItem(hDlg, IDC_REG_TO), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_REGIST), SW_SHOW);
    }
    
}
INT_PTR CAboutDialog::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG: {
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(CApp::Instance->GetHInstance(), MAKEINTRESOURCE(IDI_APP)));
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(CApp::Instance->GetHInstance(), MAKEINTRESOURCE(IDI_APP)));
        LoadReged(hDlg);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_REGIST) {
            CRegDialog::Show(hDlg);
            LoadReged(hDlg);
            return TRUE;
        }
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
