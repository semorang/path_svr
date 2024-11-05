#pragma once

#if defined(_WIN32)
void MultiByteToUnicode(const char* strMultibyte, wchar_t* strUnicode);
void UTF8ToUnicode(const char* strMultibyte, wchar_t* strUnicode);
void UnicodeToUTF8(const wchar_t* strUnicode, char* strUTF8);
void MultiByteToUTF8(const char* strMultibyte, char* strUTF8);
void UnicodeToMultiByte(const wchar_t* strUnicode, char* strMultibyte);
void UTF8ToMultiByte(const char* strUTF8, char* strMultibyte);

char* strsep(char** stringp, const char* delim);
#else
char * encoding(const char *text_input, char *source, char *target);
#endif

char* trim(char *line);
char* strupper(char* str);