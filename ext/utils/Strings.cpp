#if defined(_WIN32)
#include <windows.h>
#include <wchar.h>
#	ifndef _WINDOWS
#		define _WINDOWS
#	endif
#else
#include <iconv.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "Strings.h"


#if defined(_WIN32)
int MultiByteToUnicode(const char* strMultibyte, wchar_t* strUnicode)
{
	int nLen = 0;
	if (strMultibyte && strUnicode) {
#ifdef _WINDOWS
		nLen = MultiByteToWideChar(CP_ACP, 0, strMultibyte, strlen(strMultibyte), NULL, NULL);
		MultiByteToWideChar(CP_ACP, 0, strMultibyte, strlen(strMultibyte), strUnicode, nLen);
#else
		mbstowcs(strUnicode, strMultibyte, strlen(strMultibyte));
#endif        
	}
	return nLen;
}

int UTF8ToUnicode(const char* strMultibyte, wchar_t* strUnicode)
{
	int nLen = 0;
	if (strMultibyte && strUnicode) {
#ifdef _WINDOWS
		nLen = MultiByteToWideChar(CP_UTF8, 0, strMultibyte, strlen(strMultibyte), NULL, NULL);
		MultiByteToWideChar(CP_UTF8, 0, strMultibyte, strlen(strMultibyte), strUnicode, nLen);
#else
		mbstowcs(strUnicode, strMultibyte, strlen(strMultibyte));
#endif        
	}
	return nLen;
}

int UnicodeToUTF8(const wchar_t* strUnicode, char* strUTF8)
{
	int nLen = 0;
	if (strUnicode && strUTF8) {
#ifdef _WINDOWS
		nLen = WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), strUTF8, nLen, NULL, NULL);
#else
		wcstombs(strUnicode, strUTF8, strlen(strUTF8));
#endif
	}
	return nLen;
}

int MultiByteToUTF8(const char* strMultibyte, char* strUTF8)
{
	int nLen = 0;
	if (strMultibyte && strUTF8) {
		wchar_t wszUnicode[MAX_PATH] = { 0, };

		MultiByteToUnicode(strMultibyte, wszUnicode);
		nLen = UnicodeToUTF8(wszUnicode, strUTF8);
	}
	return nLen;
}

int UnicodeToMultiByte(const wchar_t* strUnicode, char* strMultibyte)
{
	int nLen = 0;
	if (strUnicode && strMultibyte) {
#ifdef _WINDOWS
		nLen = WideCharToMultiByte(CP_ACP, 0, strUnicode, lstrlenW(strUnicode), NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, strUnicode, lstrlenW(strUnicode), strMultibyte, nLen, NULL, NULL);
#else
		wcstombs(strUnicode, strMultibyte, strlen(strMultibyte));
#endif
	}
	return nLen;
}

int UnicodeToMultiByte(const wchar_t* strUnicode, std::string& out)
{
	int written = 0;

	out.clear();
	if (!strUnicode) return written;

#ifdef _WIN32
	int wlen = (int)wcslen(strUnicode);
	if (wlen == 0) { out.clear(); return written; }

	// 필요한 바이트 수(+1 널 포함) 구하기
	int nBytes = WideCharToMultiByte(CP_ACP, 0, strUnicode, wlen, NULL, 0, NULL, NULL);
	if (nBytes <= 0) return written;

	// out에 정확한 크기 확보 (널 포함해서 받기 위해 +1로 받고, 마지막 널은 제거)
	out.resize(nBytes);
	written = WideCharToMultiByte(CP_ACP, 0, strUnicode, wlen, &out[0], nBytes, NULL, NULL);
	if (written <= 0) { out.clear(); return written; }

	// WideCharToMultiByte는 (입력 길이를 wlen으로 주면) 널을 자동으로 안 넣을 수 있어
	// out는 "written" 길이만 유효
	out.resize(written);
#else
	// POSIX: wcstombs는 현재 locale에 의존함
	size_t need = wcstombs(nullptr, strUnicode, 0);
	if (need == (size_t)-1) return written;

	out.resize(need);
	written = wcstombs(&out[0], strUnicode, need + 1);
	if (written == (size_t)-1) { out.clear(); return written; }
	out.resize(written);
#endif
	return written;
}

int UTF8ToMultiByte(const char* strUTF8, char* strMultibyte)
{
	int nLen = 0;
	if (strUTF8 && strMultibyte) {
		wchar_t wszUnicode[MAX_PATH] = { 0, };

		UTF8ToUnicode(strUTF8, wszUnicode);
		nLen = UnicodeToMultiByte(wszUnicode, strMultibyte);
	}
	return nLen;
}


char* strsep(char** stringp, const char* delim)
{
	char* start = *stringp;
	char* p;

	p = (start != NULL) ? strpbrk(start, delim) : NULL;

	if (p == NULL)
	{
		*stringp = NULL;
	}
	else
	{
		*p = '\0';
		*stringp = p + 1;
	}

	return start;
}
#endif


