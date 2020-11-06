#include "Logger.h"
#include "CStringHlp.h"
#include <ctime>

using namespace std;

#undef LogError2
#undef LogWarn2
#undef LogInfo2
#undef Log2

Logger* globalStaticLogger = nullptr;

 void LoggerInternal::InitConst() { globalStaticLogger = new LoggerInternal(L"App"); }
 void LoggerInternal::DestroyConst() { delete globalStaticLogger; }

 Logger* GetLoggerStaticInstance()
 {
	 return globalStaticLogger;
 }

LoggerInternal::LoggerInternal(const wchar_t* tag)
{
	logTag = tag;
}
LoggerInternal::~LoggerInternal()
{
	CloseLogFile();
}

void LoggerInternal::Log(const wchar_t* str, ...)
{
	if (level <= LogLevelText) {
		va_list arg;
		va_start(arg, str);
		LogInternal(LogLevelText, str, arg);
		va_end(arg);
	}
}
void LoggerInternal::LogWarn(const wchar_t* str, ...)
{
	if (level <= LogLevelWarn) {
		va_list arg;
		va_start(arg, str);
		LogInternal(LogLevelWarn, str, arg);
		va_end(arg);
	}
}
void LoggerInternal::LogError(const wchar_t* str, ...)
{
	if (level <= LogLevelError) {
		va_list arg;
		va_start(arg, str);
		LogInternal(LogLevelError, str, arg);
		va_end(arg);
	}
}
void LoggerInternal::LogInfo(const wchar_t* str, ...)
{
	if (level <= LogLevelInfo) {
		va_list arg;
		va_start(arg, str);
		LogInternal(LogLevelInfo, str, arg);
		va_end(arg);
	}
}

void LoggerInternal::Log2(const wchar_t* str, const char* file, int line, const char* functon, ...)
{
	if (level <= LogLevelText) {
		va_list arg;
		va_start(arg, functon);
		LogInternalWithCodeAndLine(LogLevelText, str, file, line, functon, arg);
		va_end(arg);
	}
}
void LoggerInternal::LogWarn2(const wchar_t* str, const char* file, int line, const char* functon, ...)
{
	if (level <= LogLevelWarn) {
		va_list arg;
		va_start(arg, functon);
		LogInternalWithCodeAndLine(LogLevelWarn, str, file, line, functon, arg);
		va_end(arg);
	}
}
void LoggerInternal::LogError2(const wchar_t* str, const char* file, int line, const char* functon, ...)
{
	if (level <= LogLevelError) {
		va_list arg;
		va_start(arg, functon);
		LogInternalWithCodeAndLine(LogLevelError, str, file, line, functon, arg);
		va_end(arg);
	}
}
void LoggerInternal::LogInfo2(const wchar_t* str, const char* file, int line, const  char* functon, ...)
{
	if (level <= LogLevelInfo) {
		va_list arg;
		va_start(arg, functon);
		LogInternalWithCodeAndLine(LogLevelInfo, str, file, line, functon, arg);
		va_end(arg);
	}
}

void LoggerInternal::SetLogLevel(LogLevel logLevel)
{
	this->level = logLevel;
}
LogLevel LoggerInternal::GetLogLevel() {
	return this->level;
}
void LoggerInternal::SetLogOutPut(LogOutPut output)
{
	this->outPut = output;
}
void LoggerInternal::SetLogOutPutFile(const wchar_t* filePath)
{
	if (logFilePath != filePath)
	{
		CloseLogFile();
		logFilePath = filePath;
#if defined(_MSC_VER) && _MSC_VER > 1600
		_wfopen_s(&logFile, (wchar_t*)logFilePath.data(), L"w");
#else
		logFile = _wfopen(logFilePath.data(), L"w");
#endif
	}
}
void LoggerInternal::SetLogOutPutCallback(LogCallBack callback, void* lparam)
{
	callBack = callback;
	callBackData = lparam;
}

