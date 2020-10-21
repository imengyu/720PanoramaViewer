#pragma once
#include "stdafx.h"
#include "COpenGLView.h"

enum CAppUIMessageBoxIcon {
	IconNone,
	IconInfo,
	IconWarning,
	IconError,
};
enum CAppUIMessageBoxResult {
	ResultOK,
	ResultCancel,
	ResultOther,
};

class CAppUIWapper
{
public:
	CAppUIWapper(COpenGLView *view);
	~CAppUIWapper();

	void MessageBeep(CAppUIMessageBoxIcon ico);
	void ShowMessageBox(const wchar_t* str, const wchar_t* title);
	void ShowMessageBox(const wchar_t* str, const wchar_t* title, CAppUIMessageBoxIcon ico);
	CAppUIMessageBoxResult ShowConfirmBox(const wchar_t* str, const wchar_t* title, const wchar_t* okText, const wchar_t* cancelText, CAppUIMessageBoxIcon ico);


private:

	COpenGLView* view;
};

