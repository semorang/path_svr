#pragma once


#if defined(_WIN32)
void MultiByteToUnicode(const char* strMultibyte, wchar_t* strUnicode);
void UnicodeToUTF8(const wchar_t* strUnicode, char* strUTF8);
void MultiByteToUTF8(const char* strMultibyte, char* strUTF8);
#endif