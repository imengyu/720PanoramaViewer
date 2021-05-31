#pragma once
#include "stdafx.h"

struct MultiFileSelectResult {
	size_t fileCount;
	LPWSTR *files;
	LPWSTR dir;
};

class VR720_EXP SystemHelper
{
public:
   static bool ChooseOneFile(HWND hWnd, LPCWSTR startDir, LPCWSTR title, LPCWSTR fileFilter, LPCWSTR fileName, LPCWSTR defExt, LPCWSTR strrs, size_t bufsize);
   static bool ChooseOneFileA(HWND hWnd, LPCSTR startDir, LPCSTR title, LPCSTR fileFilter, LPCSTR fileName, LPCSTR defExt, LPCSTR strrs, size_t bufsize);
   static MultiFileSelectResult* ChooseMultiFile(HWND hWnd, LPCWSTR startDir, LPCWSTR title, LPCWSTR fileFilter, LPCWSTR fileName, LPCWSTR defExt);
   static void DeleteMultiFilResult(MultiFileSelectResult* v);
   static bool SaveFile(HWND hWnd, LPCWSTR startDir, LPCWSTR title, LPCWSTR fileFilter, LPCWSTR fileName, LPCWSTR defExt, LPCWSTR strrs, size_t bufsize);
   static bool FileExists(LPCWSTR file);
   static bool FileExists(LPCSTR file);
   static bool OpenAs(LPCWSTR file);
   static bool Open(LPCWSTR file);
   static bool ShellDeleteFile(HWND hWnd, LPCWSTR file);
   static bool ShellCopyFile(HWND hWnd, LPCWSTR file);
   static bool ShowInExplorer(HWND hWndMain, LPCWSTR file);
   static bool ShowFileProperties(HWND hWndMain, LPCWSTR file);
   static bool CopyToClipBoard(LPCWSTR string);
   static bool IsVistaOrLater();
   static BOOL IsWinVersionEqualTo(DWORD dwMajorVersion, DWORD dwMinorVersion);
   static BOOL IsWinVersionGreaterThan(DWORD dwMajorVersion, DWORD dwMinorVersion);
   static BOOL IsWinVersionLessThan(DWORD dwMajorVersion, DWORD dwMinorVersion);
   ;
};

