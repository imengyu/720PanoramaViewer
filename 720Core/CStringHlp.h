#pragma once
#ifndef STRING_HLP_H
#define STRING_HLP_H
#include "stdafx.h"
#include <stdio.h>  
#include <stdlib.h>  
#include <string>  
#include <vector>

//�ַ���������
class CStringHlp
{
public:

	/**
	 * ��ȡ��С�Ŀɶ��ַ���
	 * @param byteSize ��С
	 * @return
	 */
	static std::wstring GetFileSizeStringAuto(long long byteSize);

	static std::string & FormatString(std::string & _str, const char * format, ...);
	static std::wstring & FormatString(std::wstring & _str, const wchar_t * format, ...);
	static std::wstring FormatString(const wchar_t * format, ...);
	static std::wstring FormatString(const wchar_t *format, va_list marker);
	static std::string FormatString(const char * format, va_list marker);
	static std::string FormatString(const char * format, ...);

	/**
	 * ���ַ� Unicode ת Char
	 * @param szStr Դ�ַ���
	 * @return
	 */
	static std::string UnicodeToAnsi(std::wstring szStr);
	/**
	 * ���ַ� Unicode ת UTF8
	 * @param unicode Դ�ַ���
	 * @return
	 */
	static std::string UnicodeToUtf8(std::wstring unicode);
	/**
	 * Char ת���ַ� Unicode
	 * @param szStr Դ�ַ���
	 * @return
	 */
	static std::wstring AnsiToUnicode(std::string szStr);
	/**
	 * UTF8 �ַ���ת Unicode���ַ�
	 * @param szU8 Դ�ַ���
	 * @return
	 */
	static std::wstring Utf8ToUnicode(std::string szU8);

	static bool StrEqualA(const char* str1, const char* str2);
	static bool StrEqualW(const wchar_t* str1, const wchar_t* str2);
	static bool StrEmeptyW(const wchar_t* str);
	static bool StrEmeptyA(const char* strSTR);
	static int StrToIntA(const char* str);
	static int StrToIntW(const wchar_t* str);
	static bool StrToBoolA(const char* str);
	static bool StrToBoolW(const wchar_t* str);
	static char* IntToStrA(int i);
	static wchar_t* IntToStrW(int i);
};

#endif








