#include "CStringHlp.h"

std::string & CStringHlp::FormatString(std::string & _str, const char * format, ...) {
	std::string tmp;

	va_list marker = nullptr;
	va_start(marker, format);

#ifdef _MSC_VER
	size_t num_of_chars = _vscprintf(format, marker);
#else
    size_t num_of_chars = vsprintf(nullptr, format, marker);
#endif
	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}
#if defined(_MSC_VER) && _MSC_VER > 1600
	vsprintf_s((char *)tmp.data(), tmp.capacity(), format, marker);
#else
	vsprintf(nullptr, format, marker);
#endif

	va_end(marker);

	_str = tmp.c_str();
	return _str;
}
std::wstring & CStringHlp::FormatString(std::wstring & _str, const wchar_t * format, ...) {
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
	vswprintf_s((wchar_t *)tmp.data(), tmp.capacity(), format, marker);
#else
	vswprintf((wchar_t *)tmp.data(), tmp.capacity(), format, marker);
#endif

	va_end(marker);
	_str = tmp.c_str();
	return _str;
}
std::wstring CStringHlp::FormatString(const wchar_t * format, va_list marker)
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
	vswprintf_s((wchar_t *)tmp.data(), tmp.capacity(), format, marker);
#else
	vswprintf((wchar_t *)tmp.data(), tmp.capacity(), format, marker);
#endif
	return tmp;
}
std::string CStringHlp::FormatString(const char * format, va_list marker)
{
	std::string tmp;
#ifdef _MSC_VER
	size_t num_of_chars = _vscprintf(format, marker);
#else
	size_t num_of_chars = vsprintf(nullptr, format, marker);
#endif
	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}

#if defined(_MSC_VER) && _MSC_VER > 1600
	vsprintf_s((char *)tmp.data(), tmp.capacity(), format, marker);
#else
	vsprintf(nullptr, format, marker);
#endif
	return tmp;
}
std::wstring CStringHlp::FormatString(const wchar_t * format, ...)
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
	vswprintf_s((wchar_t *)tmp.data(), tmp.capacity(), format, marker);
#else
	vswprintf((wchar_t *)tmp.data(), tmp.capacity(), format, marker);
#endif
	va_end(marker);
	return tmp;
}
std::string CStringHlp::FormatString(const char * format, ...)
{
	std::string tmp;

	va_list marker = NULL;
	va_start(marker, format);

#ifdef _MSC_VER
	size_t num_of_chars = _vscprintf(format, marker);
#else
	size_t num_of_chars = vsprintf(nullptr, format, marker);
#endif
	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}

#if defined(_MSC_VER) && _MSC_VER > 1600
	vsprintf_s((char *)tmp.data(), tmp.capacity(), format, marker);
#else
	vsprintf(nullptr, format, marker);
#endif

	va_end(marker);

	std::string _str = tmp.c_str();
	return _str;
}

char* CStringHlp::AllocString(std::string& str)
{
	char* s = new char[str.size() + 1];
	strcpy_s(s, str.size() + 1, str.data());
	return s;
}
wchar_t* CStringHlp::AllocString(std::wstring &str)
{
	wchar_t* s = new wchar_t[str.size() + 1];
	wcscpy_s(s, str.size() + 1, str.data());
	return s;
}

std::wstring CStringHlp::GetFileSizeStringAuto(long long byteSize) {
	std::wstring sizeStr;
	double size;
	if (byteSize >= 1073741824) {
		size = round(byteSize / 1073741824.0 * 100.0) / 100.0;
		sizeStr = FormatString(L"%d GB", (long)size);
	}
	else if (byteSize >= 1048576) {
		size = round(byteSize / 1048576.0 * 100.0) / 100.0;
		sizeStr = FormatString(L"%d MB", (long)size);
	}
	else {
		size = round(byteSize / 1024.0 * 100.0) / 100.0;
		sizeStr = FormatString(L"%d KB", (long)size);
	}
	return sizeStr;
}

std::string CStringHlp::UnicodeToAnsi(std::wstring szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr.c_str(), -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
		return NULL;
	std::string pResult;
	pResult.resize(nLen);
	WideCharToMultiByte(CP_ACP, 0, szStr.c_str(), -1, (LPSTR)pResult.data(), nLen, NULL, NULL);
	return pResult;
}
std::string CStringHlp::UnicodeToUtf8(std::wstring unicode)
{
	int len;
	len = WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, NULL, 0, NULL, NULL);
	std::string  szUtf8;
	szUtf8.resize(len);
	WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, (LPSTR)szUtf8.data(), len, NULL, NULL);
	return szUtf8;
}
std::wstring CStringHlp::AnsiToUnicode(std::string szStr)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr.c_str(), -1, NULL, 0);
	if (nLen == 0)
		return NULL;
	std::wstring  pResult;
	pResult.resize(nLen + 1);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr.c_str(), -1, (LPWSTR)pResult.data(), nLen);
	return pResult;
}
std::wstring CStringHlp::Utf8ToUnicode(std::string szU8)
{
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, szU8.c_str(), szU8.size(), NULL, 0);
	std::wstring wszString;
	wszString.resize(wcsLen + 1) ;
	::MultiByteToWideChar(CP_UTF8, NULL, szU8.c_str(), szU8.size(), (LPWSTR)wszString.data(), wcsLen);
	return wszString;
}

bool CStringHlp::StrEqualA(const char* str1, const char* str2)
{
	return (strcmp(str1, str2) == 0);
}
bool CStringHlp::StrEqualW(const wchar_t* str1, const wchar_t* str2)
{
	return (wcscmp(str1, str2) == 0);
}
bool CStringHlp::StrEmeptyW(const wchar_t* str)
{
	return !str || CStringHlp::StrEqualW(str, L"");
}
bool CStringHlp::StrEmeptyA(const char* str)
{
	return !str || CStringHlp::StrEqualA(str, "");
}
int CStringHlp::StrToIntA(const char* str)
{
	return atoi(str);
}
int CStringHlp::StrToIntW(const wchar_t* str)
{
	return _wtoi(str);
}
bool CStringHlp::StrToBoolA(const char* str)
{
	return StrEqualA(str, "1") || StrEqualA(str, "true") || StrEqualA(str, "TRUE") || StrEqualA(str, "True") || StrEqualA(str, "ÊÇ");
}
bool CStringHlp::StrToBoolW(const wchar_t* str)
{
	return StrEqualW(str, L"1") || StrEqualW(str, L"true") || StrEqualW(str, L"TRUE") || StrEqualW(str, L"True") || StrEqualW(str, L"ÊÇ");
}
char* CStringHlp::IntToStrA(int i)
{
	int n = 1, i2 = i;
	if (i == 0)
		n = 2;
	else
	{
		while (i2)
		{
			i2 = i2 / 10;
			n++;
		}
		if (i < 0)
			n++;
	}

	char* rs = new char[n];
	_itoa_s(i, rs, n, 10);
	return rs;
}
wchar_t* CStringHlp::IntToStrW(int i)
{
	int n = 1, i2 = i;
	if (i == 0)
		n = 2;
	else
	{
		while (i2)
		{
			i2 = i2 / 10;
			n++;
		}
		if (i < 0)
			n++;
	}

	WCHAR* rs = new WCHAR[n];
	_itow_s(i, rs, n, 10);
	return rs;
}


