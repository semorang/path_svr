#if defined(_WINDOWS) || (_WIN32)
#include "../stdafx.h"
#include <windows.h>
#include <direct.h>
#include <io.h>
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#else
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "UserLog.h"
#include <time.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
// #include <stdio.h>

static bool g_bUseFileLog = false;
static FILE* g_fpFileLog = nullptr;
static char g_szFilePath[PATH_MAX] = {0,};
static char g_szFileName[PATH_MAX] = {0,};
static char g_szMsg[PATH_MAX] = { 0, };
static char g_szBuff[PATH_MAX] = { 0, };
static int g_nId = 0;

#if defined(_WIN32)
#ifndef int64_t
#define int64_t __int64
#endif
static SYSTEMTIME g_tmCurrent = {0,};
#else
static struct tm g_tmCurrent = {0,};
#endif

void LOG_SET_ID(const int id)
{
	g_nId = id;

	LOG_TRACE(LOG_DEBUG, "Log id set, id:%d", g_nId);
}

void LOG_SET_FILEPATH(const char* szFilePath, const char* szFileName)
{
	if (!szFilePath || !szFileName || strlen(szFilePath) <= 0 || strlen(szFileName) <= 0) {
		return;
	}


#if defined(_WIN32)
	// SYSTEMTIME st;
	SYSTEMTIME tmNow;
	// GetSystemTime(&st);
	GetLocalTime(&tmNow);
#else
	// time_t timer = time(NULL);
	// struct tm* tmNow = localtime(&timer);
	struct timespec specific_time;
	struct tm* tmNow;
	clock_gettime(CLOCK_REALTIME, &specific_time);
	tmNow = localtime(&specific_time.tv_sec);
	// int millsec = specific_time.tv_nsec;
	// millsec = floor(specific_time.tv_nsec / 1.0e6);
#endif




	// make new file
#if defined(_WIN32)
	if ((g_tmCurrent.wYear != tmNow.wYear ||
		g_tmCurrent.wMonth != tmNow.wMonth ||
		g_tmCurrent.wDay != tmNow.wDay))
#else
	if ((g_tmCurrent.tm_year != tmNow->tm_year ||
		g_tmCurrent.tm_mon != tmNow->tm_mon ||
		g_tmCurrent.tm_mday != tmNow->tm_mday))
#endif
	{
#if defined(_WIN32)
		// check directory created
		if (_access_s(szFilePath, 0) != 0) {
			if (_mkdir(szFilePath) != 0) {
				LOG_TRACE(LOG_ERROR, "Error, Can't create log directory : %s", szFilePath);
				return;
			}
		}

		// update date
		memcpy(&g_tmCurrent, &tmNow, sizeof(g_tmCurrent));
#else
		// check directory created
		if (access(szFilePath, W_OK) != 0) {
			if (mkdir(szFilePath, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
				LOG_TRACE(LOG_ERROR, "Error, Can't create log directory : %s", szFilePath);
				return;
			}
		}

		// update date
		memcpy(&g_tmCurrent, tmNow, sizeof(g_tmCurrent));
#endif

		char szFilePathName[PATH_MAX] = {0,};
#if defined(_WIN32)
		sprintf(szFilePathName, "%s/%s_%04d-%02d-%02d.txt", szFilePath, szFileName, tmNow.wYear, tmNow.wMonth, tmNow.wDay);
#else
		sprintf(szFilePathName, "%s/%s_%04d-%02d-%02d.txt", szFilePath, szFileName, tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday);
#endif

		// file close
		if (g_fpFileLog) {
			fflush(g_fpFileLog);
			fclose (g_fpFileLog);
			g_fpFileLog = nullptr;
		}
		

		// file open
		if (!(g_fpFileLog = fopen(szFilePathName, "at"))) {
			LOG_TRACE(LOG_ERROR, "Error, Log file open, file:%s", szFilePathName);
			return;
		}

		strcpy(g_szFilePath, szFilePath);
		strcpy(g_szFileName, szFileName);

		LOG_TRACE(LOG_DEBUG, "Log file set, path:%s, file:%s", g_szFilePath, g_szFileName);


		g_bUseFileLog = true;

		LOG_TRACE(LOG_DEBUG, "Log file open, file:%s", szFilePathName);
	}
}

/* LOG_TRACE(log level, format, args ) */
//#define LOG_TRACE(LOG_LEVEL lvl, char *fmt, ...)	LOG_PRINT(lvl, __FUNCTION__, __LINE__, fmt, ...)
//void LOG_PRINT(LOG_LEVEL lvl, const char* strFuc, const int nLine, char *fmt, ...)
void LOG_TRACE(LOG_LEVEL lvl, const char *fmt, ...)
{
#if defined(_WIN32)
#ifndef int64_t
#define int64_t __int64
#endif
	SYSTEMTIME tmNow;
	//SYSTEMTIME st;
	// GetSystemTime(&st);
	GetLocalTime(&tmNow);
	char szBuff[MAX_PATH] = {0,};
#else
	// time_t timer = time(NULL);
	// struct tm* tmNow = localtime(&timer);
	struct timespec specific_time;
	struct tm* tmNow;
	clock_gettime(CLOCK_REALTIME, &specific_time);
	tmNow = localtime(&specific_time.tv_sec);
	int millsec = specific_time.tv_nsec;
	millsec = floor(specific_time.tv_nsec / 1.0e6);
#endif


	// check date
	if (g_bUseFileLog && 
#if defined(_WIN32)
		(g_tmCurrent.wYear != tmNow.wYear || g_tmCurrent.wMonth != tmNow.wMonth || g_tmCurrent.wDay != tmNow.wDay))
#else
		(g_tmCurrent.tm_year != tmNow->tm_year || g_tmCurrent.tm_mon != tmNow->tm_mon || g_tmCurrent.tm_mday != tmNow->tm_mday))
#endif
	{
		char szChangeDateLog[PATH_MAX] = {0,};
		sprintf(szChangeDateLog, "Log file change date: %04d-%02d-%02d => %04d-%02d-%02d",
#if defined(_WIN32)
		g_tmCurrent.wYear, g_tmCurrent.wMonth, g_tmCurrent.wDay, tmNow.wYear, tmNow.wMonth, tmNow.wDay);
#else
		g_tmCurrent.tm_year, g_tmCurrent.tm_mon, g_tmCurrent.tm_mday, tmNow->tm_year, tmNow->tm_mon, tmNow->tm_mday);
#endif

		LOG_SET_FILEPATH(g_szFilePath, g_szFileName);

		LOG_TRACE(LOG_DEBUG, szChangeDateLog);
	}


	if (lvl == LOG_DEBUG_CONTINUE) {
		if (g_bUseFileLog && g_fpFileLog) {
			//g_fpFileLog[0] = '\0';
		}
		else {
			g_szMsg[0] = '\0';
			szBuff[0] = '\0';
		}
	} else {
		if (g_bUseFileLog && g_fpFileLog) {
#if defined(_WIN32)
			fprintf(g_fpFileLog, "[%02d:%02d:%02d:%03d]", tmNow.wHour, tmNow.wMinute, tmNow.wSecond, tmNow.wMilliseconds);
#else
			fprintf(g_fpFileLog, "[%02d:%02d:%02d:%03d]", tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec, millsec);
#endif
		}
		else {
#if defined(_WIN32)
			sprintf(g_szMsg, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", tmNow.wYear, tmNow.wMonth, tmNow.wDay, tmNow.wHour, tmNow.wMinute, tmNow.wSecond, tmNow.wMilliseconds);
#else
			printf("[%04d-%02d-%02d %02d:%02d:%02d:%03d]", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec, millsec);
#endif
		}

		// log id
		if (g_nId > 0) {
			if (g_bUseFileLog && g_fpFileLog) {
				fprintf(g_fpFileLog, "[%d]", g_nId);
			}
			else {
#if defined(_WIN32)
				sprintf(szBuff, "[%d]", g_nId);
				strcat(g_szMsg, szBuff);
#else
				printf("[%d]", g_nId);
#endif
			}
		}

		// log level
		if (lvl == LOG_ERROR) {
			if (g_bUseFileLog && g_fpFileLog) {
				fprintf(g_fpFileLog, " error: ");
			}
			else {
#if defined(_WIN32)
				strcat(g_szMsg, " error: ");
#else
				printf(" error: ");
#endif
			}
		}
		else if (lvl == LOG_WARNING) {
			if (g_bUseFileLog && g_fpFileLog) {
				fprintf(g_fpFileLog, " warning: ");
			}
			else {
#if defined(_WIN32)
				strcat(g_szMsg, " warning: ");
#else
				printf(" warning: ");
#endif
			}
		}
	} // if (lvl != LOG_DEBUG_LINE)

	va_list  list;
	// char *s, c;
	// int i;
	// double f;
	// int64_t l;

	va_start(list, fmt);
	if (g_bUseFileLog && g_fpFileLog) {
		vfprintf(g_fpFileLog, fmt, list);
	} else {
#if defined(_WIN32)
		vsprintf(szBuff, fmt, list);
		strcat(g_szMsg, szBuff);
#else
		vprintf(fmt, list);
#endif
	}
	va_end(list);


	if (g_bUseFileLog && g_fpFileLog) {
		if (lvl != LOG_DEBUG_LINE && lvl != LOG_DEBUG_CONTINUE) {
			fprintf(g_fpFileLog, "\n");
		}
		fflush(g_fpFileLog);
	} else {
#if defined(_WIN32)
		if (lvl != LOG_DEBUG_LINE && lvl != LOG_DEBUG_CONTINUE) {
			strcat(g_szMsg, "\n");
		}
	#if _MSC_VER >= 1900
		printf(g_szMsg);
		OutputDebugStringA(g_szMsg);
	#else
		printf(g_szMsg);
	#endif
#else
		if (lvl != LOG_DEBUG_LINE && lvl != LOG_DEBUG_CONTINUE) {
			printf("\n");
		}
		fflush(stdout);
#endif
	}


// 	//if ((lvl == LOG_INFO) || (lvl == LOG_ERROR))
// 	{
// 		va_start(list, fmt);

// 		if (g_fpFileLog) {
// #if defined(_WIN32)
// 		sprintf(g_szMsg, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
// #else
// 		sprintf(g_szMsg, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec, millsec);
// #endif
// 		} else {
// #if defined(_WIN32)
// 		sprintf(g_szMsg, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
// #else
// 		printf("[%04d-%02d-%02d %02d:%02d:%02d:%03d]", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec, millsec);
// #endif
// 		}

// 		while (*fmt)
// 		{
// 			if (*fmt != '%')
// 			{
// 				if (g_fpFileLog) {
// 					g_szMsg[strlen(g_szMsg)] = *fmt;
// 				} else {
// #if defined(_WIN32)
// 					g_szMsg[strlen(g_szMsg)] = *fmt;
// #else
// 					putc(*fmt, stdout);
// #endif
// 				}
// 			}
// 			else
// 			{
// 				switch (*++fmt)
// 				{
// 				case 's':
// 					/* set r as the next char in list (string) */
// 					s = va_arg(list, char *);
// 					if (g_fpFileLog) {
// 						strcat(g_szMsg, s);
// 					} else {
// #if defined(_WIN32)
// 						strcat(g_szMsg, s);
// #else
// 						printf("%s", s);
// #endif
// 					}
// 					break;

// 				case 'd':
// 					i = va_arg(list, int);
// 					if (g_fpFileLog) {
// 						sprintf(g_szBuff, "%d", i);
// 						strcat(g_szMsg, g_szBuff);
// 					} else {
// #if defined(_WIN32)
// 						sprintf(g_szBuff, "%d", i);
// 						strcat(g_szMsg, g_szBuff);
// #else
// 						printf("%d", i);
// #endif
// 					}
// 					break;

// 				case 'f':
// 					f = va_arg(list, double);
// 					if (g_fpFileLog) {
// 						sprintf(g_szBuff, "%f", f);
// 						strcat(g_szMsg, g_szBuff);
// 					} else {
// #if defined(_WIN32)
// 						sprintf(g_szBuff, "%f", f);
// 						strcat(g_szMsg, g_szBuff);
// #else
// 						printf("%f", f);
// #endif
// 					}
// 					break;

// 				case 'l':
// 					if ((*(fmt + 1)) == '6' && (*(fmt + 2)) == '4' && (*(fmt + 2)) == 'd')
// 						fmt += 3;
// 					else if ((*(fmt + 1)) == 'l' && (*(fmt + 2)) == 'd')
// 						fmt += 2;
// 					else if ((*(fmt + 2)) == 'd')
// 						++fmt;
// 					l = va_arg(list, int64_t);
// 					if (g_fpFileLog) {
// 						sprintf(g_szBuff, "%lld", l);
// 						strcat(g_szMsg, g_szBuff);
// 					} else {
// #if defined(_WIN32)
// 						sprintf(g_szBuff, "%lld", l);
// 						strcat(g_szMsg, g_szBuff);
// #else
// 						printf("%lld", l);
// #endif
// 					}
// 					break;

// 				case 'c':
// 					c = va_arg(list, int);
// 					if (g_fpFileLog) {
// 						sprintf(g_szBuff, "%c", c);
// 						strcat(g_szMsg, g_szBuff);
// 					} else {
// #if defined(_WIN32)
// 						sprintf(g_szBuff, "%c", c);
// 						strcat(g_szMsg, g_szBuff);
// #else
// 						printf("%c", c);
// #endif
// 					}
// 					break;

// 				default:
// 					if (g_fpFileLog) {
// 					} else {
// 	#if defined(_WIN32)

// 	#else
// 						putc(*fmt, stdout);
// 	#endif
// 					}
// 					break;
// 				}
// 			}
// 			++fmt;
// 		}
// 		va_end(list);
// 	}

// 	if (g_fpFileLog) {
// 		strcat(g_szMsg, "\n");
// 		fwrite(g_szMsg, strlen(g_szMsg), 1, g_fpFileLog);
// 	} else {
// #if defined(_WIN32)
// 	strcat(g_szMsg, "\n");
// 	#if _MSC_VER > 1900
// 		printf(g_szMsg);
// 		OutputDebugStringA(g_szMsg);
// 	#else
// 		printf(g_szMsg);
// 	#endif
// #else
// 	printf("\n");
// 	fflush(stdout);
// #endif
// 	}
}