// return value: 변환된 문자열 길이 (바이트 단위, '\0' 제외)
// 0 = 실패 또는 빈 문자열
int encoding(const char* input,
                const char* source,
                const char* target,
                char* output,
                int outputSize)
{
	// 공통 반환값 (실제 쓴 바이트 수, '\0' 제외)
    int written = 0;

    if (!input || !output || outputSize <= 0) {
        return 0;
    }

    // 초기화 (안전)
    memset(output, 0, outputSize);

#if defined(_WIN32)
	UINT cpIn  = CP_ACP;   // 기본: EUC-KR 또는 ANSI
    UINT cpOut = CP_UTF8;  // 기본: UTF-8

    // 입력 인코딩 판단
    if (_stricmp(source, "utf-8") == 0) {
        cpIn = CP_UTF8;
    } else if (_stricmp(source, "euc-kr") == 0 ||
               _stricmp(source, "cp949") == 0) {
        cpIn = 949;
    }

    // 출력 인코딩 판단
    if (_stricmp(target, "utf-8") == 0) {
        cpOut = CP_UTF8;
    } else if (_stricmp(target, "euc-kr") == 0 ||
               _stricmp(target, "cp949") == 0) {
        cpOut = 949;
    }

    // 1단계: 멀티바이트 → 유니코드
    int wlen = MultiByteToWideChar(cpIn, 0, input, -1, NULL, 0);
    if (wlen <= 0) {
        return 0;
    }

	wchar_t* wbuf = (wchar_t*)malloc(sizeof(wchar_t) * wlen);
    if (!wbuf) {
        return 0;
    }
	
    int wret = MultiByteToWideChar(cpIn, 0, input, -1, wbuf, wlen);
    if (wret <= 0) {
        free(wbuf);
        return 0;
    }

    // 2단계: 유니코드 → 출력 멀티바이트(UTF-8 등)
    // cbMultiByte에 (int)outputSize 를 주고, -1 을 cchWideChar 에 주면
    // 널 문자까지 포함해서 써준다.
    int outBytes = WideCharToMultiByte(cpOut, 0,
                                       wbuf, -1,
                                       output, (int)outputSize,
                                       NULL, NULL);

    free(wbuf);

    if (outBytes <= 0) {
        // 실패
        output[0] = '\0';
        return 0;
    }

    // outBytes에는 널까지 포함한 길이가 들어 있으니,
    // 실제 문자열 길이는 outBytes - 1
    written = outBytes - 1;

#else //#if defined(_WIN32)

    iconv_t cd = iconv_open(target, source);
    if (cd == (iconv_t)-1) {
        return 0;
    }

    char* inBuf = const_cast<char*>(input);
    size_t inBytesLeft = strlen(input);

    char* outBuf = output;
    size_t outBytesLeft = outputSize - 1;  // 마지막 1바이트는 \0 용

    size_t res = iconv(cd,
                       &inBuf, &inBytesLeft,
                       &outBuf, &outBytesLeft);

    iconv_close(cd);

	if (res == (size_t)-1) {
        // 실패 → output 은 "" 유지
        output[0] = '\0';
        return 0;
    }

    // 실제로 쓴 바이트 수 (아이콘브 기준)
    size_t writtenSz = ((size_t)outputSize - 1) - outBytesLeft;
    if (writtenSz > (size_t)(outputSize - 1)) {
        writtenSz = (size_t)(outputSize - 1);  // 방어
    }

    written = (int)writtenSz;
#endif
	// 여기서부터는 공통 처리
    if (written < 0) {
        written = 0;
    }
    if (written >= outputSize) {
        written = outputSize - 1;   // 방어
    }

    // 널 종료 보장 (윈도우에서도 한 번 더 찍어줘도 무해)
    output[written] = '\0';

    return written;

}


#define MAX_TRIM_LEN 1024

char* trim(char *line)
{
	size_t len = 0;
	char cpTrim[MAX_TRIM_LEN] = { 0, };
	int xMan = 0;
	int i;

	len = strlen(line);

	if (len > 0) 
	{
		if (len >= MAX_TRIM_LEN)
		{
			puts("string too long");
			return NULL;
		}
		else
		{
			strcpy(cpTrim, line);

			// 앞에거 잘라내기
			for (i = 0; i < len; i++)
			{
				if (cpTrim[i] == ' ' || cpTrim[i] == '\t')
					xMan++;
				else
					break;
			}

			// 뒤에거 잘라내기
			for (i = len - 1; i >= 0; i--)
			{
				if (cpTrim[i] == ' ' || cpTrim[i] == '\t' || cpTrim[i] == '\n')
					cpTrim[i] = '\0';
				else
					break;
			}

			if (len != (strlen(cpTrim) - xMan)) {
				strcpy(line, cpTrim + xMan);
			}
		}
	}

	//return strlen(line);
	return line;
}


