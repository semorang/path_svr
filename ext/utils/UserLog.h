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
void LOG_TRACE(LOG_LEVEL lvl, const char *fmt, ...);
size_t TICK_COUNT();