#include "CCursor.h"

CCursor* CCursor::Default;
CCursor* CCursor::Arrow;
CCursor* CCursor::Grab;
CCursor* CCursor::Hand;

void CCursor::Init(HINSTANCE hInst) {
	Default = new CCursor(hInst, MAKEINTRESOURCE(IDC_CUR_ARROW));
	Hand = new CCursor(hInst, MAKEINTRESOURCE(IDC_CUR_HAND));
	Arrow = new CCursor(hInst, MAKEINTRESOURCE(IDC_CUR_ARROW));
	Grab = new CCursor(hInst, MAKEINTRESOURCE(IDC_CUR_MOVE));
}

void CCursor::Destroy() {
	delete Default;
	delete Arrow;
	delete Grab;
	delete Hand;
}

void CCursor::SetViewCursur(COpenGLView* view, CCursor* cursor)
{
	SetClassLong(view->GetHWND(), GCL_HCURSOR, (LONG)cursor->hCursor);
	SetCursor(cursor->hCursor);
}

CCursor::CCursor(LPCWSTR type)
{
	hCursor = LoadCursor(NULL, type);
}
CCursor::CCursor(HINSTANCE hInst, LPCWSTR type)
{
	hCursor = LoadCursor(hInst, type);
}
CCursor::CCursor(char* texturePath)
{
	hCursor = LoadCursorFromFileA(texturePath);
}
CCursor::~CCursor()
{
	DestroyCursor(hCursor);
}
