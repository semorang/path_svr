#if defined(_WINDOWS) || (_WIN32)
#include "../stdafx.h"
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <psapi.h> // for mem check
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#else
#include <pthread.h>
#include <math.h>
#include <unistd.h> // sysconf
#include <sys/stat.h>
#include <sys/resource.h>	// getrusage
#endif

#include "UserLog.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <omp.h>
#include <map>
#include <thread>

using namespace std;

// #include <stdio.h>

typedef struct _tagstLogInfo
{
public:
	FILE* fpLog;
	string strPath;
	string strName;
	string strMsg;
	LOGTIME tmNow;

private:
#if defined(_WIN32)
	CRITICAL_SECTION mtxLog;
#else 
	pthread_mutex_t mtxLog;
#endif
	int mutex_try;


public:
	_tagstLogInfo()
	{
		fpLog = nullptr;
		mutex_try = 0;
#if defined(_WIN32)
		InitializeCriticalSection(&mtxLog);
#else
		pthread_mutex_init(&mtxLog, NULL);
#endif
		memset(&tmNow, 0x00, sizeof(tmNow));
	}

	~_tagstLogInfo()
	{
		if (fpLog) {
			fclose(fpLog);
			fpLog = nullptr;
		}
		mutex_try = 0;
#if defined(_WIN32)
		DeleteCriticalSection(&mtxLog);
#else
		pthread_mutex_destroy(&mtxLog);
#endif
	}

	void lock()
	{
#if defined(_WIN32)
		EnterCriticalSection(&mtxLog);
#else
		pthread_mutex_lock(&mtxLog);
#endif
		mutex_try = 0;
	}

	int try_lock()
	{
#if defined(_WIN32)
		if (TryEnterCriticalSection(&mtxLog) == TRUE) {
			return 0;
		}
#else
		if (pthread_mutex_trylock(&mtxLog) == 0) {
			return 0;
		}
#endif
		return ++mutex_try;
	}

	int try_lock_count()
	{
		return mutex_try;
	}

	void unlock()
	{
#if defined(_WIN32)	
		LeaveCriticalSection(&mtxLog);
#else
		pthread_mutex_unlock(&mtxLog);
#endif
		mutex_try = 0;
	}
}stLogInfo;

static const int MAX_MUTEX_TRY = 1000;
static const int MAX_BUFF = 1024;
static int g_nId = 0;
static int g_nPrintLogLevel = LOG_DEFAULT;

typedef map<string, stLogInfo*> mapLogInfo;
mapLogInfo g_mapLogInfo;

thread g_TimeThread;
LOGTIME g_current_time;
bool g_isTimeThreadRun = false;
const char g_LogTimeThreadName[] = "Log time thread";

#if defined(_WIN32)
//static CRITICAL_SECTION g_mutex_log;
#else 
//static pthread_mutex_t g_mutex_log = PTHREAD_MUTEX_INITIALIZER;
#endif

time_t getLogTime(LOGTIME& logTime, time_t tmCurrent)
{
	time_t retEpochTime = 0;

	if (tmCurrent > 0) {
		struct tm* tmNow = localtime(&tmCurrent);

		logTime.year = tmNow->tm_year + 1900;
		logTime.month = tmNow->tm_mon + 1;
		logTime.day = tmNow->tm_mday;
		logTime.hour = tmNow->tm_hour;
		logTime.minute = tmNow->tm_min;
		logTime.second = tmNow->tm_sec;
		logTime.millisecond = 0;

		retEpochTime = tmCurrent * 1000;;
	} else {
#if defined(_WIN32)
#ifndef int64_t
#define int64_t __int64
#endif
		SYSTEMTIME sysNow;
		GetLocalTime(&sysNow);

		logTime.year = sysNow.wYear;
		logTime.month = sysNow.wMonth;
		logTime.day = sysNow.wDay;
		logTime.hour = sysNow.wHour;
		logTime.minute = sysNow.wMinute;
		logTime.second = sysNow.wSecond;
		logTime.millisecond = sysNow.wMilliseconds;

		struct tm tmNow;
		tmNow.tm_year = sysNow.wYear - 1900;
		tmNow.tm_mon = sysNow.wMonth - 1;
		tmNow.tm_mday = sysNow.wDay;
		tmNow.tm_hour = sysNow.wHour;
		tmNow.tm_min = sysNow.wMinute;
		tmNow.tm_sec = sysNow.wSecond;

		retEpochTime = mktime(&tmNow) * 1000 + sysNow.wMilliseconds;
#else
		struct timespec g_specific_time;
		struct tm* tmNow;

		clock_gettime(CLOCK_REALTIME, &g_specific_time);
		tmNow = localtime(&g_specific_time.tv_sec);

		logTime.year = tmNow->tm_year + 1900;
		logTime.month = tmNow->tm_mon + 1;
		logTime.day = tmNow->tm_mday;
		logTime.hour = tmNow->tm_hour;
		logTime.minute = tmNow->tm_min;
		logTime.second = tmNow->tm_sec;
		logTime.millisecond = floor(g_specific_time.tv_nsec / 1.0e6);

		retEpochTime = g_specific_time.tv_sec * 1000 + logTime.millisecond;
#endif
	}

	return retEpochTime;
}


