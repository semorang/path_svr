#pragma once

#include "route/MapDef.h"

#include "shp/shpio.h"
#include "libdsp/UniDib32.h"
#include "libdsp/GnOSMCoord.h"
#include "libdsp/GnType.h"
#include "libdsp/FontHandler.h"
#include "libdsp/ImageHandler.h"


typedef union {
	uint32_t rgba;
	struct {
		uint32_t r : 8;
		uint32_t g : 8;
		uint32_t b : 8;
		uint32_t a : 8;
	};
}color32_t;

#define COLOR_RGBA(r,g,b,a)	(uint32_t)((a << 24) | (b << 16) | (g << 8) | r)

#define RGBA_TRE_HIKING			COLOR_RGBA(50, 130, 240, 255)		// ����
#define RGBA_TRE_TRAIL			COLOR_RGBA(50, 120, 30, 255)		// �ѷ���
#define RGBA_TRE_BIKE			COLOR_RGBA(240, 150, 90, 255)		// �����ű�
#define RGBA_TRE_CROSS			COLOR_RGBA(110, 250, 250, 255)		// ���ֱ�
#define RGBA_TRE_DEFAULT		COLOR_RGBA(240, 240, 240, 255)		// ������

#define RGBA_TRE_ROAD_TATTERED	COLOR_RGBA(230, 50, 30, 255)		// �ʴ���
#define RGBA_TRE_ROAD_ROPE		COLOR_RGBA(250, 150, 10, 255)		// ����
#define RGBA_TRE_ROAD_LADDER	COLOR_RGBA(230, 50, 40, 255)		// ��ٸ�
#define RGBA_TRE_ROAD_RIDGE		COLOR_RGBA(230, 60, 250, 255)		// ����
#define RGBA_TRE_ROAD_ROCK		COLOR_RGBA(210, 10, 10, 255)		// �ϸ�
#define RGBA_TRE_ROAD_DECK		COLOR_RGBA(120, 70, 20, 255)		// ��ũ�ε�
#define RGBA_TRE_ROAD_PALM		COLOR_RGBA(160, 250, 250, 255)		// ���ڼ���Ʈ
#define RGBA_TRE_ROAD_BRIDGE	COLOR_RGBA(50, 200, 10, 255)		// ����
#define RGBA_TRE_ROAD_STAIRS	COLOR_RGBA(140, 250, 140, 255)		// ���
#define RGBA_TRE_ROAD_PAVE		COLOR_RGBA(30, 220, 10, 255)		// �����
#define RGBA_TRE_ROAD_ALLEY		COLOR_RGBA(100, 200, 10, 255)		// ���ֱ�
#define RGBA_TRE_ROAD_DEFAULT	COLOR_RGBA(240, 240, 240, 255)		// ������




extern CUniDib32 g_UniDib;
extern CFontHandler g_pFontMgr;
extern CImageHandler g_pImageMgr;

class CDrawing
{
public:
	CDrawing();
	virtual ~CDrawing();

private:

public:
	void DrawLinkInfo(IN const stLinkInfo* pLink, IN const uint32_t nWidth, IN const uint32_t nHeight);
	void DrawRouteInfo(IN const RouteResultInfo* pRouteResult, IN const uint32_t nWidth, IN const uint32_t nHeight);
	void DrawColorInfo(IN const int type, IN const uint32_t nWidth, IN const uint32_t nHeight);
};

