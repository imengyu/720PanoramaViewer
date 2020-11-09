#pragma once
#include "stdafx.h"

class VR720_EXP SystemHelper
{
public:
   static bool ChooseOneFile(HWND hWnd, LPCWSTR startDir, LPCWSTR title, LPCWSTR fileFilter, LPCWSTR fileName, LPCWSTR defExt, LPCWSTR strrs, size_t bufsize);
   static bool ChooseOneFileA(HWND hWnd, LPCSTR startDir, LPCSTR title, LPCSTR fileFilter, LPCSTR fileName, LPCSTR defExt, LPCSTR strrs, size_t bufsize);
   static bool FileExists(LPCWSTR file);
   static bool FileExists(LPCSTR file);
   static bool OpenAs(LPCWSTR file);
   static bool ShellDeleteFile(HWND hWnd, LPCWSTR file);
   static bool ShellCopyFile(HWND hWnd, LPCWSTR file);
   static bool ShowInExplorer(LPCWSTR file);
   static bool CopyToClipBoard(LPCWSTR string);;
};