/*
//char* trim(char *s); // 문자열 좌우 공백 모두 삭제 함수
//char* ltrim(char *s); // 문자열 좌측 공백 제거 함수
//char* rtrim(char* s); // 문자열 우측 공백 제거 함수

// 문자열 우측 공백문자 삭제 함수
char* rtrim(char* s) {
	char t[MAX_TRIM_LEN];
	char *end;

	// Visual C 2003 이하에서는
	strcpy(t, s);
	// 이렇게 해야 함
	//strcpy_s(t, s); // 이것은 Visual C 2005용
	end = t + strlen(t) - 1;
	while (end != t && isspace(*end))
		end--;
	*(end + 1) = '\0';
	s = t;

	return s;
}


// 문자열 좌측 공백문자 삭제 함수
char* ltrim(char *s) {
	char* begin;
	begin = s;

	while (*begin != '\0') {
		if (isspace(*begin))
			begin++;
		else {
			s = begin;
			break;
		}
	}

	return s;
}


// 문자열 앞뒤 공백 모두 삭제 함수
char* trim(char *s) {
	return rtrim(ltrim(s));
}
*/

#define MAX_UPPER_LEN 1024
char* strupper(char* lower)
{
#if defined(_WIN32)
	_strupr(lower);
#else
	size_t len = 0;
	char cpUpper[MAX_UPPER_LEN] = { 0, };
	int i;

	if (lower != nullptr) {
		len = strlen(lower);

		if (len >= MAX_UPPER_LEN) {
			puts("lower string too long");
			return NULL;
		}
		else if (len > 0) {
			for (i = 0; i < len; i++) {
				cpUpper[i] = toupper(lower[i]);
			} // for
			cpUpper[i] = '\0';

			strcpy(lower, cpUpper);
		}
	}
#endif

	return lower;
}


char* strlower(char* upper)
{
#if defined(_WIN32)
	_strlwr(upper);
#else
	size_t len = 0;
	char cpLower[MAX_UPPER_LEN] = { 0, };
	int i;

	if (upper != nullptr) {
		len = strlen(upper);

		if (len >= MAX_UPPER_LEN) {
			puts("upper string too long");
			return NULL;
		} else if (len > 0) {
			for (i = 0; i < len; i++) {
				cpLower[i] = tolower(upper[i]);
			} // for
			cpLower[i] = '\0';

			strcpy(upper, cpLower);
		}
	}
#endif

	return upper;
}


const char* getDistanceString(const int dist, const double base)
{
	static char szBuff[MAX_UPPER_LEN];

	memset(szBuff, 0x00, sizeof(szBuff));

	if ((base < 1000.f) && (dist < 1000.f)) {
		if (dist <= 0) {
			strcpy(szBuff, "0 m");
		} else { // if (dist < 1000.f) {
			sprintf(szBuff, "%d m", dist);
		}
	} else {
		if (dist <= 0) {
			strcpy(szBuff, "0 km");
		} else if (dist < 1000.f) {
			sprintf(szBuff, "%.2f km", dist / 1000.f);
		} else {
			sprintf(szBuff, "%.1f km", dist / 1000.f);
		}
	}

	return szBuff;
}


const char* getDurationString(const int time, const bool contraction)
{
	static char szBuff[MAX_UPPER_LEN];
	memset(szBuff, 0x00, sizeof(szBuff));

	if (time <= 0) {
		if (contraction) {
			sprintf(szBuff, "0 m");
		} else {
			sprintf(szBuff, "0 mins");
		}
	} else {
		if (time > 3600) {
			if (contraction) {
				sprintf(szBuff, "%d h %d m", time / 3600, (time % 3600) / 60);
			} else {
				sprintf(szBuff, "%d hours %d mins", time / 3600, (time % 3600) / 60);
			}			
		} else if (time > 60) {
			if (contraction) {
				sprintf(szBuff, "%d m", time / 60);
			} else {
				sprintf(szBuff, "%d mins", time / 60);
			}			
		} else {
			if (contraction) {
				sprintf(szBuff, "1 m");
			} else {
				sprintf(szBuff, "1 mins");
			}			
		}
	}

	return szBuff;
}


const char* getCostString(const double cost)
{
	static char szBuff[MAX_UPPER_LEN];
	memset(szBuff, 0x00, sizeof(szBuff));

	if (cost <= 0) {
		return "0 points";
	} else {
		sprintf(szBuff, "%d points", static_cast<long long int>(cost));
	}

	return szBuff;
}

const char* getSizeString(const unsigned long size)
{
	static char szBuff[MAX_UPPER_LEN];
	memset(szBuff, 0x00, sizeof(szBuff));

	if (size >= 1024 * 1024 * 1024) { // GiB
		sprintf(szBuff, "%.2f GiB", static_cast<double>(size / (1024.f * 1024.f * 1024.f)));
	} else if (size >= 1024 * 1024) { // MiB
		sprintf(szBuff, "%.2f MiB", static_cast<double>(size / (1024.f * 1024.f)));
	} else if (size >= 1024) { // KiB
		sprintf(szBuff, "%.2f KiB", static_cast<double>(size / 1024.f));
	} else {
		sprintf(szBuff, "%d Byte", size);
	}

	return szBuff;
}