void logtime_thread()
{
	// check every 1minute
	for ( ; g_isTimeThreadRun; ) {
		LOGTIME logNow;
		time_t tEpochTime = getLogTime(logNow, 0);

		if ((g_current_time.year != logNow.year) || (g_current_time.month != logNow.month) || (g_current_time.day != logNow.day)) {
			char szChangeDateLog[PATH_MAX + 1] = { 0, };
#if defined(_WIN32) && defined(_WINDOWS)
			sprintf_s(szChangeDateLog, PATH_MAX, "Log file change: %04d-%02d-%02d => %04d-%02d-%02d", g_current_time.year, g_current_time.month, g_current_time.day, logNow.year, logNow.month, logNow.day);
#else
			snprintf(szChangeDateLog, PATH_MAX, "Log file change date: %04d-%02d-%02d => %04d-%02d-%02d", g_current_time.year, g_current_time.month, g_current_time.day, logNow.year, logNow.month, logNow.day);
#endif

			// update date
			memcpy(&g_current_time, &logNow, sizeof(logNow));

			LOG_TRACE(LOG_DEBUG, szChangeDateLog);

			if (!g_mapLogInfo.empty()) {
				FILE* pFileLog = nullptr;
				stLogInfo* pLogInfo = nullptr;

				for (auto& info : g_mapLogInfo) {
					pLogInfo = info.second;
					pFileLog = pLogInfo->fpLog;

					// make new file
					LOG_SET_FILEPATH(info.first.c_str(), pLogInfo->strPath.c_str(), pLogInfo->strName.c_str());
				}
			}
		}

		// sleep 1min
		this_thread::sleep_for(chrono::minutes(1));

	} // for
}