size_t TICK_COUNT()
{
#if defined	_WIN32
	return GetTickCount();
#elif defined __MACH__
	// Mac/iPhone OS
	static mach_timebase_info_data_t sTimebaseInfo;
	static uint64_t	ullTickBase = 0;
	if (sTimebaseInfo.denom == 0)
	{
		(void)mach_timebase_info(&sTimebaseInfo);
	}

	const uint64_t current_absolute = mach_absolute_time();
	const double dTime = (double)sTimebaseInfo.numer / (sTimebaseInfo.denom * 1000000.0);
	uint64_t current_millisec = current_absolute * dTime;

	if (ullTickBase == 0)
	{
		ullTickBase = current_millisec;
	}
	current_millisec -= ullTickBase;

	return (uint32_t)(0xFFFFFFFF & current_millisec);
#else
	// 	timeval	tv;
	// 	gettimeofday(&tv, NULL);
	// 	return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);

	struct timespec now;

	if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
		return 0;

	return now.tv_sec * 1000 + now.tv_nsec / 1000000;

	//struct timeb s_time;
	//long time2;
	//long time3;
	//
	//ftime(&s_time);
	//time2 = s_time.time >> 20;
	//time3 = s_time.time - (time2 << 20);
	//
	//return time3 * 1000 + s_time.millitm;
#endif
}