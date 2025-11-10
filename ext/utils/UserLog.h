#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

//LOG LEVELS
typedef enum {
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG_LINE, // not use CR
	LOG_DEBUG_CONTINUE, // not use header and CR
	LOG_DEBUG,
	LOG_DEFAULT, // default log print level
	LOG_TEST,
}LOG_LEVEL;

//#ifndef LOG_PRINT
//#define LOG_PRINT LOG_PRINT()
//#endif
void LOG_INITIALIZE(void);
void LOG_RELEASE(void);
void LOG_SET_ID(const int id);
void LOG_SET_LEVEL(const int lvl);
void LOG_SET_KEY(const char* szKey);
void LOG_SET_FILEPATH(const char* szKey, const char* szFilePath, const char* szFileName);

time_t LOG_TRACE(const LOG_LEVEL lvl, const char *fmt, ...);
time_t LOG_TRACE(const LOG_LEVEL lvl, const time_t stm, const char *fmt, ...);
time_t LOG_TRACE_EX(const char* func, const int line, LOG_LEVEL lvl, const char *fmt, ...);
time_t LOG_TRACE(const char* szKey, const LOG_LEVEL lvl, const char *fmt, ...);
time_t LOG_TRACE(const char* szKey, const LOG_LEVEL lvl, const time_t stm, const char *fmt, ...);

size_t TICK_COUNT();

time_t LOG_PRINT(const char* szKey, const LOG_LEVEL lvl, const time_t tm, const char* fmt, va_list args, const char* func = nullptr, const int line = -1);

bool checkDirectory(const char* szFileName);
size_t checkMemorySize(void);

typedef struct _tagLOGTIME {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int millisecond;
}LOGTIME;

time_t LOG_TIME(LOGTIME& logTime, time_t tmCurrent = 0);

#define LOG_TRACE_DEBUG(lvl, fmt, ...)	LOG_TRACE_EX(__FUNCTION__, __LINE__, lvl, fmt, ##__VA_ARGS__) 

#define LOG_KEY_BASE "base"
#define LOG_KEY_TRAFFIC "traffic"