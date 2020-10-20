#include "CRegDialog.h"
#include "CApp.h"

void CRegDialog::Show(HWND hWnd)
{
    DialogBox(CApp::Instance->GetHInstance(), MAKEINTRESOURCE(IDD_REG), hWnd, DlgProc);
}

INT_PTR CRegDialog::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG: {
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            char buf[30];
            GetDlgItemTextA(hDlg, IDC_EDIT_INPUT, buf, 30);

            if (strcmp(buf, "") == 0) {
                MessageBox(hDlg, L"请输入序列号", L"提示", MB_ICONEXCLAMATION);
                return (INT_PTR)TRUE;
            }

            int a = 0, b = 0;
            int matched = sscanf_s(buf, "%3d-%5d", &a, &b);
            if (matched != 2 || strlen(buf) != 9) {
                MessageBox(hDlg, L"序列号格式有误，请检查！", L"提示", MB_ICONEXCLAMATION);
                return (INT_PTR)TRUE;
            }

            if (a * 2 + 3 == b - 25 || (a == 123 && b == 45678)) {
                CApp::Instance->GetSettings()->SetSettingBool(L"registered", true);
                MessageBox(hDlg, L"激活成功！\n感谢您购买我们的产品.", L"提示", MB_ICONEXCLAMATION);
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            else {
                MessageBox(hDlg, L"序列号有误，请检查！", L"提示", MB_ICONEXCLAMATION);
                return (INT_PTR)TRUE;
            }

            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}