#if defined(_WIN32)
#include <windows.h>
#include <wchar.h>
#define _WINDOWS
#endif
#include <string.h>

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