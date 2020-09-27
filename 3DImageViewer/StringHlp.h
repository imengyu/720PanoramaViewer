#pragma once
#include "stdafx.h"
#include <stdio.h>  
#include <stdlib.h>  
#include <string>  
#include <vector>

//�ַ���������
class StringHlp
{
public:
	StringHlp();
	~StringHlp();

	static std::string & FormatString(std::string & _str, const char * _Format, ...);
	static std::wstring & FormatString(std::wstring & _str, const wchar_t * _Format, ...);
	static std::wstring FormatString(const wchar_t * format, ...);
	static std::wstring FormatString(const wchar_t *_Format, va_list marker);
	static std::string FormatString(const char *_Format, va_list marker);
	static std::string FormatString(const char * format, ...);

	static wchar_t* Utf8ToUnicode(const char* szU8);
	static char* UnicodeToAnsi(const wchar_t* szStr);
	static char* UnicodeToUtf8(const wchar_t* unicode);
	static wchar_t* AnsiToUnicode(const char* szStr);
	static void FreeStringPtr(void * ptr);

	static std::string* FormatStringPtr2A(std::string *_str, const char * _Format, ...);
	static std::wstring * FormatStringPtr2W(std::wstring *_str, const wchar_t * _Format, ...);
	static std::wstring * FormatStringPtrW(const wchar_t *format, ...);
	static std::string *FormatStringPtrA(const char *format, ...);

	//�ַ����Ƿ��ǿյ�
	static BOOLEAN StrEmeptyW(LPCWSTR str);
	//�ַ����Ƿ��ǿյ�
	static BOOLEAN StrEmeptyA(LPCSTR str);

	static LPCWSTR StrUpW(LPCWSTR str);
	static LPCSTR StrUpA(LPCSTR str);
	static LPCWSTR StrLoW(LPCWSTR str);
	static LPCSTR StrLoA(LPCSTR str);
	static LPCWSTR StrAddW(LPCWSTR str1, LPCWSTR str2);
	static LPCSTR StrAddA(LPCSTR str1, const LPCSTR str2);
	//�ַ����Ƿ����
	static BOOL StrEqualA(LPCSTR str1, LPCSTR str2);
	//�ַ����Ƿ����
	static BOOL StrEqualW(const wchar_t* str1, const wchar_t* str2);
	static LPCSTR IntToStrA(int i);
	static LPCWSTR IntToStrW(int i);
	static LPCSTR LongToStrA(long i);
	static LPCWSTR LongToStrW(long i);
	static int StrToIntA(char * str);
	static int StrToIntW(LPCWSTR str);
	static bool StrToBoolA(LPCSTR str);
	static bool StrToBoolW(LPCWSTR str);
	static DWORD StrSplitA(char * str, LPCSTR splitStr, LPCSTR * result, char ** lead);
	static DWORD StrSplitW(wchar_t * str, const LPCWSTR splitStr, LPCWSTR * result, wchar_t ** lead);
	static BOOL StrContainsA(LPCSTR str, LPCSTR testStr, LPCSTR * resultStr);
	static BOOL StrContainsW(LPCWSTR str, LPCWSTR testStr, LPCWSTR * resultStr);
	static BOOL StrContainsCharA(LPCSTR str, CHAR testStr);
	static BOOL StrContainsCharW(LPCWSTR str, WCHAR testStr);
	static int HexStrToIntW(wchar_t *s);
	static long long HexStrToLongW(wchar_t *s);
private:

};

//��ʽ���ַ��������ڵ�ǰģ���/MD���п����ʱʹ�ã�������ģ����ʹ�� FormatStringPtr
#define FormatString StringHlp::FormatString
//�ַ����Ƿ���ĳ���ַ�
#define StrContainsChar StringHlp::StrContainsCharW
//����ַ����Ƿ��ǿյ�
#define StrEmepty StringHlp::StrEmeptyW
//����ַ����Ƿ��ǿյ�
#define StrEmeptyAnsi StringHlp::StrEmeptyA
//�ַ����Ƿ����
#define StrEqual StringHlp::StrEqualW
//�ַ����Ƿ����A
#define StrEqualAnsi StringHlp::StrEqualA
//խ�ַ�תΪ���ַ�
#define A2W StringHlp::AnsiToUnicode
//���ַ�תΪխ�ַ�
#define W2A StringHlp::UnicodeToAnsi
//��ʽ���ַ��������ص��ַ���ָ����ʹ�� FreeStringPtr �ͷ�
#define FormatStringPtr StringHlp::FormatStringPtrW
//�ͷ��� StringHlp �෵�ص��ַ���ָ��
#define FreeStringPtr(v) StringHlp::FreeStringPtr((LPVOID)v)






