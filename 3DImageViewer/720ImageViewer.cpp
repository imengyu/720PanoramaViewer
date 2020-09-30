// 3DImageViewer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "CApp.h"
#include <dbghelp.h>

CApp *app = nullptr;

BOOL GenerateCrashInfo(PEXCEPTION_POINTERS pExInfo, LPCWSTR info_file_name, LPCWSTR file_name, SYSTEMTIME tm, LPCWSTR dir) {

	FILE* fp = NULL;
	_wfopen_s(&fp, info_file_name, L"w");
	if (fp) {
		fwprintf_s(fp, L"=== 720ImageViewer ===== %04d/%02d/%02d %02d:%02d:%02d ===========", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
		fwprintf_s(fp, L"\r\n应用程序错误 ：Address : 0x%08x  Code : 0x%08X  (0x%08X)",
			(ULONG_PTR)pExInfo->ExceptionRecord->ExceptionAddress, pExInfo->ExceptionRecord->ExceptionCode,
			pExInfo->ExceptionRecord->ExceptionFlags);
		fwprintf_s(fp, L"\r\n=== 720ImageViewer =====================================");
		fwprintf_s(fp, L"\r\n我们生成了关于描述这个错误的错误报告(不包含您的个人信息)：");
		fwprintf_s(fp, L"\r\n=== 文件内容 =====================================");
		fwprintf_s(fp, L"\r\n[错误转储文件] %s", file_name);
		fwprintf_s(fp, L"\r\n====================================");
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}
LONG GenerateMiniDump(PEXCEPTION_POINTERS pExInfo)
{
	TCHAR dmp_path[MAX_PATH];
	wcscpy_s(dmp_path, app->GetCurrentDir());

	SYSTEMTIME tm;
	GetLocalTime(&tm);//获取时间
	TCHAR file_name[128];
	swprintf_s(file_name, L"%s\\CrashDump%d%02d%02d-%02d%02d%02d.dmp", dmp_path,
		tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);//设置dmp文件名称
	TCHAR info_file_name[128];
	swprintf_s(info_file_name, L"%s\\CrashInfo%d%02d%02d-%02d%02d%02d.txt", dmp_path,
		tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);

	//Create file
	HANDLE hFile = CreateFile(file_name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	//Generate Crash info
	BOOL hasCrashInfo = GenerateCrashInfo(pExInfo, info_file_name, file_name, tm, dmp_path);

	//Gen Dump File and show dialog

	TCHAR expInfo[128];
	swprintf_s(expInfo, L"Exception !!! Address : 0x%08x  Code : 0x%08X  (0x%08X)",
		(ULONG_PTR)pExInfo->ExceptionRecord->ExceptionAddress, pExInfo->ExceptionRecord->ExceptionCode,
		pExInfo->ExceptionRecord->ExceptionFlags);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION expParam;
		expParam.ThreadId = GetCurrentThreadId();
		expParam.ExceptionPointers = pExInfo;
		expParam.ClientPointers = FALSE;
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, (pExInfo ? &expParam : NULL), NULL, NULL);
		CloseHandle(hFile);

		TCHAR info[300];
		swprintf_s(info, L"应用程序出现了一个错误，%s。\n%s", (hasCrashInfo ? L"需要关闭，已生成关于此错误的报告" : L"并且无法生成错误转储文件"), expInfo);
		MessageBox(NULL, info, L"应用程序错误", MB_ICONERROR);
	}
	else
	{
		TCHAR info[300];
		swprintf_s(info, L"应用程序出现了一个错误，并且无法生成错误转储文件。\n%s\nFail to create dump file: %s \nLast Error : %d\n现在应用程序即将关闭。", expInfo, file_name, GetLastError());
		MessageBox(NULL, info, L"应用程序错误", MB_ICONERROR | MB_SYSTEMMODAL);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}
LONG WINAPI AppUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
{
	if (IsDebuggerPresent())
		return EXCEPTION_CONTINUE_SEARCH;
	return GenerateMiniDump(pExInfo);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	SetUnhandledExceptionFilter(NULL);
	SetUnhandledExceptionFilter(AppUnhandledExceptionFilter);

	app = new CApp();
	int rs = app->Run();
	delete app;
	return rs;
}