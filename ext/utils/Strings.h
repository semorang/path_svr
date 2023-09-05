#pragma once

#if defined(_WIN32)
void MultiByteToUnicode(const char* strMultibyte, wchar_t* strUnicode);
void UTF8ToUnicode(const char* strMultibyte, wchar_t* strUnicode);
void UnicodeToUTF8(const wchar_t* strUnicode, char* strUTF8);
void MultiByteToUTF8(const char* strMultibyte, char* strUTF8);
#endif

char* strsep(char** stringp, const char* delim);
char* trim(char *line);
