#pragma once
#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include <list>
#include <string>

//日志级别
enum LogLevel {
	//文字
	LogLevelText,
	//信息
	LogLevelInfo,
	//警告
	LogLevelWarn,
	//错误
	LogLevelError,
	//禁用
	LogLevelDisabled,
};
//日志输出位置
enum LogOutPut {
	//输出到系统默认控制台
	LogOutPutConsolne,
	//输出到文件
	LogOutPutFile,
	//输出到自定义回调
	LogOutPutCallback,
};

typedef void(*LogCallBack)(const wchar_t* str, LogLevel level, void* lparam);

struct LOG_SLA {
	std::wstring str;
	LogLevel level;
};

class Logger;

//获取静态全局日志记录器
EXTERN_C VR720_EXP Logger* GetLoggerStaticInstance();

//日志记录
class VR720_EXP Logger {

public:
	static Logger* GetStaticInstance() { return GetLoggerStaticInstance(); }

	virtual void Log(const wchar_t* str, ...) {}
	virtual void LogWarn(const wchar_t* str, ...) {}
	virtual void LogError(const wchar_t* str, ...) {}
	virtual void LogInfo(const wchar_t* str, ...) {}

	virtual void Log2(const wchar_t* str, const char* file, int line, const char* functon, ...) {}
	virtual void LogWarn2(const wchar_t* str, const  char* file, int line, const char* functon, ...) {}
	virtual void LogError2(const wchar_t* str, const char* file, int line, const char* functon, ...) {}
	virtual void LogInfo2(const wchar_t* str, const char* file, int line, const char* functon, ...) {}

	virtual LogLevel GetLogLevel() { return LogLevel::LogLevelDisabled; }
	virtual void SetLogLevel(LogLevel logLevel) {}
	virtual void SetLogOutPut(LogOutPut output) {}
	virtual void SetLogOutPutCallback(LogCallBack callback, void* lparam) {}
	virtual void SetLogOutPutFile(const wchar_t* filePath) {}

	virtual void ResentNotCaputureLog() {}
	virtual void InitLogConsoleStdHandle() {}
};

#ifdef VR720_EXPORTS

class LoggerInternal : public Logger
{
public:

	static void InitConst();
	static void DestroyConst();

	LoggerInternal(const wchar_t* tag);
	~LoggerInternal();

	void Log(const wchar_t* str, ...);
	void LogWarn(const wchar_t* str, ...);
	void LogError(const wchar_t* str, ...);
	void LogInfo(const wchar_t* str, ...);

	void Log2(const wchar_t* str, const char* file, int line, const char* functon, ...);
	void LogWarn2(const wchar_t* str, const  char* file, int line, const char* functon, ...);
	void LogError2(const wchar_t* str, const char* file, int line, const char* functon, ...);
	void LogInfo2(const wchar_t* str, const char* file, int line, const char* functon, ...);

	LogLevel GetLogLevel();
	void SetLogLevel(LogLevel logLevel);
	void SetLogOutPut(LogOutPut output);
	void SetLogOutPutCallback(LogCallBack callback, void* lparam);
	void SetLogOutPutFile(const wchar_t* filePath);

	void ResentNotCaputureLog();
	void InitLogConsoleStdHandle();

private:
	std::list<LOG_SLA> logPendingBuffer;
	std::wstring logFilePath;
	std::wstring logTag;
	FILE* logFile = nullptr;
	LogLevel level = LogLevelInfo;
	LogOutPut outPut = LogOutPutConsolne;
	LogCallBack callBack = nullptr;
	void* callBackData;
	void* hOutput = NULL;

	void LogOutputToStdHandle(LogLevel logLevel, const wchar_t* str, size_t len);
	void WritePendingLog(const wchar_t* str, LogLevel logLevel);

	void LogInternalWithCodeAndLine(LogLevel logLevel, const wchar_t* str, const char* file, int line, const char* functon, va_list arg);
	void LogInternal(LogLevel logLevel, const wchar_t* str, va_list arg);
	void LogOutput(LogLevel logLevel, const wchar_t* str, const wchar_t* srcStr, size_t len);
	void CloseLogFile();
};

#endif

//快速记录日志

#undef LOG 
#undef LOGI 
#undef LOGW 
#undef LOGE
#undef LOGD

#define LOG Logger::GetStaticInstance()
#define LOGI(fmt) Logger::GetStaticInstance()->LogInfo(fmt)
#define LOGW(fmt) Logger::GetStaticInstance()->LogWarn(fmt)
#define LOGE(fmt) Logger::GetStaticInstance()->LogError(fmt)
#define LOGD(fmt) Logger::GetStaticInstance()->Log(fmt)
#define LOGIF(fmt, ...) Logger::GetStaticInstance()->LogInfo(fmt, __VA_ARGS__)
#define LOGWF(fmt, ...) Logger::GetStaticInstance()->LogWarn(fmt, __VA_ARGS__)
#define LOGEF(fmt, ...) Logger::GetStaticInstance()->LogError(fmt, __VA_ARGS__)
#define LOGDF(fmt, ...) Logger::GetStaticInstance()->Log(fmt, __VA_ARGS__)

#define LogError2(str, ...) LogError2(str, __FILE__, __LINE__, __FUNCTION__,__VA_ARGS__)
#define LogWarn2(str, ...) LogWarn2(str, __FILE__, __LINE__, __FUNCTION__,__VA_ARGS__)
#define LogInfo2(str, ...) LogInfo2(str, __FILE__, __LINE__, __FUNCTION__,__VA_ARGS__)
#define Log2(str, ...) Log2(str, __FILE__, __LINE__, __FUNCTION__,__VA_ARGS__)