void LoggerInternal::InitLogConsoleStdHandle() {
	hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
}
void LoggerInternal::LogOutputToStdHandle(LogLevel logLevel, const wchar_t* str, size_t len) {
	switch (logLevel)
	{
	case LogLevelInfo:
		SetConsoleTextAttribute(hOutput, FOREGROUND_INTENSITY | FOREGROUND_BLUE);
		break;
	case LogLevelWarn:
		SetConsoleTextAttribute(hOutput, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		break;
	case LogLevelError:
		SetConsoleTextAttribute(hOutput, FOREGROUND_INTENSITY | FOREGROUND_RED);
		break;
	case LogLevelText:
		SetConsoleTextAttribute(hOutput, FOREGROUND_INTENSITY | FOREGROUND_RED |
			FOREGROUND_GREEN |
			FOREGROUND_BLUE);
		break;
	}
	WriteConsoleW(hOutput, str, len, NULL, NULL);
}

void LoggerInternal::ResentNotCaputureLog()
{
	if (outPut == LogOutPutCallback && callBack) {
		std::list< LOG_SLA>::iterator i;
		for (i = logPendingBuffer.begin(); i != logPendingBuffer.end(); i++)
			callBack((*i).str.c_str(), (*i).level, callBackData);
		logPendingBuffer.clear();
	}
}
void LoggerInternal::WritePendingLog(const wchar_t* str, LogLevel logLevel)
{
	LOG_SLA sla = { std::wstring(str), logLevel };
	logPendingBuffer.push_back(sla);
}

void LoggerInternal::LogInternalWithCodeAndLine(LogLevel logLevel, const wchar_t* str, const char* file, int line, const char* functon, va_list arg)
{
	std::wstring format1 = CStringHlp::FormatString(L"%s\n[In] %hs (%d) : %hs", str, file, line, functon);
	LogInternal(logLevel, format1.c_str(), arg);
}
void LoggerInternal::LogInternal(LogLevel logLevel, const wchar_t* str, va_list arg)
{
	const wchar_t* levelStr;
	switch (logLevel)
	{
	case LogLevelInfo: levelStr = L"I"; break;
	case LogLevelWarn: levelStr = L"W"; break;
	case LogLevelError: levelStr = L"E"; break;
	case LogLevelText: levelStr = L"T"; break;
	default: levelStr = L""; break;
	}
	time_t time_log = time(NULL);
#if defined(_MSC_VER) && _MSC_VER > 1600
	struct tm tm_log;
	localtime_s(&tm_log, &time_log);
	std::wstring format1 = CStringHlp::FormatString(L"[%02d:%02d:%02d] [%s] %s\n", tm_log.tm_hour, tm_log.tm_min, tm_log.tm_sec, levelStr, str);
#else
	tm* tm_log = localtime(&time_log);
	std::wstring format1 = CStringHlp::FormatString(L"%s/%s:%02d:%02d:%02d %s\n", logTag.c_str(), levelStr, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec, str);
#endif
	std::wstring out = CStringHlp::FormatString(format1.c_str(), arg);
	LogOutput(logLevel, out.c_str(), str, out.size());
}
void LoggerInternal::LogOutput(LogLevel logLevel, const wchar_t* str, const wchar_t* srcStr, size_t len)
{
#if _DEBUG
	OutputDebugString(str);
#else
	if (outPut == LogOutPutConsolne)
		OutputDebugString(str);
#endif
	if (outPut == LogOutPutFile && logFile)
		fwprintf_s(logFile, L"%s", str);
	else if (outPut == LogOutPutConsolne) {
		if (hOutput != NULL) LogOutputToStdHandle(logLevel, str, len);
		else wprintf_s(L"%s", str);
	}
	else if (outPut == LogOutPutCallback && callBack)
		callBack(str, logLevel, callBackData);
	else
		WritePendingLog(str, logLevel);
}
void LoggerInternal::CloseLogFile()
{
	if (logFile) {
		fclose(logFile);
		logFile = nullptr;
	}
}

