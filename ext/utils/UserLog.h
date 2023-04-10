#pragma once

#include <stdio.h>
#include <stdarg.h>

//LOG LEVELS
typedef enum {
	LOG_DEFAULT,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_DEBUG,
	LOG_DEBUG_LINE, // not use CR
	LOG_DEBUG_CONTINUE, // not use header and CR
}LOG_LEVEL;

//#ifndef LOG_PRINT
//#define LOG_PRINT LOG_PRINT()
//#endif

void LOG_SET_FILEPATH(const char* szFilePath, const char* szFileName);
void LOG_SET_ID(const int id);
void LOG_TRACE(LOG_LEVEL lvl, const char *fmt, ...);
size_t TICK_COUNT();