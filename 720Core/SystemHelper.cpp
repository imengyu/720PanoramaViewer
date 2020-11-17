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
MultiFileSelectResult* SystemHelper::ChooseMultiFile(HWND hWnd, LPCWSTR startDir, LPCWSTR title, LPCWSTR fileFilter, LPCWSTR fileName, LPCWSTR defExt)
{
	OPENFILENAME ofn;
	TCHAR szFile[2048];
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
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	if (GetOpenFileName(&ofn))
	{
		MultiFileSelectResult* result = new MultiFileSelectResult();

		wchar_t* ptr = ofn.lpstrFile;
		ptr[ofn.nFileOffset - 1] = 0;

		result->dir = (LPWSTR)malloc((lstrlenW(ptr) + 1) * sizeof(wchar_t));
		lstrcpyW(result->dir, ptr);

		std::vector<LPWSTR> strs;

		ptr += ofn.nFileOffset;
		while (*ptr)
		{
			strs.push_back(ptr);
			ptr += (lstrlenW(ptr) + 1);
		}

		result->fileCount = strs.size();
		result->files = (LPWSTR*)malloc(result->fileCount * sizeof(LPWSTR));

		for (size_t i = 0; i < result->fileCount; i++) {
			LPWSTR str = strs[i];
			result->files[i] = (LPWSTR)malloc((lstrlenW(str) + 1) * sizeof(wchar_t));
			lstrcpyW(result->files[i], str);
		}

		return result;
	}
	return nullptr;
}
void SystemHelper::DeleteMultiFilResult(MultiFileSelectResult* v)
{
	for (size_t i = 0; i < v->fileCount; i++)
		delete v->files[i];
	delete v->dir;
	delete v;
}

bool SystemHelper::FileExists(LPCWSTR file) {
	return PathFileExists(file);
}
bool SystemHelper::FileExists(LPCSTR file) {
	return PathFileExistsA(file);
}
bool SystemHelper::OpenAs(LPCWSTR file) {
	if(Path::Exists(L"C:\\Windows\\System32\\OpenWith.exe"))
		return (int)ShellExecute(NULL, L"open", L"OpenWith.exe", file, NULL, NULL) >= 32;
	else
		return (int)ShellExecute(NULL, L"open", L"rundll32", CStringHlp::FormatString(L"shell32, OpenAs_RunDLL %s", file).c_str(), NULL, NULL) >= 32;
}
bool SystemHelper::Open(LPCWSTR file) {
	return (int)ShellExecute(NULL, L"open", file, NULL, NULL, NULL) >= 32;
}
bool SystemHelper::ShowInExplorer(HWND hWndMain, LPCWSTR szFile) {
	std::wstring path2 = CStringHlp::FormatString(L"/select, %s", szFile);
	return (ULONG_PTR)ShellExecute(hWndMain, L"open", L"explorer.exe", path2.c_str(), NULL, 5) >= 32;
}
bool SystemHelper::ShowFileProperties(HWND hWndMain, LPCWSTR file) {
	SHELLEXECUTEINFO info = { 0 };
	info.cbSize = sizeof(SHELLEXECUTEINFO);
	info.hwnd = hWndMain;	
	info.lpVerb = L"properties";
	info.lpFile = file;
	info.nShow = SW_SHOW;
	info.fMask = SEE_MASK_INVOKEIDLIST;
	return ShellExecuteEx(&info) >= 32;
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
bool SystemHelper::IsVistaOrLater() {
	static int isVista = -1;
	if (isVista == -1)
		isVista = IsWinVersionGreaterThan(6, 0);
	return isVista != FALSE;
}
BOOL SystemHelper::IsWinVersionEqualTo(DWORD dwMajorVersion, DWORD dwMinorVersion)
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;

	//Initialize the OSVERSIONINFOEX structure.
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = dwMajorVersion;
	osvi.dwMinorVersion = dwMinorVersion;

	//Initialize the condition mask.
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_EQUAL);

	return VerifyVersionInfo(
		&osvi,
		VER_MAJORVERSION | VER_MINORVERSION,
		dwlConditionMask
	);
}
BOOL SystemHelper::IsWinVersionGreaterThan(DWORD dwMajorVersion, DWORD dwMinorVersion)
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;

	//Initialize the OSVERSIONINFOEX structure.
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = dwMajorVersion;
	osvi.dwMinorVersion = dwMinorVersion;

	//system major version > dwMajorVersion
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER);
	if (VerifyVersionInfo(&osvi, VER_MAJORVERSION, dwlConditionMask))
		return TRUE;

	//sytem major version = dwMajorVersion && minor version > dwMinorVersion
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER);

	return VerifyVersionInfo(
		&osvi,
		VER_MAJORVERSION | VER_MINORVERSION,
		dwlConditionMask
	);
}
BOOL SystemHelper::IsWinVersionLessThan(DWORD dwMajorVersion, DWORD dwMinorVersion)
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;

	//Initialize the OSVERSIONINFOEX structure.
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = dwMajorVersion;
	osvi.dwMinorVersion = dwMinorVersion;

	//system major version < dwMajorVersion
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_LESS);
	if (VerifyVersionInfo(&osvi, VER_MAJORVERSION, dwlConditionMask))
		return TRUE;

	//sytem major version = dwMajorVersion && minor version < dwMinorVersion
	dwlConditionMask = 0;
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_LESS);

	return VerifyVersionInfo(
		&osvi,
		VER_MAJORVERSION | VER_MINORVERSION,
		dwlConditionMask
	);
}