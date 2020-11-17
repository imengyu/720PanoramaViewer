#pragma once
#include "stdafx.h"
#include <string>

std::wstring& FormatString(std::wstring& _str, const wchar_t* format, ...);
std::wstring FormatString(const wchar_t* format, va_list marker);
std::wstring FormatString(const wchar_t* format, ...);