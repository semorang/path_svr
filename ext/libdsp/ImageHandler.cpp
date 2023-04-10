#include "../stdafx.h"

//#include "pch.h"
//2021.12.28, imgunill@naver.com

#include <stdlib.h>
#include <math.h>
#include <string>
#include <unordered_map>
#include "ImageHandler.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

struct stImageCache {
	unsigned short nWidth;
	unsigned short nHeight;
	unsigned char* pData;
};
unordered_map<string, stImageCache> g_ImageCache;

//#define AlphaBlend32(pDest, pSrc, colsrc) pDest = (colsrc & 0xff000000) | ((((((colsrc & 0x00ff00ff) * pSrc) + ((pDest & 0x00ff00ff) * (0xff - pSrc))) & 0xff00ff00) | ((((colsrc & 0x0000ff00) * pSrc) + ((pDest & 0x0000ff00) * (0xff - pSrc))) & 0x00ff0000)) >> 8)

CImageHandler::CImageHandler() {
}

CImageHandler::~CImageHandler() {
	_CleanImageCache(true);
}

//------------------------------------------------------------------------------------------------------------------
static void ConvertRGBAToBGRA(unsigned char* pBitMap, int w, int h, bool bUseAlpha) {
	for (int i = w * h; i--; pBitMap += 4) {
		*(pBitMap) ^= *(pBitMap + 2) ^= *pBitMap ^= *(pBitMap + 2);
		if (!bUseAlpha) *(pBitMap + 3) = 255;
	} //for
}

unsigned char* CImageHandler::LoadImageFromFile(char* chsFileName, int* rw, int* rh, bool bUseAlpha, bool bUseCache) {
	struct stat buffer;
	if (stat(chsFileName, &buffer) != 0)
		return NULL; //For fast checking if file exist.

	unsigned char* pByImage = NULL;
	if (bUseCache) {
		_CleanImageCache(false); //check if over count 200 ea then clean

		unordered_map<string, stImageCache>::iterator itr = g_ImageCache.find(chsFileName);
		if (itr == g_ImageCache.end()) { //Cache에 없으면..
		//if (true) { //Cache에 없으면..
			int channels;
			unsigned char* pNewImg = stbi_load(chsFileName, rw, rh, &channels, 4); //BITMAP order is --> RGBA 
			if (pNewImg) {
				ConvertRGBAToBGRA(pNewImg, *rw, *rh, bUseAlpha); //change order to --> BGRA 

				stImageCache tmpImgInfo = { *rw, *rh, NULL };
				tmpImgInfo.pData = (unsigned char*)malloc(*rw * *rh * 4);
				memcpy(tmpImgInfo.pData, pNewImg, *rw * *rh * 4);

				g_ImageCache.insert(unordered_map<string, stImageCache>::value_type(chsFileName, tmpImgInfo));
				free(pNewImg);
				pByImage = tmpImgInfo.pData;
			} //if
		}
		else {
			*rw = itr->second.nWidth;
			*rh = itr->second.nHeight;
			pByImage = itr->second.pData;
		} //if
	}
	else {
		struct stat buffer;
		if (stat(chsFileName, &buffer) != 0)
			return NULL; //For fast checking if file exist.

		int channels;
		pByImage = stbi_load(chsFileName, rw, rh, &channels, 4); //BITMAP order is --> RGBA 
		if (pByImage) ConvertRGBAToBGRA(pByImage, *rw, *rh, bUseAlpha);
		else pByImage = NULL;
	}

	return pByImage; //after use, we have to do --> STBI_FREE(pByImage); or free(pByImage);
}

void CImageHandler::_CleanImageCache(bool bAll) {
	if (g_ImageCache.empty()) return;

	if (bAll) {
		for (unordered_map<string, stImageCache>::iterator itr = g_ImageCache.begin(); itr != g_ImageCache.end(); ++itr) {
			if (itr->second.pData)
				free(itr->second.pData);
		}
		unordered_map<string, stImageCache>().swap(g_ImageCache);
	}
	else if (g_ImageCache.size() > 200) {
		for (unordered_map<string, stImageCache>::iterator itr = g_ImageCache.begin(); itr != g_ImageCache.end(); ++itr)
			if (itr->second.pData)
				free(itr->second.pData);
		unordered_map<string, stImageCache>().swap(g_ImageCache);
	}
}

unsigned char* CImageHandler::LoadImageFromMemory(unsigned char* chsFileBuffer, int sizeBuffer, int* rw, int* rh, bool bUseAlpha) {
	if (chsFileBuffer == NULL) {
		*rw = *rh = 0;
		return NULL;
	}

	int channels;
	unsigned char* pByImage = stbi_load_from_memory(chsFileBuffer, sizeBuffer, rw, rh, &channels, 4);
	if (pByImage) ConvertRGBAToBGRA(pByImage, *rw, *rh, bUseAlpha);
	else pByImage = NULL;
	return pByImage; //after use, we have to do --> STBI_FREE(pByImage); or free(pByImage);
}