void log_print_head(const char* szKey, FILE* fpLog, const int lvl, const LOGTIME& logtime)
{
	if (lvl != LOG_DEBUG_CONTINUE) {
		if (fpLog) {
			fprintf(fpLog, "[%02d:%02d:%02d:%03d]", logtime.hour, logtime.minute, logtime.second, logtime.millisecond);
		} else {
			if (szKey && strlen(szKey) > 0) {
				printf("[%04d-%02d-%02d %02d:%02d:%02d:%03d][%s]", logtime.year, logtime.month, logtime.day, logtime.hour, logtime.minute, logtime.second, logtime.millisecond, szKey);
#if defined(_WIN32) && defined(_WINDOWS) && (_MSC_VER >= 1900)
				char szMsg[MAX_BUFF + 1] = { 0, };
				sprintf_s(szMsg, MAX_BUFF, "[%04d-%02d-%02d %02d:%02d:%02d:%03d][%s]", logtime.year, logtime.month, logtime.day, logtime.hour, logtime.minute, logtime.second, logtime.millisecond, szKey);
				OutputDebugStringA(szMsg);
#endif
			} else {
				printf("[%04d-%02d-%02d %02d:%02d:%02d:%03d]", logtime.year, logtime.month, logtime.day, logtime.hour, logtime.minute, logtime.second, logtime.millisecond);
#if defined(_WIN32) && defined(_WINDOWS) && (_MSC_VER >= 1900)
				char szMsg[MAX_BUFF + 1] = { 0, };
				sprintf_s(szMsg, MAX_BUFF, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", logtime.year, logtime.month, logtime.day, logtime.hour, logtime.minute, logtime.second, logtime.millisecond);
				OutputDebugStringA(szMsg);
#endif
			}
		}

		// log id
		if (g_nId > 0) {
			if (fpLog) {
				fprintf(fpLog, "[%d]", g_nId);
			} else {
				printf("[%d]", g_nId);
#if defined(_WIN32) && defined(_WINDOWS) && (_MSC_VER >= 1900)
				char szMsg[MAX_BUFF + 1] = { 0, };
				sprintf_s(szMsg, MAX_BUFF, "[%d]", g_nId);
				OutputDebugStringA(szMsg);
#endif
			}
		}

		// log level
		if (lvl == LOG_ERROR) {
			if (fpLog) {
				fprintf(fpLog, " error: ");
			} else {
				printf(" error: ");
#if defined(_WIN32) && defined(_WINDOWS) && (_MSC_VER >= 1900)
				OutputDebugStringA(" error: ");
#endif
			}
		} else if (lvl == LOG_WARNING) {
			if (fpLog) {
				fprintf(fpLog, " warning: ");
			} else {
				printf(" warning: ");
#if defined(_WIN32) && defined(_WINDOWS) && (_MSC_VER >= 1900)
				OutputDebugStringA(" warning: ");	
#endif
			}
		}
	} // if (lvl != LOG_DEBUG_LINE)

	  // char *s, c;
	  // int i;
	  // double f;
	  // int64_t l;
}


void log_print_body(FILE* fpLog, const int lvl, const char* fmt, va_list args, const char* func, const int line)
{
	//va_list args;
	//va_start(args, fmt);
	if (fpLog) {
		if (func != nullptr) {
			if (line > 0) {
				fprintf(fpLog, "%s(%d)", func, line);
			} else {
				fprintf(fpLog, "%s", func);
			}
		}
		vfprintf(fpLog, fmt, args);
	} else {
		if (func != nullptr) {
			if (line > 0) {
				printf("%s(%d)", func, line);
			} else {
				printf("%s", func);
			}
		}
		vprintf(fmt, args);
#if defined(_WIN32) && defined(_WINDOWS) && (_MSC_VER >= 1900)
		int len = _vscprintf(fmt, args) + 1;
		char* pBuff = (char*)malloc(len * sizeof(char));
		vsprintf_s(pBuff, len, fmt, args);
		OutputDebugStringA(pBuff);
		free(pBuff);
#endif
	}
	//va_end(args);

	// 	//if ((lvl == LOG_INFO) || (lvl == LOG_ERROR))
	// 	{
	// 		va_start(list, fmt);
	//
	// 		if (g_fpFileLog) {
	// #if defined(_WIN32)
	// 		sprintf(g_szMsg, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
	// #else
	// 		sprintf(g_szMsg, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", g_tmNow->tm_year + 1900, g_tmNow->tm_mon + 1, g_tmNow->tm_mday, g_tmNow->tm_hour, g_tmNow->tm_min, g_tmNow->tm_sec, millsec);
	// #endif
	// 		} else {
	// #if defined(_WIN32)
	// 		sprintf(g_szMsg, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
	// #else
	// 		printf("[%04d-%02d-%02d %02d:%02d:%02d:%03d]", g_tmNow->tm_year + 1900, g_tmNow->tm_mon + 1, g_tmNow->tm_mday, g_tmNow->tm_hour, g_tmNow->tm_min, g_tmNow->tm_sec, millsec);
	// #endif
	// 		}
	//
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
	//
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
	//
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
	//
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
	//
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
	//
	// 				default:
	// 					if (g_fpFileLog) {
	// 					} else {
	// 	#if defined(_WIN32)
	//
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
	//
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

void log_print_tail(FILE* fpLog, const int lvl, const int time = 0)
{
	if (lvl != LOG_DEBUG_LINE && lvl != LOG_DEBUG_CONTINUE) {
		if (time != 0) {
			if (fpLog) {
				fprintf(fpLog, ", t:%d\n", time);
				fflush(fpLog);
			} else {
				printf(", t:%d\n", time);
				fflush(stdout);
#if defined(_WIN32) && defined(_WINDOWS) && (_MSC_VER >= 1900)
				char szMsg[MAX_BUFF + 1] = { 0, };
				sprintf_s(szMsg, MAX_BUFF, ", t:%d\n", time);
				OutputDebugStringA(szMsg);		
#endif
			}
		} else {
			if (fpLog) {
				fprintf(fpLog, "\n");
				fflush(fpLog);
			} else {
				printf("\n");
				fflush(stdout);
#if defined(_WIN32) && defined(_WINDOWS) && (_MSC_VER >= 1900)
				OutputDebugStringA("\n");
#endif			
			}
		}
	}
}


void LOG_INITIALIZE(void)
{
	// set default log key
	LOG_SET_KEY(LOG_KEY_BASE);

	// create thread
	if (!g_isTimeThreadRun) {
		g_isTimeThreadRun = true;
		g_TimeThread = thread(logtime_thread);

		LOG_TRACE(LOG_DEBUG, "%s start", g_LogTimeThreadName);
	}
}


void LOG_RELEASE(void)
{
	// release thread
	if (g_isTimeThreadRun) {
		g_isTimeThreadRun = false;

		//g_TimeThread.detach();
		if (g_TimeThread.joinable()) {
#if 1
			g_TimeThread.detach();
#else
			LOG_TRACE(LOG_DEBUG, "%s will stop", g_LogTimeThreadName);
			g_TimeThread.join();
			LOG_TRACE(LOG_DEBUG, "%s end", g_LogTimeThreadName);
#endif
		}
	}

	if (!g_mapLogInfo.empty()) {
		for (auto& info : g_mapLogInfo) {
			delete (info.second);
			info.second = nullptr;
		}
		g_mapLogInfo.clear();
		mapLogInfo().swap(g_mapLogInfo);
	}
}


void LOG_SET_ID(const int id)
{
	g_nId = id;

	LOG_TRACE(LOG_DEBUG, "Log id set, id:%d", g_nId);
}


void LOG_SET_LEVEL(const int lvl)
{
	LOG_TRACE(LOG_DEBUG, "Log level change, %d -> %d", g_nPrintLogLevel, lvl);
	g_nPrintLogLevel = lvl;
}


void LOG_SET_KEY(const char* szKey)
{
	// find same key
	mapLogInfo::iterator itLog = g_mapLogInfo.find(szKey);
	if (itLog != g_mapLogInfo.end()) {
		LOG_TRACE(LOG_WARNING, "exist log key: \'%s\'", szKey);
	} else { // if (itLog == g_mapLogInfo.end()) {
		stLogInfo* pLogInfo = new stLogInfo;
		g_mapLogInfo.emplace(szKey, pLogInfo);
		LOG_TRACE(LOG_DEBUG, "created log key: \'%s\'", szKey);
	}
}


/* LOG_TRACE(log level, format, args ) */
//#define LOG_TRACE(LOG_LEVEL lvl, char *fmt, ...)	LOG_PRINT(lvl, __FUNCTION__, __LINE__, fmt, ...)
//void LOG_PRINT(LOG_LEVEL lvl, const char* strFuc, const int nLine, char *fmt, ...)


void LOG_SET_FILEPATH(const char* szKey, const char* szFilePath, const char* szFileName)
{
	if (!szKey || !szFilePath || !szFileName || strlen(szKey) <= 0 || strlen(szFilePath) <= 0 || strlen(szFileName) <= 0) {
		return;
	}

	stLogInfo* pLogInfo = nullptr;

	// find same key
	mapLogInfo::iterator itLog = g_mapLogInfo.find(szKey);
	if (g_mapLogInfo.empty() || (itLog == g_mapLogInfo.end())) {
		return;
	} else {
		pLogInfo = itLog->second;
	}

	// make new file
	if (checkDirectory(szFilePath) == false) {
		LOG_TRACE(szKey, LOG_ERROR, "Can't create log directory : %s", szFilePath);
		return;
	}

	pLogInfo->lock();

	if ((g_current_time.year <= 0)) {
		getLogTime(g_current_time, 0);
	}

	char szFilePathName[PATH_MAX + 1] = { 0, };
#if defined(_WIN32) && defined(_WINDOWS)
	sprintf_s(szFilePathName, PATH_MAX, "%s/%s_%04d-%02d-%02d.txt", szFilePath, szFileName, g_current_time.year, g_current_time.month, g_current_time.day);
#else
	snprintf(szFilePathName, PATH_MAX, "%s/%s_%04d-%02d-%02d.txt", szFilePath, szFileName, g_current_time.year, g_current_time.month, g_current_time.day);
#endif

	// file close
	if (pLogInfo->fpLog) {
		fflush(pLogInfo->fpLog);
		fclose(pLogInfo->fpLog);
		pLogInfo->fpLog = nullptr;
	}

	// file open
	if (!(pLogInfo->fpLog = fopen(szFilePathName, "at"))) {
		LOG_TRACE(szKey, LOG_ERROR, "Log file open failed, file:%s", szFilePathName);
		pLogInfo->unlock();
		return;
	}

	pLogInfo->strPath = szFilePath;
	pLogInfo->strName = szFileName;
	LOG_TRACE(szKey, LOG_DEBUG, "Log file set, path:%s, file:%s", szFilePath, szFileName);
	LOG_TRACE(szKey, LOG_DEBUG, "Log file open, file:%s", szFilePathName);

	pLogInfo->unlock();
}


time_t LOG_PRINT(const char* szKey, const LOG_LEVEL lvl, const time_t tm, const char* fmt, va_list args, const char* func, const int line)
{
	// get current time
	LOGTIME logNow;
	time_t retEpochTime = getLogTime(logNow, 0);
	if (!szKey || strlen(szKey) <= 0 || g_nPrintLogLevel < lvl || !fmt || strlen(fmt) <= 0) {
		return retEpochTime;
	}

	FILE* pFileLog = nullptr;
	stLogInfo* pLogInfo = nullptr;

	// find same key
	mapLogInfo::iterator itLog = g_mapLogInfo.find(szKey);
	if (!g_mapLogInfo.empty() && (itLog != g_mapLogInfo.end())) {
		pLogInfo = itLog->second;
		pFileLog = pLogInfo->fpLog;
	}

	// lock
	for (; pLogInfo && (pLogInfo->try_lock() != 0); ) {
#if defined(_WIN32)
		Sleep(1); // 1ms
#else
		usleep(1000); // 1ms
#endif

		// try 1 sec
		if (pLogInfo->try_lock_count() >= MAX_MUTEX_TRY) {
			log_print_head(szKey, pFileLog, LOG_WARNING, logNow);
			printf("wait count over than %d times, will return, pid:%d", MAX_MUTEX_TRY, omp_get_thread_num());
			log_print_tail(pFileLog, lvl);

			//va_list args;
			//va_start(args, fmt);
			log_print_body(pFileLog, LOG_WARNING, fmt, args, nullptr, 0);
			printf("\n");
			//va_end(args);

			pLogInfo->unlock();

			return retEpochTime;
		}
	} // for


	  // print head
	log_print_head(szKey, pFileLog, lvl, logNow);


	// print body
	//va_list args;
	//va_start(args, fmt);
	log_print_body(pFileLog, lvl, fmt, args, nullptr, 0);
	//va_end(args);


	// print tail
	if ((tm > 0) && (retEpochTime > tm)) {
		// work time
		log_print_tail(pFileLog, lvl, retEpochTime - tm);
	} else {
		log_print_tail(pFileLog, lvl);
	}


	// unlock
	if (pLogInfo) {
		pLogInfo->unlock();
	}

	return retEpochTime;
}


time_t LOG_TRACE_EX(const char* func, const int line, LOG_LEVEL lvl, const char *fmt, ...)
{
	time_t tEpochTime = 0;

	if (g_nPrintLogLevel < lvl) {
		return tEpochTime;
	}
}


time_t LOG_TRACE(const LOG_LEVEL lvl, const char *fmt, ...)
{
	const char szKey[] = "base";

	va_list args;
	va_start(args, fmt);
	time_t retEpochTime = LOG_PRINT(szKey, lvl, 0, fmt, args);
	va_end(args);

	return retEpochTime;
}


time_t LOG_TRACE(const LOG_LEVEL lvl, const time_t tm, const char *fmt, ...)
{
	const char szKey[] = "base";

	va_list args;
	va_start(args, fmt);
	time_t retEpochTime = LOG_PRINT(szKey, lvl, tm, fmt, args);
	va_end(args);

	return retEpochTime;
}


time_t LOG_TRACE(const char* szKey, const LOG_LEVEL lvl, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	time_t retEpochTime = LOG_PRINT(szKey, lvl, 0, fmt, args);
	va_end(args);

	return retEpochTime;
}


time_t LOG_TRACE(const char* szKey, const LOG_LEVEL lvl, const time_t tm, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	time_t retEpochTime = LOG_PRINT(szKey, lvl, tm, fmt, args);
	va_end(args);

	return retEpochTime;
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


bool checkDirectory(const char* szFileName)
{
	// path check
	//char* pTok = strrchr((char*)szFileName, '/');
	//if (pTok != nullptr) 
	//{
	//	char szPath[MAX_PATH] = { 0, };
	//	strncpy(szPath, szFileName, (pTok - szFileName));
	bool isCreate = false;
#if defined(_WIN32)
		// check directory created
		if (_access_s(szFileName, 0) != 0) {
			// LOG_TRACE(LOG_DEBUG, "directory was not created : %s", szPath);

			if (_mkdir(szFileName) != 0) {
				// LOG_TRACE(LOG_ERROR, "Error, Can't create directory : %s", szPath);
				return false;
			}
			isCreate = true;
		}
#else
		// check directory created
		if (access(szFileName, W_OK) != 0) {
			// LOG_TRACE(LOG_DEBUG, "save directory was not created : %s", szPath);

			if (mkdir(szFileName, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
				// LOG_TRACE(LOG_ERROR, "Error, Can't create save directory : %s", szPath);
				return false;
			}
			isCreate = true;
			
		}
#endif
		if (isCreate) {
			LOG_TRACE(LOG_DEBUG, "created directory: %s", szFileName);
		}
	//}

		return true;
}


size_t checkMemorySize(void)
{
	size_t physMemUsed = 0;
#if defined(_WIN32)
	PROCESS_MEMORY_COUNTERS memInfo;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &memInfo, sizeof(memInfo))) {
		physMemUsed = memInfo.WorkingSetSize;
	}
#else
	// 1) /proc/self/statm : 2번째 필드(resident pages) × 페이지 크기
	{
		FILE* f = fopen("/proc/self/statm", "r");
		if (f) {
			long pages = 0, dummy = 0;
			if (fscanf(f, "%ld %ld", &dummy, &pages) == 2) {
				long pageSize = sysconf(_SC_PAGESIZE); // bytes/page
				if (pageSize > 0 && pages >= 0) {
					physMemUsed = static_cast<size_t>(pages) * static_cast<size_t>(pageSize);
				}
			}
			fclose(f);
		}
	}

	// 2) 보조 경로: /proc/self/status 의 "VmRSS:" (단위 kB)
	if (physMemUsed == 0) {
		FILE* f = fopen("/proc/self/status", "r");
		if (f) {
			char line[256];
			while (fgets(line, sizeof(line), f)) {
				if (strncmp(line, "VmRSS:", 6) == 0) {
					unsigned long kb = 0;
					// 예: "VmRSS:   12345 kB"
					if (sscanf(line + 6, "%lu", &kb) == 1) {
						physMemUsed = static_cast<size_t>(kb) * 1024u; // bytes
						break;
					}
				}
			}
			fclose(f);
		}
	}

	// 3) 최후 수단: getrusage (Linux에선 kB 단위의 "최대" RSS, 현재값 아님)
	if (physMemUsed == 0) {
		struct rusage ru {};
		if (getrusage(RUSAGE_SELF, &ru) == 0) {
			physMemUsed = static_cast<size_t>(ru.ru_maxrss) * 1024u; // bytes (Linux: kB)
		}
	}
#endif

	return physMemUsed;
}


time_t LOG_TIME(LOGTIME& logTime, time_t tmCurrent)
{ 
	return getLogTime(logTime, tmCurrent);
}