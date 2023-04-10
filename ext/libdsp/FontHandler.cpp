#include "../stdafx.h"

//#include "pch.h"
//2021.12.28, imgunill@naver.com

//#include "../pch.h"
#include <Windows.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <unordered_map>
#include "FontHandler.h"

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h" /* http://nothings.org/stb/stb_truetype.h */

using namespace std;

struct stTextCache {
	unsigned short nWidth;
	unsigned short nHeight;
	unsigned char* pData;
};
unordered_map<string, stTextCache> g_TextCache;
string g_FontFilePath = "./NotoSansKR-Bold-Hestia.ttf";

//#define AlphaBlend32(pDest, pSrc, colsrc) pDest = (colsrc & 0xff000000) | ((((((colsrc & 0x00ff00ff) * pSrc) + ((pDest & 0x00ff00ff) * (0xff - pSrc))) & 0xff00ff00) | ((((colsrc & 0x0000ff00) * pSrc) + ((pDest & 0x0000ff00) * (0xff - pSrc))) & 0x00ff0000)) >> 8)

CFontHandler::CFontHandler() {
}

CFontHandler::~CFontHandler() {
	_CleanImageCache(true);
}

void MultiByteToUnicode(const char* strMultibyte, wchar_t* strUnicode)
{
#ifdef _WINDOWS
	int nLen = MultiByteToWideChar(CP_ACP, 0, strMultibyte, strlen(strMultibyte), NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, strMultibyte, strlen(strMultibyte), strUnicode, nLen);
#else
	mbstowcs(strUnicode, strMultibyte, strlen(strMultibyte));
#endif        
}

int STR_UTF8toUnicode(const char* pUtf8Str, wchar_t* pUniStr) {
	int iIndex = 0;
	int iCount = 0;
	unsigned short wChar;
	while (0 != pUtf8Str[iIndex]) {
		if ((0xE0 == (pUtf8Str[iIndex] & 0xE0))) {
			wChar = ((pUtf8Str[iIndex] & 0x0f) << 12) | ((pUtf8Str[iIndex + 1] & 0x3F) << 6) | (pUtf8Str[iIndex + 2] & 0x3F);
			iIndex += 3;
		}
		else if (0xC0 == (pUtf8Str[iIndex] & 0xC0)) {
			wChar = ((pUtf8Str[iIndex] & 0x1F) << 6) | (pUtf8Str[iIndex + 1] & 0x3F);
			iIndex += 2;
		}
		else {
			wChar = pUtf8Str[iIndex] & 0x7F;
			iIndex++;
		}
		pUniStr[iCount] = wChar;
		iCount++;
	}
	pUniStr[iCount] = 0;
	return iCount;
}
//------------------------------------------------------------------------------------------------------------------
static void ConvertRGBAToBGRA(unsigned char* pBitMap, int w, int h, bool bUseAlpha) {
	for (int i = w * h; i--; pBitMap += 4) {
		*(pBitMap) ^= *(pBitMap + 2) ^= *pBitMap ^= *(pBitMap + 2);
		if (!bUseAlpha) *(pBitMap + 3) = 255;
	} //for
}

void CFontHandler::_CleanImageCache(bool bAll) {
	if (g_TextCache.empty()) return;

	if (bAll) {
		for (unordered_map<string, stTextCache>::iterator itr = g_TextCache.begin(); itr != g_TextCache.end(); ++itr) {
			if (itr->second.pData)
				free(itr->second.pData);
		}
		unordered_map<string, stTextCache>().swap(g_TextCache);
	}
	else if (g_TextCache.size() > 200) {
		for (unordered_map<string, stTextCache>::iterator itr = g_TextCache.begin(); itr != g_TextCache.end(); ++itr)
			if (itr->second.pData)
				free(itr->second.pData);
		unordered_map<string, stTextCache>().swap(g_TextCache);
	}
}

void CFontHandler::SetFontFile(char* chsFontFilePath) {
	if (chsFontFilePath)
	{
		g_FontFilePath = chsFontFilePath;
	}
}

