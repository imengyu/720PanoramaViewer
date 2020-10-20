#include "SystemHelper.h"
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
	return ShellExecute(NULL, L"open", L"OpenWith.exe", file, NULL, NULL);
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
