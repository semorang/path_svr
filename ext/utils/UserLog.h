#pragma once

#include <stdio.h>
#include <stdarg.h>

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
void LOG_SET_FILEPATH(const char* szFilePath, const char* szFileName);
void LOG_SET_ID(const int id);
void LOG_SET_LEVLE(const int lvl);
time_t LOG_TRACE(LOG_LEVEL lvl, const char *fmt, ...);
time_t LOG_TRACE_TM(LOG_LEVEL lvl, time_t stm, const char *fmt, ...);
time_t LOG_TRACE_EX(const char* func, const int line, LOG_LEVEL lvl, const char *fmt, ...);
size_t TICK_COUNT();

typedef struct _tagLOGTIME {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int millisecond;
}LOGTIME;

#define LOG_PRINT(lvl, fmt, ...)	LOG_TRACE_EX(__FUNCTION__, __LINE__, lvl, fmt, ##__VA_ARGS__) 