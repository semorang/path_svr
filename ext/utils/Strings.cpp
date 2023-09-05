#if defined(_WIN32)
#include <windows.h>
#include <wchar.h>
#define _WINDOWS
#else
#include <iconv.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Strings.h"


#if defined(_WIN32)
void MultiByteToUnicode(const char* strMultibyte, wchar_t* strUnicode)
{
#ifdef _WINDOWS
	int nLen = MultiByteToWideChar(CP_ACP, 0, strMultibyte, strlen(strMultibyte), NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, strMultibyte, strlen(strMultibyte), strUnicode, nLen);
#else
	mbstowcs(strUnicode, strMultibyte, strlen(strMultibyte));
#endif        
}

void UTF8ToUnicode(const char* strMultibyte, wchar_t* strUnicode)
{
#ifdef _WINDOWS
	int nLen = MultiByteToWideChar(CP_UTF8, 0, strMultibyte, strlen(strMultibyte), NULL, NULL);
	MultiByteToWideChar(CP_UTF8, 0, strMultibyte, strlen(strMultibyte), strUnicode, nLen);
#else
	mbstowcs(strUnicode, strMultibyte, strlen(strMultibyte));
#endif        
}

void UnicodeToUTF8(const wchar_t* strUnicode, char* strUTF8)
{
#ifdef _WINDOWS
	int nLen = WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), strUTF8, nLen, NULL, NULL);
#else
	wcstombs(strUnicode, strMultibyte, strlen(strMultibyte));
#endif        
}

void MultiByteToUTF8(const char* strMultibyte, char* strUTF8)
{
	wchar_t wszUnicode[MAX_PATH] = { 0, };

	MultiByteToUnicode(strMultibyte, wszUnicode);

	UnicodeToUTF8(wszUnicode, strUTF8);
}

#else //#if defined(_WIN32)

char * encoding(const char *text_input, char *source, char *target)
{
	iconv_t it;

	int input_len = strlen(text_input) + 1;
	int output_len = input_len * 2;

	size_t in_size = input_len;
	size_t out_size = output_len;

	char *output = (char *)malloc(output_len);

	char *output_buf = output;
	char *input_buf = const_cast<char*>(text_input);

	it = iconv_open(target, source);
	int ret = iconv(it, &input_buf, &in_size, &output_buf, &out_size);


	iconv_close(it);

	return output;
}
#endif //#if defined(_WIN32)


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


//char *trim(char *str, char *buf, int buf_len)
//{
//	int str_len, start_point = 0;
//	int i, j;
//
//	if (!str || !buf) return NULL;
//
//	str_len = strlen(str) - 1;
//	while (*(str + (str_len -= 1)) == ' ');
//	while (*(str + (start_point += 1)) == ' ');
//
//	for (i = start_point, j = 0; i <= str_len && j < buf_len; i++, j++) {
//		buf[j] = str[i];
//	}
//	buf[j] = '\0';
//
//	return buf;
//}

#define MAX_TRIM_LEN 1024

char* trim(char *line)
{
	size_t len = 0;
	char cpTrim[MAX_TRIM_LEN];
	int xMan = 0;
	int i;

	len = strlen(line);
	if (len >= MAX_TRIM_LEN)
	{
		puts("string too long");
		return NULL;
	}

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

