//Modified Version for WebAssembly, 2019.11.13, gijoe@engistech.com
//10.20 - Image Rotate function added.
//10.29 - Image Zoom function added.
//11.13 - Tile Image function added.

#ifndef _GN_PNGHANDLER_H
#define _GN_PNGHANDLER_H

class CImageHandler
{
	// 생성입니다.
public:
	CImageHandler();
	virtual ~CImageHandler();
	static unsigned char* LoadImageFromFile(char* chsFileName, int* rw, int* rh, bool bUseAlpha=true, bool bUseCache=true);
	static void _CleanImageCache(bool bAll = false); //only for the FromFile

	static unsigned char* LoadImageFromMemory(unsigned char* chsFileBuffer, int sizeBuffer, int* rw, int* rh, bool bUseAlpha=true);
};

#endif