unsigned char* CFontHandler::GetTextImage(char* chsMultiStr, unsigned char nTextHeight, int* rw, int* rh)
{
	unsigned char* pByImage = NULL;
	_CleanImageCache(false); //check if over count 200 ea then clean

	string szSearchTxt = chsMultiStr; szSearchTxt.push_back((unsigned char)nTextHeight);

	unordered_map<string, stTextCache>::iterator itr = g_TextCache.find(szSearchTxt.c_str());
	if (itr == g_TextCache.end()) { //Cache에 없으면..
		// load font file ----------------------------------------------------------------
		unsigned char* fontBuffer = NULL;
		unordered_map<string, stTextCache>::iterator itr_Font = g_TextCache.find(g_FontFilePath);
		if (itr_Font == g_TextCache.end()) { //Cache에 없으면..

			FILE* fontFile = fopen(g_FontFilePath.c_str(), "rb");
			fseek(fontFile, 0, SEEK_END);
			long size = ftell(fontFile); /* how long is the file ? */
			fseek(fontFile, 0, SEEK_SET); /* reset */

			fontBuffer = (unsigned char*)malloc(size);
			fread(fontBuffer, size, 1, fontFile);
			fclose(fontFile);

			stTextCache tmpFontinfo = { 0, 0, (unsigned char*)fontBuffer };

			g_TextCache.insert(unordered_map<string, stTextCache>::value_type(g_FontFilePath, tmpFontinfo));
		} else {
			fontBuffer = itr_Font->second.pData;
		}//if
		//--------------------------------------------------------------------------------
		wchar_t strUnicode[256] = { 0, };

#if 0 //in case of Unicode
		wchar_t* strUnicode = L"안녕 이건 테스트야 Hello World!!!";
#endif

#if 1 //in case of multibyte
		//char* chsMultiStr = "안녕 이건 테스트야 Hello World!!!";
		MultiByteToUnicode(chsMultiStr, strUnicode);
		//int num_chars = lstrlenW((LPCWSTR)strUnicode);
#endif

#if 0 //in case of Utf8
	//char* chsMultiStr = "Hello World!!! 12345.";
		STR_UTF8toUnicode(chsMultiStr, strUnicode);
#endif
		int num_chars = wcslen((wchar_t*)strUnicode);
		//----------------------------------------------------------------

		/* prepare font */
		stbtt_fontinfo info;
		if (!stbtt_InitFont(&info, fontBuffer, 0)) {
			printf("failed\n");
		}

		unsigned char chsBitmap[128 * 512] = { 0x00, };
		const int bitmap_w = 512; /* m_byBitmap width */
		const int bitmap_h = 128; /* m_byBitmap height */
		int lineHeight = nTextHeight; /* line(font) height */

		/* calculate font scaling */
		float scale = stbtt_ScaleForPixelHeight(&info, lineHeight);

		int x_width = 0;
		int ascent, descent, lineGap;
		stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

		ascent = roundf(ascent * scale);
		descent = roundf(descent * scale);

		int i;
		for (i = 0; i < num_chars; ++i) {
			/* how wide is this character */
			int advanceWidth = 0;
			int leftSideBearing = 0;
			stbtt_GetCodepointHMetrics(&info, strUnicode[i], &advanceWidth, &leftSideBearing);
			/* (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character strUnicode[i].) */

			/* get bounding box for character (may be offset to account for chars that dip above or below the line */
			int c_x1, c_y1, c_x2, c_y2;
			stbtt_GetCodepointBitmapBox(&info, strUnicode[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

			/* compute y (different characters have different heights */
			int y = ascent + c_y1;

			/* render character (stride and offset is important here) */
			int byteOffset = x_width + roundf(leftSideBearing * scale) + (y * bitmap_w);
			stbtt_MakeCodepointBitmap(&info, chsBitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, bitmap_w, scale, scale, strUnicode[i]);
			/* advance x */
			x_width += roundf(advanceWidth * scale);

			/* add kerning */
			int kern;
			kern = stbtt_GetCodepointKernAdvance(&info, strUnicode[i], strUnicode[i + 1]);
			x_width += roundf(kern * scale);
		} //for

		stTextCache tmpImgInfo = { x_width, lineHeight, (unsigned char*)malloc(x_width * lineHeight) };
		unsigned char* pSrc = &chsBitmap[0];
		unsigned char* pDest = &tmpImgInfo.pData[0];

		for (int y = 0; y < lineHeight; ++y) {
			pSrc = chsBitmap + (y * 512);
			for (int x = 0; x < x_width; ++x, ++pSrc) {
				(*pDest++) = *pSrc;
			} //for x
		} //for y

		g_TextCache.insert(unordered_map<string, stTextCache>::value_type(szSearchTxt, tmpImgInfo));

		*rw = tmpImgInfo.nWidth; // x;
		*rh = tmpImgInfo.nHeight; // l_h;
		pByImage = tmpImgInfo.pData;
	}
	else {
		*rw = itr->second.nWidth;
		*rh = itr->second.nHeight;
		pByImage = itr->second.pData;
	} //if
	
	return pByImage;
}