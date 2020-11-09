#include "SystemHelper.h"
#include "CStringHlp.h"
#include "PathHelper.h"
#include <commdlg.h>
#include <Shlwapi.h>
#include <shellapi.h>

bool SystemHelper::ChooseOneFile(HWND hWnd, LPCWSTR startDir, LPCWSTR title, LPCWSTR fileFilter, LPCWSTR fileName, LPCWSTR defExt, LPCWSTR strrs, size_t bufsize)
{
	if (strrs) {
		OPENFILENAME ofn;
		TCHAR szFile[MAX_PATH];
		if (fileName != 0 && wcslen(fileName) != 0)
			wcscpy_s(szFile, fileName);
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.nFilterIndex = 1;
		ofn.lpstrFilter = fileFilter;
		ofn.lpstrDefExt = defExt;
		ofn.lpstrFileTitle = (LPWSTR)title;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = startDir;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (GetOpenFileName(&ofn))
		{
			//显示选择的文件。 szFile
			wcscpy_s((LPWSTR)strrs, bufsize, szFile);
			return TRUE;
		}
	}
	return 0;
}
bool SystemHelper::ChooseOneFileA(HWND hWnd, LPCSTR startDir, LPCSTR title, LPCSTR fileFilter, LPCSTR fileName, LPCSTR defExt, LPCSTR strrs, size_t bufsize)
{
	if (strrs) {
		OPENFILENAMEA ofn;
		CHAR szFile[MAX_PATH];
		if (fileName != 0 && strlen(fileName) != 0)
			strcpy_s(szFile, fileName);
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.nFilterIndex = 1;
		ofn.lpstrFilter = fileFilter;
		ofn.lpstrDefExt = defExt;
		ofn.lpstrFileTitle = (LPSTR)title;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = startDir;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (GetOpenFileNameA(&ofn))
		{
			//显示选择的文件。 szFile
			strcpy_s((LPSTR)strrs, bufsize, szFile);
			return TRUE;
		}
	}
	return 0;
}
bool SystemHelper::FileExists(LPCWSTR file) {
	return PathFileExists(file);
}
bool SystemHelper::FileExists(LPCSTR file) {
	return PathFileExistsA(file);
}
bool SystemHelper::OpenAs(LPCWSTR file) {
	if(Path::Exists(L"C:\\Windows\\System32\\OpenWith.exe"))
		return ShellExecute(NULL, L"open", L"OpenWith.exe", file, NULL, NULL);
	else
		return ShellExecute(NULL, L"open", L"rundll32", CStringHlp::FormatString(L"shell32, OpenAs_RunDLL %s", file).c_str(), NULL, NULL);
}
bool SystemHelper::ShowInExplorer(LPCWSTR file) {
	return ShellExecute(NULL, L"open", L"explorer.exe", CStringHlp::FormatString(L"/select, %s", file).c_str(), NULL, NULL);
}
bool SystemHelper::ShellDeleteFile(HWND hWnd, LPCWSTR file) {
	SHFILEOPSTRUCT lpfileop;
	lpfileop.hwnd = hWnd;
	lpfileop.wFunc = FO_DELETE;//删除操作
	lpfileop.pFrom = file;
	lpfileop.pTo = NULL;
	lpfileop.fFlags = FOF_ALLOWUNDO | FOF_NO_UI;
	lpfileop.hNameMappings = NULL;
	return SHFileOperation(&lpfileop) == 0;
}
bool SystemHelper::ShellCopyFile(HWND hWnd, LPCWSTR file) {
	SHFILEOPSTRUCT lpfileop;
	lpfileop.hwnd = hWnd;
	lpfileop.wFunc = FO_COPY;
	lpfileop.pFrom = file;
	lpfileop.pTo = NULL;
	lpfileop.hNameMappings = NULL;
	return SHFileOperation(&lpfileop) == 0;
}
bool SystemHelper::CopyToClipBoard(LPCWSTR string) {
	if (OpenClipboard(NULL))
	{
		size_t bufferSize = (wcslen(string) + 1) * sizeof(WCHAR);

		HGLOBAL hMem = GlobalAlloc(GMEM_DDESHARE, bufferSize);
		WCHAR* pMem = (WCHAR*)GlobalLock(hMem);
		EmptyClipboard();

		memset(pMem, 0, bufferSize);
		memcpy(pMem, string, bufferSize);

		GlobalUnlock(pMem);
		SetClipboardData(CF_UNICODETEXT, hMem);
		CloseClipboard();
		GlobalFree(hMem);
		return true;
	}
	return false;
}