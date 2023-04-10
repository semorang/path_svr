#include "Drawing.h"

#include "utils\UserLog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// drawing members;
CUniDib32 g_UniDib;
CFontHandler g_pFontMgr;
CImageHandler g_pImageMgr;


CDrawing::CDrawing()
{
	//g_pFontMgr.SetFontFile("C:/__Down/chrome/MONACO.TTF");
	g_pFontMgr.SetFontFile("C:/__Data/font/naver/maruburi/마루 부리/MaruBuriTTF/MaruBuri-Light.ttf");
}


CDrawing::~CDrawing()
{
}

void CDrawing::DrawLinkInfo(IN const stLinkInfo* pLink, IN const uint32_t nWidth, IN const uint32_t nHeight)
{
	if (pLink == nullptr) {
		return;
	}

	int32_t nStartX = nWidth - 300;
	int32_t nStartY = nHeight - 600;
	// draw bg box
	g_UniDib.FillRect(nStartX, nStartY, 300, 600, 100, 100, 100, 200);

	// darw center coordnate
	char szInfo[64] = { 0, };
	int nFontHeight = 20;
	int retWidth = 0;
	int retHeight = 0;
	int nOffY = 50;
	int nGapX = 20;
	int nGapY = 40;
	unsigned char* pCoord;

	sprintf(szInfo, "Clicked link info");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;
	sprintf(szInfo, "Mesh: %d", pLink->link_id.tile_id);
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;
	sprintf(szInfo, "Link: %d", pLink->link_id.nid);
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;
	sprintf(szInfo, "SNode: %d, %d, %d°", pLink->snode_id.tile_id, pLink->snode_id.nid, pLink->snode_id.dir);
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;
	sprintf(szInfo, "ENode: %d, %d, %d°", pLink->enode_id.tile_id, pLink->enode_id.nid, pLink->enode_id.dir);
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;
	if (pLink->length > 1000) {
		sprintf(szInfo, "Length: %.3fkm", pLink->length / 1000.f);
	}
	else {
		sprintf(szInfo, "Length: %dm", (int)pLink->length);
	}
	
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;
	sprintf(szInfo, "Vtx Count: %d", pLink->vtPts.size() - 1);
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);


	// course type
	nOffY += nGapY;
	sprintf(szInfo, "Course Type: ");

	if (pLink->link_type == TYPE_DATA_TREKKING)
	{
		// 0:미정의, 1 : 등산로, 2 : 둘레길, 3 : 자전거길, 4 : 종주코스
		if (pLink->tre.course_type == TYPE_TRE_HIKING) { // 등산로
			strcat(szInfo, "등산로");
		}
		else if (pLink->tre.course_type == TYPE_TRE_TRAIL) { // 둘레길
			strcat(szInfo, "둘레길");
		}
		else if (pLink->tre.course_type == TYPE_TRE_BIKE) { // 자전거길
			strcat(szInfo, "자전거길");
		}
		else if (pLink->tre.course_type == TYPE_TRE_CROSS) { // 종주길
			strcat(szInfo, "종주길");
		}
		else {
			strcat(szInfo, "미정의");
		}
	}
	else if (pLink->link_type == TYPE_DATA_PEDESTRIAN)
	{
		//보행자도로 타입, 1:복선도록, 2:차량겸용도로, 3:자전거전용도로, 4:보행전용도로, 5:가상보행도로
		if (pLink->ped.walk_type == TYPE_WALK_SIDE) {
			strcat(szInfo, "복선도록");
		}
		else if (pLink->ped.walk_type == TYPE_WALK_WITH_CAR) {
			strcat(szInfo, "차량겸용도로");
		}
		else if (pLink->ped.walk_type == TYPE_WALK_WITH_BYC) {
			strcat(szInfo, "자전거전용도로");
		}
		else if (pLink->ped.walk_type == TYPE_WALK_ONLY) {
			strcat(szInfo, "보행전용도로");
		}
		else if (pLink->ped.walk_type == TYPE_WALK_THROUGH) {
			strcat(szInfo, "가상보행도로");
		}
		else {
			strcat(szInfo, "미정의");
		}

		// 자전거도로 타입, 1:자전거전용, 2:보행자/차량겸용 자전거도로, 3:보행도로
		if (pLink->ped.bycicle_type == TYPE_BYC_ONLY) {
			strcat(szInfo, ",자전거전용");
		}
		else if (pLink->ped.bycicle_type == TYPE_BYC_WITH_CAR) {
			strcat(szInfo, ",보행자/차량겸용 자전거도로");
		}
		else if (pLink->ped.bycicle_type == TYPE_BYC_WITH_WALK) {
			strcat(szInfo, ",보행도로");
		}
		else {
			strcat(szInfo, ",미정의");
		}
	}
	else { // 미지정
		
	}
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);


	// road type
	nOffY += nGapY;
	sprintf(szInfo, "Road Type: ");

	if (pLink->link_type == TYPE_DATA_TREKKING)
	{
		if (pLink->tre.road_info > 0) {
			int nRoadType = pLink->tre.road_info;
			int cntRoadType = 0;
			// 노면정보 코드, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드

			if ((nRoadType & ROUTE_AVOID_DECK) == ROUTE_AVOID_DECK) { // 11:데크로드 = 1000 0000 0000 = 1024
																	  /*strcat(szInfo, ", deck road");*/
				if (cntRoadType > 0) { strcat(szInfo, ", "); }
				strcat(szInfo, "데크길");
				cntRoadType++;
			}
			if ((nRoadType & ROUTE_AVOID_PALM) == ROUTE_AVOID_PALM) { // 10:야자수매트 = 100 0000 0000 = 512
																	  //strcat(szInfo, ", palm mat");
				if (cntRoadType > 0) { strcat(szInfo, ", "); }
				strcat(szInfo, "야자수매트");
				cntRoadType++;
			}
			if ((nRoadType & ROUTE_AVOID_TATTERED) == ROUTE_AVOID_TATTERED) { // 9:너덜길 = 1 0000 0000 = 256
																			  //strcat(szInfo, ", tattered road");
				if (cntRoadType > 0) { strcat(szInfo, ", "); }
				strcat(szInfo, "너덜길");
				cntRoadType++;
			}
			if ((nRoadType & ROUTE_AVOID_ROPE) == ROUTE_AVOID_ROPE) { // 8:밧줄 = 1000 0000 = 128
																	  //strcat(szInfo, ", rope");
				if (cntRoadType > 0) { strcat(szInfo, ", "); }
				strcat(szInfo, "밧줄");
				cntRoadType++;
			}
			if ((nRoadType & ROUTE_AVOID_LADDER) == ROUTE_AVOID_LADDER) { // 7;사다리 = 100 0000 = 64
																		  //strcat(szInfo, ", ladder");
				if (cntRoadType > 0) { strcat(szInfo, ", "); }
				strcat(szInfo, "사다리");
				cntRoadType++;
			}
			if ((nRoadType & ROUTE_AVOID_RIDGE) == ROUTE_AVOID_RIDGE) { // 6:릿지 = 10 0000 = 32
																		//strcat(szInfo, ", ridge");
				if (cntRoadType > 0) { strcat(szInfo, ", "); }
				strcat(szInfo, "릿지");
				cntRoadType++;
			}
			if ((nRoadType & ROUTE_AVOID_ROCK) == ROUTE_AVOID_ROCK) { // 5:암릉 = 1 0000 = 16
																	  //strcat(szInfo, ", rocky hill");
				if (cntRoadType > 0) { strcat(szInfo, ", "); }
				strcat(szInfo, "암릉");
				cntRoadType++;
			}
			if ((nRoadType & ROUTE_AVOID_BRIDGE) == ROUTE_AVOID_BRIDGE) { // 4:교량 = 1000 = 8
																		  //strcat(szInfo, ", bridge");
				if (cntRoadType > 0) { strcat(szInfo, ", "); }
				strcat(szInfo, "교량");
				cntRoadType++;
			}
			if ((nRoadType & ROUTE_AVOID_STAIRS) == ROUTE_AVOID_STAIRS) { // 3:계단 = 100 = 4
																		  //strcat(szInfo, ", stairs");
				if (cntRoadType > 0) { strcat(szInfo, ", "); }
				strcat(szInfo, "계단");
				cntRoadType++;
			}
			if ((nRoadType & ROUTE_AVOID_PAVE) == ROUTE_AVOID_PAVE) { // 2:포장길 = 10 = 2
																	  //strcat(szInfo, ", pave");
				if (cntRoadType > 0) { strcat(szInfo, ", "); }
				strcat(szInfo, "포장길");
				cntRoadType++;
			}
			if ((nRoadType & ROUTE_AVOID_ALLEY) == ROUTE_AVOID_ALLEY) { // 1:오솔길 = 1
																		//strcat(szInfo, ", alley");
				if (cntRoadType > 0) { strcat(szInfo, ", "); }
				strcat(szInfo, "오솔길");
				cntRoadType++;
			}
		}
	}
	else if (pLink->link_type == TYPE_DATA_PEDESTRIAN)
	{
		// 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
		if (pLink->ped.facility_type == 1) {
			strcat(szInfo, "토끼굴");
		}
		else if (pLink->ped.facility_type == 2) {
			strcat(szInfo, "지하보도");
		}
		else if (pLink->ped.facility_type == 3) {
			strcat(szInfo, "육교");
		}
		else if (pLink->ped.facility_type == 4) {
			strcat(szInfo, "고가도로");
		}
		else if (pLink->ped.facility_type == 5) {
			strcat(szInfo, "교량");
		}
		else if (pLink->ped.facility_type == 6) {
			strcat(szInfo, "지하철역");
		}
		else if (pLink->ped.facility_type == 7) {
			strcat(szInfo, "철도");
		}
		else if (pLink->ped.facility_type == 8) {
			strcat(szInfo, "중앙버스정류장");
		}
		else if (pLink->ped.facility_type == 9) {
			strcat(szInfo, "지하상가");
		}
		else if (pLink->ped.facility_type == 10) {
			strcat(szInfo, "건물관통도로");
		}
		else if (pLink->ped.facility_type == 11) {
			strcat(szInfo, "단지도로_공원");
		}
		else if (pLink->ped.facility_type == 12) {
			strcat(szInfo, "단지도로_주거시설");
		}
		else if (pLink->ped.facility_type == 13) {
			strcat(szInfo, "단지도로_관광지");
		}
		else if (pLink->ped.facility_type == 14) {
			strcat(szInfo, "단지도로_기타");
		}
		else {
			strcat(szInfo, "미정의");
		}
	}

	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);




	if (pLink->link_type == TYPE_DATA_PEDESTRIAN)
	{
		nOffY += nGapY;
		sprintf(szInfo, "Lane Cnt: %d", pLink->ped.lane_count);

		pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
		g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);



		// 진입로 타입, 0:미정의, 1:경사로, 2:계단, 3:에스컬레이터, 4:계단/에스컬레이터, 5:엘리베이터, 6:단순연결로, 7:횡단보도, 8:무빙워크, 9:징검다리, 10:의사횡단
		nOffY += nGapY;
		sprintf(szInfo, "Entrance Type: ");

		if (pLink->ped.gate_type == 1) {
			strcat(szInfo, "경사로");
		}
		else if (pLink->ped.gate_type == 2) {
			strcat(szInfo, "계단");
		}
		else if (pLink->ped.gate_type == 3) {
			strcat(szInfo, "에스컬레이터");
		}
		else if (pLink->ped.gate_type == 4) {
			strcat(szInfo, "계단/에스컬레이터");
		}
		else if (pLink->ped.gate_type == 5) {
			strcat(szInfo, "엘리베이터");
		}
		else if (pLink->ped.gate_type == 6) {
			strcat(szInfo, "단순연결로");
		}
		else if (pLink->ped.gate_type == 7) {
			strcat(szInfo, "횡단보도");
		}
		else if (pLink->ped.gate_type == 8) {
			strcat(szInfo, "무빙워크");
		}
		else if (pLink->ped.gate_type == 9) {
			strcat(szInfo, "징검다리");
		}
		else if (pLink->ped.gate_type == 10) {
			strcat(szInfo, "의사횡단");
		}
		else {
			strcat(szInfo, "미정의");
		}

		pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
		g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);
	}

}


