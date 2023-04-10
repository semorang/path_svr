//Modified Version for WebAssembly, 2022.04.21, gijoe@inavi.kr
//04.21 - first version using stb_freetype.h

#ifndef _GN_FONTHANDLER_H
#define _GN_FONTHANDLER_H

class CFontHandler
{
	// 생성입니다.
public:
	CFontHandler();
	virtual ~CFontHandler();
	static unsigned char* GetTextImage(char* chsMultiStr, unsigned char nTextHeight, int* rw, int* rh);
	static void SetFontFile(char* chsFontFilePath);
	static void _CleanImageCache(bool bAll);
};

#endif