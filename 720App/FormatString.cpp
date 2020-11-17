#include "stdafx.h"
#include "FormatString.h"

std::wstring& FormatString(std::wstring& _str, const wchar_t* format, ...) {
	std::wstring tmp;
	va_list marker = NULL;
	va_start(marker, format);
#ifdef _MSC_VER
	size_t num_of_chars = _vscwprintf(format, marker);
#else
	size_t num_of_chars = vswprintf(nullptr, 0, format, marker);
#endif
	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}
#if defined(_MSC_VER) && _MSC_VER > 1600
	vswprintf_s((wchar_t*)tmp.data(), tmp.capacity(), format, marker);
#else
	vswprintf((wchar_t*)tmp.data(), tmp.capacity(), format, marker);
#endif

	va_end(marker);
	_str = tmp.c_str();
	return _str;
}
std::wstring FormatString(const wchar_t* format, va_list marker)
{
	std::wstring tmp;
#ifdef _MSC_VER
	size_t num_of_chars = _vscwprintf(format, marker);
#else
	size_t num_of_chars = vswprintf(nullptr, 0, format, marker);
#endif
	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}
#if defined(_MSC_VER) && _MSC_VER > 1600
	vswprintf_s((wchar_t*)tmp.data(), tmp.capacity(), format, marker);
#else
	vswprintf((wchar_t*)tmp.data(), tmp.capacity(), format, marker);
#endif
	return tmp;
}
std::wstring FormatString(const wchar_t* format, ...)
{
	std::wstring tmp;
	va_list marker = NULL;
	va_start(marker, format);
#ifdef _MSC_VER
	size_t num_of_chars = _vscwprintf(format, marker);
#else
	size_t num_of_chars = vswprintf(nullptr, 0, format, marker);
#endif

	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}
#if defined(_MSC_VER) && _MSC_VER > 1600
	vswprintf_s((wchar_t*)tmp.data(), tmp.capacity(), format, marker);
#else
	vswprintf((wchar_t*)tmp.data(), tmp.capacity(), format, marker);
#endif
	va_end(marker);
	return tmp;
}