void CDrawing::DrawRouteInfo(IN const RouteResultInfo* pRouteResult, IN const uint32_t nWidth, IN const uint32_t nHeight)
{
	if (pRouteResult == nullptr) {
		return;
	}

	int32_t nStartX = nWidth - 300;
	int32_t nStartY = 0;
	// draw bg box
	g_UniDib.FillRect(nStartX, nStartY, 300, 350, 100, 100, 100, 200);

	// darw center coordnate
	char szInfo[64] = { 0, };
	int nFontHeight = 20;
	int retWidth = 0;
	int retHeight = 0;
	int nOffY = 50;
	int nGapX = 20;
	int nGapY = 40;
	unsigned char* pCoord;

	sprintf(szInfo, "Routing Result");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);


	// dist
	nOffY += nGapY;
	if (pRouteResult->TotalLinkDist > 1000) {
		sprintf(szInfo, "Dist: %dkm %dm", pRouteResult->TotalLinkDist / 1000, pRouteResult->TotalLinkDist % 1000);
	}
	else {
		sprintf(szInfo, "Dist: %dm", pRouteResult->TotalLinkDist);
	}
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);


	// eta
	nOffY += nGapY;
	if (pRouteResult->TotalLinkTime >= 3600) {
		sprintf(szInfo, "ETA: %dhr %dmin", pRouteResult->TotalLinkTime / 3600, pRouteResult->TotalLinkTime % 3600 / 60);
	}
	else {
		sprintf(szInfo, "ETA: %dmin", pRouteResult->TotalLinkTime / 60);
	}
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);


	// start coord
	nOffY += nGapY;
	sprintf(szInfo, "Start Lng: %.5f", pRouteResult->StartCoord.x);
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += 30;
	sprintf(szInfo, "Start Lat: %.5f", pRouteResult->StartCoord.y);
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);


	// end coord
	nOffY += nGapY;
	sprintf(szInfo, "End Lng : %.5f", pRouteResult->EndCoord.x);
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += 30;
	sprintf(szInfo, "End Lat : %.5f", pRouteResult->EndCoord.y);
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);
}


