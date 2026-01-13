#pragma once

#include <string>

#if defined(_WIN32)
int MultiByteToUnicode(const char* strMultibyte, wchar_t* strUnicode);
int UTF8ToUnicode(const char* strMultibyte, wchar_t* strUnicode);
int UnicodeToUTF8(const wchar_t* strUnicode, char* strUTF8);
int MultiByteToUTF8(const char* strMultibyte, char* strUTF8);
int UnicodeToMultiByte(const wchar_t* strUnicode, char* strMultibyte);
int UnicodeToMultiByte(const wchar_t* strUnicode, std::string& out);
int UTF8ToMultiByte(const char* strUTF8, char* strMultibyte);

char* strsep(char** stringp, const char* delim);
#endif

int encoding(const char* input, const char* source, const char* target, char* output, int outputSize);

char* trim(char *line);
char* strupper(char* str);
char* strlower(char* str);

const char* getDistanceString(const int dist, const double base = 1000.f);
const char* getDurationString(const int time, const bool contraction = false);
const char* getCostString(const double cost);
const char* getSizeString(const unsigned long size);