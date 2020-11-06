#include "CAppUIWapper.h"
#include "CWindowsOpenGLView.h"

CAppUIWapper::CAppUIWapper(COpenGLView* view)
{
	this->view = view;
}
CAppUIWapper::~CAppUIWapper()
{
}

void CAppUIWapper::MessageBeep(CAppUIMessageBoxIcon ico)
{
	switch (ico)
	{
	case IconInfo:
		::MessageBeep(MB_ICONINFORMATION);
		break;
	case IconWarning:
		::MessageBeep(MB_ICONEXCLAMATION);
		break;
	case IconError:
		::MessageBeep(MB_ICONERROR);
		break;
	}
}
void CAppUIWapper::ShowMessageBox(const wchar_t* str, const wchar_t* title)
{
	MessageBox(((CWindowsOpenGLView*)view)->GetHWND(), str, title, MB_OK);
}
void CAppUIWapper::ShowMessageBox(const wchar_t* str, const wchar_t* title, CAppUIMessageBoxIcon ico)
{
	switch (ico)
	{
	case IconNone:
		MessageBox(((CWindowsOpenGLView*)view)->GetHWND(), str, title, MB_OK);
		break;
	case IconInfo:
		MessageBox(((CWindowsOpenGLView*)view)->GetHWND(), str, title, MB_ICONINFORMATION);
		break;
	case IconWarning:
		MessageBox(((CWindowsOpenGLView*)view)->GetHWND(), str, title, MB_ICONEXCLAMATION);
		break;
	case IconError:
		MessageBox(((CWindowsOpenGLView*)view)->GetHWND(), str, title, MB_ICONERROR);
		break;
	}
}

CAppUIMessageBoxResult CAppUIWapper::ShowConfirmBox(const wchar_t* str, const wchar_t* title, const wchar_t* okText, const wchar_t* cancelText, CAppUIMessageBoxIcon ico)
{
	int result = 0;
	switch (ico)
	{
	case IconNone:
		result = MessageBox(((CWindowsOpenGLView*)view)->GetHWND(), str, title, MB_YESNO);
		break;
	case IconInfo:
		result = MessageBox(((CWindowsOpenGLView*)view)->GetHWND(), str, title, MB_ICONINFORMATION | MB_YESNO);
		break;
	case IconWarning:
		result = MessageBox(((CWindowsOpenGLView*)view)->GetHWND(), str, title, MB_ICONEXCLAMATION | MB_YESNO);
		break;
	case IconError:
		result = MessageBox(((CWindowsOpenGLView*)view)->GetHWND(), str, title, MB_ICONERROR | MB_YESNO);
		break;
	}
	switch (result)
	{
	case IDYES:
		return ResultOK;
	case IDNO:
		return ResultCancel;
	default:
		return ResultOther;
	}
}