void CDrawing::DrawColorInfo(IN const int type, IN const uint32_t nWidth, IN const uint32_t nHeight)
{
	int32_t nStartX = 0;
	int32_t nStartY = nHeight - 200;
	// draw bg box
	g_UniDib.FillRect(nStartX, nStartY, nWidth - 300, 200, 100, 100, 100, 200);

	// darw center coordnate
	char szInfo[64] = { 0, };
	int nFontHeight = 20;
	int retWidth = 0;
	int retHeight = 0;
	int nOffY = 10;
	int nGapX = 80;
	int nGapY = 30;
	int nGapImgX = 20;
	int nGapImgY = 15;
	int nImgWidth = 30;
	float fImgHeight = 3.f;
	unsigned char* pCoord;
	unsigned char a = 255;
	color32_t col;

	// course type
	sprintf(szInfo, "Course Type: ");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY, pCoord, retWidth, retHeight, 255, 255, 0);

	// 0:미정의, 1 : 등산로, 2 : 둘레길, 3 : 자전거길, 4 : 종주코스

	nOffY += nGapY; nGapImgY += nGapY;
	col.rgba = RGBA_TRE_HIKING;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "등산로");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_TRAIL;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "둘레길");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_BIKE;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "자전거길");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	// next filed
	nOffY = 10;
	nGapX += 150;
	nGapY = 30;
	nGapImgX += 150;
	nGapImgY = 15;
	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_CROSS;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "종주코스");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_DEFAULT;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "미정의");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);



	// next filed
	nOffY = 10;
	nGapX += 200;
	nGapY = 30;
	nGapImgX += 200;
	nGapImgY = 15;

	// road type
	sprintf(szInfo, "Road Type: ");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY, pCoord, retWidth, retHeight, 255, 255, 0);

	// 노면정보 코드, 0:기타, 1:오솔길, 2:포장길, 3:계단, 4:교량, 5:암릉, 6:릿지, 7;사다리, 8:밧줄, 9:너덜길, 10:야자수매트, 11:데크로드

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_ROAD_ALLEY;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "오솔길");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_ROAD_PAVE;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "포장길");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_ROAD_STAIRS;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "계단");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);


	// next filed
	nOffY = 10;
	nGapX += 150;
	nGapY = 30;
	nGapImgX += 150;
	nGapImgY = 15;

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_ROAD_BRIDGE;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "교량");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_ROAD_ROCK;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "암릉");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_ROAD_RIDGE;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "릿지");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);


	// next filed
	nOffY = 10;
	nGapX += 150;
	nGapY = 30;
	nGapImgX += 150;
	nGapImgY = 15;

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_ROAD_LADDER;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "사다리");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_ROAD_ROPE;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "밧줄");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_ROAD_TATTERED;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "너덜길");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);


	// next filed
	nOffY = 10;
	nGapX += 150;
	nGapY = 30;
	nGapImgX += 150;
	nGapImgY = 15;

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_ROAD_PALM;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "야자수매트");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);

	nOffY += nGapY;	nGapImgY += nGapY;
	col.rgba = RGBA_TRE_ROAD_DECK;
	g_UniDib.ThickLine(nStartX + nGapImgX, nStartY + nGapImgY, nStartX + nGapImgX + nImgWidth, nStartY + nGapImgY, fImgHeight, col.r, col.g, col.b, col.a);
	sprintf(szInfo, "데크로드");
	pCoord = g_pFontMgr.GetTextImage(szInfo, nFontHeight, &retWidth, &retHeight);
	g_UniDib.BltFromAlpha(nStartX + nGapX, nStartY + nOffY, pCoord, retWidth, retHeight, 255, 255, 0);
}