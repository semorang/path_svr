#include "../include/MapDef.h"

#include "RoutePackage.h"

#if defined(USE_CJSON)
#include "../libjson/cjson/cJSON.h"
#endif

#include "../utils/Strings.h"
#include "../utils/UserLog.h"
#include "../utils/GeoTools.h"


template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}


CRoutePackage::CRoutePackage()
{
	m_pDataMgr = nullptr;
}


CRoutePackage::~CRoutePackage()
{
	Release();
}


bool CRoutePackage::Initialize(void)
{
	return true;
}


void CRoutePackage::Release(void)
{

}


void CRoutePackage::SetDataMgr(IN CDataManager* pDataMgr)
{
	if (pDataMgr) {
		m_pDataMgr = pDataMgr;
	}
}


void CRoutePackage::GetErrorResult(IN const int32_t err_code,  OUT string& strJson)
{
	// string strJson;

#if defined(USE_CJSON)
	cJSON* root = cJSON_CreateObject();

	string err_msg;

	switch (err_code) {
	case ROUTE_RESULT_SUCCESS:
		err_msg = "성공";
		break;
	case ROUTE_RESULT_FAILED:
		err_msg = "탐색 실패(내부 오류에 의한 실패)";
		break;
	case ROUTE_RESULT_FAILED_SAME_ROUTE:
		err_msg = "스마트 재탐색 적용(기존 경로와 동일)";
	case ROUTE_RESULT_FAILED_WRONG_PARAM:
		err_msg = "잘못된 파라미터(필수 파라미터 체크)";
		break;
	case ROUTE_RESULT_FAILED_SET_MEMORY:
		err_msg = "탐색 확장 관련 메모리 할당 오류(탐색 초기화 관련)";
		break;
	case ROUTE_RESULT_FAILED_READ_DATA:
		err_msg = "탐색 관련 데이터(지도, 옵션 등) 파일 읽기 실패(탐색 초기화 관련)";
		break;
	case ROUTE_RESULT_FAILED_SET_START:
		err_msg = "출발지가 프로젝션이 안되거나, 잘못된 출발지";
		break;
	case ROUTE_RESULT_FAILED_SET_VIA:
		err_msg = "경유지가 프로젝션이 안되거나, 잘못된 경유지";
		break;
	case ROUTE_RESULT_FAILED_SET_END:
		err_msg = "목적지가 프로젝션이 안되거나, 잘못된 목적지";
		break;
	case ROUTE_RESULT_FAILED_DIST_OVER:
		err_msg = "탐색 가능 거리 초과";
		break;
	case ROUTE_RESULT_FAILED_TIME_OVER:
		err_msg = "탐색 시간 초과(10초 이상)";
		break;
	case ROUTE_RESULT_FAILED_NODE_OVER:
		err_msg = "확장 가능 Node 개수 초과";
		break;
	case ROUTE_RESULT_FAILED_EXPEND:
		err_msg = "확장 실패";
		break;
	default:
		err_msg = "실패(알수 없는 오류)";
		break;
	}

	if (1) {
		cJSON_AddNumberToObject(root, "result_code", err_code);

		cJSON_AddStringToObject(root, "isSuccessful", "failed");
		cJSON_AddNumberToObject(root, "resultCode", err_code);
		cJSON_AddStringToObject(root, "resultMessage", err_msg.c_str());
	} else {
		cJSON_AddStringToObject(root, "isSuccessful", "failed");
		cJSON_AddNumberToObject(root, "resultCode", err_code);
		cJSON_AddStringToObject(root, "resultMessage", err_msg.c_str());
	}

	char* pJson = cJSON_Print(root);
	strJson.append(pJson);
	cJSON_free(pJson);

	cJSON_Delete(root);
#endif // #if defined(USE_CJSON)
}



bool CRoutePackage::GetRouteResultJson(IN const RouteResultInfo* pResult, IN const time_t time, IN const bool isJunction, OUT void* pJson)
{
	if (pResult == nullptr || pJson == nullptr) {
		return false;
	}

	uint32_t request_id = pResult->RequestId;
	uint32_t result_code = pResult->ResultCode;

	time_t timer;
	struct tm* tmNow;
	string strEta;

#if defined(USE_CJSON)
	// route
	cJSON* route = reinterpret_cast<cJSON*>(pJson);// cJSON_CreateObject();


	// - summary
	cJSON* summary = cJSON_CreateObject();
	// - type
	cJSON_AddItemToObject(summary, "option", cJSON_CreateNumber(pResult->RouteOption));
	// - distance
	cJSON_AddItemToObject(summary, "distance", cJSON_CreateNumber(pResult->TotalLinkDist));
	// - time
	cJSON_AddItemToObject(summary, "time", cJSON_CreateNumber(pResult->TotalLinkTime));
	// - now
	timer = time;
	tmNow = localtime(&timer);
	strEta = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
	cJSON_AddItemToObject(summary, "now", cJSON_CreateString(strEta.c_str()));

	// eta
	timer += pResult->TotalLinkTime;
	tmNow = localtime(&timer);
	strEta = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
	cJSON_AddItemToObject(summary, "eta", cJSON_CreateString(strEta.c_str()));

	// add routes - summary
	cJSON_AddItemToObject(route, "summary", summary);


	// - link_info
	unordered_map<uint64_t, int> mapVisited; // link_id, dir
	cJSON* links = cJSON_CreateArray();
	int cntLinks = pResult->LinkInfo.size();
	int vertex_offset = 0;
	for (const auto& link : pResult->LinkInfo) {
		cJSON* idoff = cJSON_CreateObject();

		mapVisited.emplace(link.link_id.llid, link.dir);

		// 링크 부가 정보 
		uint64_t sub_info = link.link_info;
		stLinkBaseInfo* pBaseInfo = nullptr;
		if (sub_info > 0) {
			pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&sub_info);
		}
		if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
			cJSON_AddItemToObject(idoff, "link_id", cJSON_CreateNumber(link.link_id.nid));
		} else {
			cJSON_AddItemToObject(idoff, "link_id", cJSON_CreateNumber(link.link_id.llid));
		}
		cJSON_AddItemToObject(idoff, "length", cJSON_CreateNumber(link.length));
		cJSON_AddItemToObject(idoff, "time", cJSON_CreateNumber(link.time));
		cJSON_AddItemToObject(idoff, "angle", cJSON_CreateNumber(link.angle));
		cJSON_AddItemToObject(idoff, "vertex_offset", cJSON_CreateNumber(vertex_offset));
		cJSON_AddItemToObject(idoff, "vertex_count", cJSON_CreateNumber(link.vtx_cnt));
		cJSON_AddItemToObject(idoff, "remain_distance", cJSON_CreateNumber(link.rlength));
		cJSON_AddItemToObject(idoff, "remain_time", cJSON_CreateNumber(link.rtime));
		cJSON_AddItemToObject(idoff, "guide_type", cJSON_CreateNumber(link.guide_type));

		if (link.vtx_cnt > 0) {
			vertex_offset += link.vtx_cnt; // 경유지에서 offset 초기화 되지 않도록
		}

		if (pBaseInfo != nullptr) {
			// 종단 노드 정보
			stLinkInfo* pLink = m_pDataMgr->GetLinkDataById(link.link_id, pBaseInfo->link_type);
			KeyID first_node_key;
			KeyID last_node_key;
			if (pLink) {
				uint64_t last_node_id = 0;
				uint32_t last_node_nid = 0;
				if (link.dir == 0) { // 정방향 enode가 종단 노드
					first_node_key = pLink->snode_id;
					last_node_key = pLink->enode_id;
					if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
						last_node_id = pLink->enode_id.nid;
					} else {
						last_node_id = pLink->enode_id.llid;
					}
					last_node_nid = pLink->enode_id.nid;
				}
				else { // 역방향 snode가 종단 노드
					first_node_key = pLink->enode_id;
					last_node_key = pLink->snode_id;
					if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
						last_node_id = pLink->snode_id.nid;
					} else {
						last_node_id = pLink->snode_id.llid;
					}
					last_node_nid = pLink->snode_id.nid;
				}
				cJSON_AddItemToObject(idoff, "node_id", cJSON_CreateNumber(last_node_id));

				stNodeInfo* pNode = m_pDataMgr->GetNodeDataById(last_node_key, pBaseInfo->link_type);
				if (pNode) {
					cJSON_AddItemToObject(idoff, "conn_count", cJSON_CreateNumber(pNode->base.connnode_count));
				}
				//if (link.type == LINK_GUIDE_TYPE_DEPARTURE) { // 시작점은 항상 노드 연결이 2개다.
				//	cJSON_AddItemToObject(idoff, "conn_count", cJSON_CreateNumber(2));
				//}
				//else {
				//	stNodeInfo* pNode = m_pDataMgr->GetNodeDataById(link.node_id, pBaseInfo->link_type);
				//	cJSON_AddItemToObject(idoff, "conn_count", cJSON_CreateNumber(pNode->base.connnode_count));
				//}
			}
#if defined(USE_FOREST_DATA)
			cJSON_AddItemToObject(idoff, "link_type", cJSON_CreateNumber(pBaseInfo->link_type));
#endif
			// 숲길 데이터
			if (pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
				stLinkTrekkingInfo* pForestInfo = reinterpret_cast<stLinkTrekkingInfo*>(&sub_info);
				cJSON_AddItemToObject(idoff, "course_type", cJSON_CreateNumber(pForestInfo->course_type));
				cJSON_AddItemToObject(idoff, "road_type", cJSON_CreateNumber(pForestInfo->road_info));
				cJSON_AddItemToObject(idoff, "difficult", cJSON_CreateNumber(pForestInfo->diff));
				cJSON_AddItemToObject(idoff, "popular", cJSON_CreateNumber(pForestInfo->pop_grade));
				cJSON_AddItemToObject(idoff, "legal", cJSON_CreateNumber(pForestInfo->legal));
				// 고도(altitude)
				if ((link.link_id.llid != NOT_USE) && (pLink != nullptr)) {
					stNodeInfo* pSNode = m_pDataMgr->GetFNodeDataById(pLink->snode_id);
					stNodeInfo* pENode = m_pDataMgr->GetFNodeDataById(pLink->enode_id);

					if (link.dir == 0) { // 정
						cJSON_AddItemToObject(idoff, "slop", cJSON_CreateNumber(static_cast<int8_t>(pForestInfo->slop)));
						cJSON_AddItemToObject(idoff, "alt_s", cJSON_CreateNumber(static_cast<int16_t>(pSNode->trk.z_value)));
						cJSON_AddItemToObject(idoff, "alt_e", cJSON_CreateNumber(static_cast<int16_t>(pENode->trk.z_value)));
					}
					else {
						cJSON_AddItemToObject(idoff, "slop", cJSON_CreateNumber(static_cast<int8_t>(pForestInfo->slop)  * -1));
						cJSON_AddItemToObject(idoff, "alt_s", cJSON_CreateNumber(static_cast<int16_t>(pENode->trk.z_value)));
						cJSON_AddItemToObject(idoff, "alt_e", cJSON_CreateNumber(static_cast<int16_t>(pSNode->trk.z_value)));
					}
				}
				// 서브코스 정보
#if defined(USE_MOUNTAIN_DATA)
				if ((pLink != nullptr) && pLink->trk_ext.course_cnt > 0) {
					set<uint32_t>* pCourse = m_pDataMgr->GetCourseByLink(pLink->link_id.llid);
					if (pCourse != nullptr) {
						cJSON* courses = cJSON_CreateArray();
						stCourseInfo courseInfo;
						for (const auto& course_id : *pCourse) {
							cJSON* course = cJSON_CreateObject();
							courseInfo.course_id = course_id;
							cJSON_AddItemToObject(course, "type", cJSON_CreateNumber(courseInfo.course_type));
							cJSON_AddItemToObject(course, "code", cJSON_CreateNumber(courseInfo.course_cd));
							cJSON_AddItemToArray(courses, course);
						}
						cJSON_AddItemToObject(idoff, "sub_course", courses);
					}
				}
#endif // #if defined(USE_MOUNTAIN_DATA)
			}
			// 보행자 데이터
			else if (pBaseInfo->link_type == TYPE_LINK_DATA_PEDESTRIAN) {
				stLinkPedestrianInfo* pPedInfo = reinterpret_cast<stLinkPedestrianInfo*>(&sub_info);
				cJSON_AddItemToObject(idoff, "walk_type", cJSON_CreateNumber(pPedInfo->walk_type));
				cJSON_AddItemToObject(idoff, "bicycle_type", cJSON_CreateNumber(pPedInfo->bicycle_type));
				cJSON_AddItemToObject(idoff, "facility_type", cJSON_CreateNumber(pPedInfo->facility_type));
				cJSON_AddItemToObject(idoff, "gate_type", cJSON_CreateNumber(pPedInfo->gate_type));

				// 입출구 정보
				stNodeInfo* pNode = m_pDataMgr->GetNodeDataById(first_node_key, pBaseInfo->link_type);
				if (pNode && pNode->ped.gate_info) {
					char szIO[MAX_PATH] = { 0, };
					uint32_t fctNameIdx = 0;
					uint32_t ioNameIdx = static_cast<uint32_t>(m_pDataMgr->GetExtendDataById(first_node_key, TYPE_WNODEEX_IO, TYPE_KEY_NODE));
					if (pNode && pNode->ped.fct_info) {
						fctNameIdx = static_cast<uint32_t>(m_pDataMgr->GetExtendDataById(first_node_key, TYPE_WNODEEX_FCT, TYPE_KEY_NODE));
					}
					if (fctNameIdx && ioNameIdx) {
						sprintf(szIO, "%s %s", m_pDataMgr->GetNameDataByIdx(fctNameIdx), m_pDataMgr->GetNameDataByIdx(ioNameIdx));
					} else if (ioNameIdx) {
						sprintf(szIO, "%s", m_pDataMgr->GetNameDataByIdx(ioNameIdx));
					}
					if (strlen(szIO) > 0) {
#if defined(_WIN32)
						char szUTF8[MAX_PATH] = { 0, };
						MultiByteToUTF8(szIO, szUTF8);
						cJSON_AddStringToObject(idoff, "io_name", szUTF8);
#else
						cJSON_AddStringToObject(idoff, "io_name", encoding(szIO, "euc-kr", "utf-8"));
#endif // #if defined(_WIN32)
						//cJSON_AddItemToObject(idoff, "io_name", cJSON_CreateString(szIO));						
					}
				}
			}
			else {
				cJSON_AddItemToObject(idoff, "facility_type", cJSON_CreateNumber(9));
				cJSON_AddItemToObject(idoff, "gate_type", cJSON_CreateNumber(9));

#if defined(USE_VEHICLE_DATA)
// 경로 생성 시점의 정보를 재계산 하느냐, 확장 시점의 정보를 쓰느냐
				uint8_t curSpeed = 255;
				uint8_t curSpeedType = 0;
#if 1 // 우선은 확장 시점 정보 쓰자
				curSpeed = link.speed;
				curSpeedType = link.speed_type;
#else 
				curSpeed = m_pDataMgr->GetTrafficTTLSpeed(pLinkPrev->link_id, dirTarget);
				if (curSpeed != 255) {
					curSpeedType = TYPE_TRAFFIC_TTL;
				}
#endif
				cJSON_AddItemToObject(idoff, "speed", cJSON_CreateNumber(curSpeed));
				cJSON_AddItemToObject(idoff, "speed_type", cJSON_CreateNumber(curSpeedType));
#endif // #if defined(USE_VEHICLE_DATA)
			}
		}
		cJSON_AddItemToArray(links, idoff);
	} // for

	  // add routes - link_info
	cJSON_AddItemToObject(route, "link_info", links);


	// - vertex_info
	cJSON* vertices = cJSON_CreateArray();
	for (const auto& vtx : pResult->LinkVertex) {
		cJSON* lnglat = cJSON_CreateObject();
		cJSON_AddItemToObject(lnglat, "x", cJSON_CreateNumber(vtx.x));
		cJSON_AddItemToObject(lnglat, "y", cJSON_CreateNumber(vtx.y));
		cJSON_AddItemToArray(vertices, lnglat);

		// Local<Array> lnglat = Array::New(isolate);
		// lnglat->Set(context, 0, Number::New(isolate, pResult->LinkVertex[ii].x));
		// lnglat->Set(context, 1, Number::New(isolate, pResult->LinkVertex[ii].y));
		// coords->Set(context, ii, lnglat);
	}
	// add routes - vertex_info
	cJSON_AddItemToObject(route, "vertex_info", vertices);


	// junction_info
	if (isJunction == true) {
		cJSON* junctions = cJSON_CreateArray();

#if defined(TARGET_FOR_KAKAO_VX)
		vector<RouteProbablePath*> vtRpp;

		int cntRpp = GetRouteProbablePath(mapVisited, pResult->LinkInfo, vtRpp);
		for (auto jj = 0; jj < cntRpp; jj++) {
			cJSON* link_info = cJSON_CreateObject();
			cJSON* links = cJSON_CreateArray();
			RouteProbablePath* pRpp = vtRpp[jj];
			// for (auto kk = 0; kk<pRpp->JctLinks.size(); kk++) {
			for (auto const& jct : pRpp->JctLinks) {
				cJSON* jct_info = cJSON_CreateObject();
				cJSON* link_vtx = cJSON_CreateArray();
				stLinkInfo* pLink = jct.second;
				for (auto ll = 0; ll < pLink->getVertexCount(); ll++) {
					cJSON* lnglat = cJSON_CreateObject();
					cJSON_AddItemToObject(lnglat, "x", cJSON_CreateNumber(pLink->getVertexX(ll)));
					cJSON_AddItemToObject(lnglat, "y", cJSON_CreateNumber(pLink->getVertexY(ll)));
					cJSON_AddItemToArray(link_vtx, lnglat);
				} // for vtx
				cJSON_AddItemToObject(jct_info, "id", cJSON_CreateNumber(pLink->link_id.nid));
				cJSON_AddItemToObject(jct_info, "vertices", link_vtx);
				cJSON_AddItemToArray(links, jct_info);
			} // for links
			cJSON_AddItemToObject(link_info, "id", cJSON_CreateNumber(pRpp->LinkId.nid));
#if defined(TARGET_FOR_KAKAO_VX)
			if (pRpp->NodeDir == 0) { // 정
				cJSON_AddItemToObject(link_info, "node_id", cJSON_CreateNumber(pRpp->ENodeId.nid));
			} else {
				cJSON_AddItemToObject(link_info, "node_id", cJSON_CreateNumber(pRpp->SNodeId.nid));
			}
#else			
			cJSON_AddItemToObject(link_info, "snode_id", cJSON_CreateNumber(pRpp->SNodeId.nid));
			cJSON_AddItemToObject(link_info, "enode_id", cJSON_CreateNumber(pRpp->ENodeId.nid));
#endif
			cJSON_AddItemToObject(link_info, "junction", links);
			cJSON_AddItemToArray(junctions, link_info);

			//release
			SAFE_DELETE(pRpp);
		} // for junctions

#else
		vector<RouteProbablePath*> vtRpp;
		int cntRpp = GetRouteProbablePathEx(&mapVisited, 0, 0, 100, vtRpp);
		stLinkInfo* pLink = nullptr;

		for (const auto& link : vtRpp) {
			//KeyID key = { key.llid = linkId };
			//pLink = m_pDataMgr->GetLinkDataById(key, TYPE_LINK_DATA_NONE);

			// ex에서는 1개만 들어있을 거임.
			pLink = link->JctLinks.begin()->second;

			if (pLink == nullptr) {
				continue;
			}

			// 링크 부가 정보 
			stLinkBaseInfo* pBaseInfo = nullptr;
			if (pLink->sub_info > 0) {
				pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&pLink->sub_info);
			}

			cJSON* link_info = cJSON_CreateObject();
			if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
				// link_id
				cJSON_AddItemToObject(link_info, "link_id", cJSON_CreateNumber(pLink->link_id.nid));
				// s_node_id
				cJSON_AddItemToObject(link_info, "snode_id", cJSON_CreateNumber(pLink->snode_id.nid));
				// e_node_id
				cJSON_AddItemToObject(link_info, "enode_id", cJSON_CreateNumber(pLink->enode_id.nid));
			}
			else {
				// link_id
				cJSON_AddItemToObject(link_info, "link_id", cJSON_CreateNumber(pLink->link_id.llid));
				// s_node_id
				cJSON_AddItemToObject(link_info, "snode_id", cJSON_CreateNumber(pLink->snode_id.llid));
				// e_node_id
				cJSON_AddItemToObject(link_info, "enode_id", cJSON_CreateNumber(pLink->enode_id.llid));
			}

			// vertex
			cJSON* vertices = cJSON_CreateArray();
			for (auto ll = 0; ll<pLink->getVertexCount(); ll++) {
				cJSON* lnglat = cJSON_CreateObject();
				cJSON_AddItemToObject(lnglat, "x", cJSON_CreateNumber(pLink->getVertexX(ll)));
				cJSON_AddItemToObject(lnglat, "y", cJSON_CreateNumber(pLink->getVertexY(ll)));
				cJSON_AddItemToArray(vertices, lnglat);
			} // for vtx
			cJSON_AddItemToObject(link_info, "vertices", vertices);

			cJSON_AddItemToArray(junctions, link_info);
		} // for jct
#endif

		  // add routes - junction_info
		cJSON_AddItemToObject(route, "junction_info", junctions);
	} // if (isJunction == true)
#endif // #if defined(USE_CJSON)

	return true;
}


bool CRoutePackage::GetMapsRouteResultJson(IN const RouteResultInfo* pResult, IN const time_t time, OUT void* pJson)
{
	if (pResult == nullptr || pJson == nullptr) {
		return false;
	}

	uint32_t request_id = pResult->RequestId;
	uint32_t result_code = pResult->ResultCode;

	time_t timer;
	struct tm* tmNow;
	string strEta;

#if defined(USE_CJSON)
	// route
	cJSON* route = reinterpret_cast<cJSON*>(pJson);// cJSON_CreateObject();

	cJSON_AddItemToObject(route, "option", cJSON_CreateNumber(pResult->RouteOption));
	cJSON_AddItemToObject(route, "spend_time", cJSON_CreateNumber(pResult->TotalLinkTime));
	cJSON_AddItemToObject(route, "distance", cJSON_CreateNumber(pResult->TotalLinkDist));
	cJSON_AddItemToObject(route, "toll_fee", cJSON_CreateNumber(0));
	cJSON_AddItemToObject(route, "taxiFare", cJSON_CreateNumber(0));
	cJSON_AddItemToObject(route, "isHighWay", cJSON_CreateBool(false));

	// 경로 정보 (Array)
	cJSON* paths = cJSON_CreateArray();
	stLinkInfo* pLink = nullptr;
	stLinkVehicleInfo vehInfo;
	char szBuff[MAX_PATH] = { 0, };

	int vertex_offset = 0;

	for (const auto& link : pResult->LinkInfo) {
		// 경로 링크 정보
		cJSON* path = cJSON_CreateObject();

		// 경로선 (Array)
		cJSON* coords = cJSON_CreateArray();
		int vtxCount = link.vtx_cnt;

		memcpy(&vehInfo, &link.link_info, sizeof(vehInfo));

		// 경로선 확장
		for (int ii = 0; ii < vtxCount; ii++) {
			// 남은 거리로 vertex 확인

			cJSON* coord = cJSON_CreateObject();
			cJSON_AddItemToObject(coord, "x", cJSON_CreateNumber(pResult->LinkVertex[vertex_offset].x));
			cJSON_AddItemToObject(coord, "y", cJSON_CreateNumber(pResult->LinkVertex[vertex_offset].y));

			vertex_offset++;

			// add coord to coords
			cJSON_AddItemToArray(coords, coord);
		} // for

		  // add coords to path
		cJSON_AddItemToObject(path, "coords", coords);

		// speed
		cJSON_AddNumberToObject(path, "speed", 0);

		// time
		cJSON_AddNumberToObject(path, "spend_time", link.time);

		// distance
		cJSON_AddNumberToObject(path, "distance", link.length);

		// road_code
		cJSON_AddNumberToObject(path, "road_code", vehInfo.road_type);

		// traffic_color
		cJSON_AddStringToObject(path, "traffic_color", "green");

#if defined(USE_P2P_DATA) // P2P HD 매칭을 위한 SD 링크 ID 정보
		pLink = m_pDataMgr->GetVLinkDataById(link.link_id);
		if (pLink != nullptr && link.link_id.llid != NULL_VALUE) {
			// road_name
			if (pLink->name_idx > 0) {
#if defined(_WIN32)
				char szUTF8[MAX_PATH] = { 0, };
				MultiByteToUTF8(m_pDataMgr->GetNameDataByIdx(pLink->name_idx), szUTF8);
				cJSON_AddStringToObject(path, "road_name", szUTF8);
#else
				cJSON_AddStringToObject(path, "road_name", encoding(m_pDataMgr->GetNameDataByIdx(pLink->name_idx), "euc-kr", "utf-8"));
#endif // #if defined(_WIN32)
			}

			// p2p 추가정보
			cJSON* p2p = cJSON_CreateObject();

			// speed 재설정
			cJSON_SetNumberHelper(cJSON_GetObjectItem(path, "speed"), pLink->veh.speed_f);

			// hd matching link id
			// LOG_TRACE(LOG_DEBUG, "tile:%d, id:%d, snode:%d, enode:%d",pLink->link_id.tile_id, pLink->link_id.nid, pLink->snode_id.nid, pLink->enode_id.nid);
			sprintf(szBuff, "%d%06d%06d", pLink->link_id.tile_id, pLink->snode_id.nid, pLink->enode_id.nid);
			// cJSON_AddNumberToObject(p2p, "link_id", (pLink->link_id.tile_id * 1000000000000) + (pLink->snode_id.nid * 1000000) + pLink->enode_id.nid); // 원본 ID 사용, (snode 6자리 + enode 6자리)
			cJSON_AddStringToObject(p2p, "link_id", szBuff);

			// dir, 0:정방향, 1:역방향
			cJSON_AddNumberToObject(p2p, "dir", link.dir);

			// type 링크 타입, 0:일반, 1:출발지링크, 2:경유지링크, 3:도착지링크
			cJSON_AddNumberToObject(p2p, "guide_type", link.guide_type);

			// ang, 진행각
			cJSON_AddNumberToObject(p2p, "angle", link.angle);

			// add new coordinate for air navigation
			// 링크 중심 좌표에서 진행방향 우측으로 이격된 좌표를 제공
			SPoint ptFirst, ptLast, ptNav = { 0.f, };
			if (vtxCount > 2) {
				ptFirst = pResult->LinkVertex[link.vtx_off + (vtxCount / 2)];
				ptLast = pResult->LinkVertex[link.vtx_off + (vtxCount / 2) + 1];
			} else {
				ptFirst = pResult->LinkVertex[link.vtx_off];
				ptLast = pResult->LinkVertex[link.vtx_off + 1];
			}

			// 도로로부터 지정된거리(도로너비) 만큼 띄워 도로변으로 위치시켜 주자
			static const double dwDist = 0.000001f; // 10cm
			static const bool isRight = true;
			cJSON* nav_coord = cJSON_CreateObject();
			if (getPointByDistanceFromCenter(ptFirst.x, ptFirst.y, ptLast.x, ptLast.y, dwDist, isRight, ptNav.x, ptNav.y) == true) {
				cJSON_AddItemToObject(nav_coord, "x", cJSON_CreateNumber(ptNav.x));
				cJSON_AddItemToObject(nav_coord, "y", cJSON_CreateNumber(ptNav.y));
			} else {
				cJSON_AddItemToObject(nav_coord, "x", cJSON_CreateNumber(ptFirst.x));
				cJSON_AddItemToObject(nav_coord, "y", cJSON_CreateNumber(ptFirst.y));
			}
			// 내비게이션용 경유지 좌표
			cJSON_AddItemToObject(p2p, "nav_coord", nav_coord);

			// add p2p to path
			cJSON_AddItemToObject(path, "p2p_extend", p2p);
		}
#endif // #if 1 defined(USE_P2P_DATA)

		// add path to paths
		cJSON_AddItemToArray(paths, path);

	} // for paths

	  // add paths to data
	cJSON_AddItemToObject(route, "paths", paths);
	
#endif // #if defined(USE_CJSON)

	return true;
}


void CRoutePackage::GetRouteResult(IN const RouteResultInfo* pResult, IN const bool isJunction, OUT string& strJson)
{
	int result_code = ROUTE_RESULT_FAILED;

#if defined(USE_CJSON)
	cJSON* root = cJSON_CreateObject();

	if (pResult == nullptr) {
		cJSON_AddItemToObject(root, "user_id", cJSON_CreateNumber(0));
		cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(ROUTE_RESULT_FAILED));
		cJSON_AddItemToObject(root, "error_msg", cJSON_CreateString("failed"));
	}
	else if (pResult->ResultCode != ROUTE_RESULT_SUCCESS) {
		// header
		cJSON_AddItemToObject(root, "user_id", cJSON_CreateNumber(pResult->RequestId));
		cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(pResult->ResultCode));
		cJSON_AddItemToObject(root, "error_msg", cJSON_CreateString("failed"));
	}
	else {
		// header
		cJSON_AddItemToObject(root, "user_id", cJSON_CreateNumber(pResult->RequestId));
		cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(ROUTE_RESULT_SUCCESS));
		cJSON_AddItemToObject(root, "error_msg", cJSON_CreateString("success"));

		// summarys
		// cJSON* summarys = cJSON_CreateArray();
		int cntRoutes = pResult->RouteSummarys.size();

		// - now
		time_t timer = time(NULL);
		//struct tm* tmNow;
		string strEta;

		// routes
		cJSON* routes = cJSON_CreateArray();

		for (int ii = 0; ii<cntRoutes; ii++) {
			cJSON* route = cJSON_CreateObject();

#if 1
			GetRouteResultJson(pResult, timer, isJunction, route);
#else
			// - summary
			cJSON* summary = cJSON_CreateObject();
			// - type
			cJSON_AddItemToObject(summary, "option", cJSON_CreateNumber(pResult->RouteOption));
			// - distance
			cJSON_AddItemToObject(summary, "distance", cJSON_CreateNumber(pResult->TotalLinkDist));
			// - time
			cJSON_AddItemToObject(summary, "time", cJSON_CreateNumber(pResult->TotalLinkTime));
			// - now
			timer = time(NULL);
			tmNow = localtime(&timer);
			strEta = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
			cJSON_AddItemToObject(summary, "now", cJSON_CreateString(strEta.c_str()));

			// eta
			timer += pResult->TotalLinkTime;
			tmNow = localtime(&timer);
			strEta = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
			cJSON_AddItemToObject(summary, "eta", cJSON_CreateString(strEta.c_str()));

			// add routes - summary
			cJSON_AddItemToObject(route, "summary", summary);
			
			unordered_set<uint64_t> setVisited;

			// - link_info
			cJSON* links = cJSON_CreateArray();
			int cntLinks = pResult->LinkInfo.size();
			int vertex_offset = 0;
			for (const auto& link : pResult->LinkInfo) {
				cJSON* idoff = cJSON_CreateObject();

				setVisited.emplace(link.link_id.llid);

				// 링크 부가 정보 
				uint64_t sub_info = link.link_info;
				stLinkBaseInfo* pBaseInfo = nullptr;
				if (sub_info > 0) {
					pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&sub_info);
				}				
				if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
					cJSON_AddItemToObject(idoff, "link_id", cJSON_CreateNumber(link.link_id.nid));
				} else {
					cJSON_AddItemToObject(idoff, "link_id", cJSON_CreateNumber(link.link_id.llid));			
				}
				cJSON_AddItemToObject(idoff, "length", cJSON_CreateNumber(link.length));
				cJSON_AddItemToObject(idoff, "time", cJSON_CreateNumber(link.time));
				cJSON_AddItemToObject(idoff, "angle", cJSON_CreateNumber(link.angle));
				cJSON_AddItemToObject(idoff, "vertex_offset", cJSON_CreateNumber(vertex_offset));
				cJSON_AddItemToObject(idoff, "vertex_count", cJSON_CreateNumber(link.vtx_cnt));
				cJSON_AddItemToObject(idoff, "remain_distance", cJSON_CreateNumber(link.rlength));
				cJSON_AddItemToObject(idoff, "remain_time", cJSON_CreateNumber(link.rtime));
				cJSON_AddItemToObject(idoff, "guide_type", cJSON_CreateNumber(link.guide_type));

				if (link.vtx_cnt > 0) {
					vertex_offset += link.vtx_cnt; // 경유지에서 offset 초기화 되지 않도록
				}

				if (pBaseInfo != nullptr) {
					// 종단 노드 정보
					stLinkInfo* pLink = m_pDataMgr->GetLinkDataById(link.link_id, pBaseInfo->link_type);
					if (pLink) {
						KeyID last_node_key;
						uint64_t last_node_id = 0;
						if (link.dir == 0) { // 정방향 enode가 종단 노드
							last_node_key = pLink->enode_id;
							if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
								last_node_id = pLink->enode_id.nid;
							} else {
								last_node_id = pLink->enode_id.llid;
							}
						}
						else { // 역방향 snode가 종단 노드
							last_node_key = pLink->snode_id;
							if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
								last_node_id = pLink->snode_id.nid;
							} else {
								last_node_id = pLink->snode_id.llid;
							}
						}
						cJSON_AddItemToObject(idoff, "node_id", cJSON_CreateNumber(last_node_id));

						stNodeInfo* pNode = m_pDataMgr->GetNodeDataById(last_node_key, pBaseInfo->link_type);
						if (pNode) {
							cJSON_AddItemToObject(idoff, "conn_count", cJSON_CreateNumber(pNode->base.connnode_count));
						}
						//if (link.type == LINK_GUIDE_TYPE_DEPARTURE) { // 시작점은 항상 노드 연결이 2개다.
						//	cJSON_AddItemToObject(idoff, "conn_count", cJSON_CreateNumber(2));
						//}
						//else {
						//	stNodeInfo* pNode = m_pDataMgr->GetNodeDataById(link.node_id, pBaseInfo->link_type);
						//	cJSON_AddItemToObject(idoff, "conn_count", cJSON_CreateNumber(pNode->base.connnode_count));
						//}
					}
#if defined(USE_FOREST_DATA)
					cJSON_AddItemToObject(idoff, "link_type", cJSON_CreateNumber(pBaseInfo->link_type));
#endif
					// 숲길 데이터
					if (pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
						stLinkTrekkingInfo* pForestInfo = reinterpret_cast<stLinkTrekkingInfo*>(&sub_info);
						cJSON_AddItemToObject(idoff, "course_type", cJSON_CreateNumber(pForestInfo->course_type));
						cJSON_AddItemToObject(idoff, "road_type", cJSON_CreateNumber(pForestInfo->road_info));
						cJSON_AddItemToObject(idoff, "difficult", cJSON_CreateNumber(pForestInfo->diff));
						cJSON_AddItemToObject(idoff, "slop", cJSON_CreateNumber(static_cast<int8_t>(pForestInfo->slop)));
						cJSON_AddItemToObject(idoff, "popular", cJSON_CreateNumber(pForestInfo->popular));
						cJSON_AddItemToObject(idoff, "legal", cJSON_CreateNumber(pForestInfo->legal));
						// 고도(altitude)
						if ((link.link_id.llid != NOT_USE) && (pLink != nullptr)) {
							stNodeInfo* pSNode = m_pDataMgr->GetFNodeDataById(pLink->snode_id);
							stNodeInfo* pENode = m_pDataMgr->GetFNodeDataById(pLink->enode_id);

							if (link.dir == 0) { // 정
								cJSON_AddItemToObject(idoff, "alt_s", cJSON_CreateNumber(static_cast<int16_t>(pSNode->trk.z_value)));
								cJSON_AddItemToObject(idoff, "alt_e", cJSON_CreateNumber(static_cast<int16_t>(pENode->trk.z_value)));
							}
							else {
								cJSON_AddItemToObject(idoff, "alt_s", cJSON_CreateNumber(static_cast<int16_t>(pENode->trk.z_value)));
								cJSON_AddItemToObject(idoff, "alt_e", cJSON_CreateNumber(static_cast<int16_t>(pSNode->trk.z_value)));
							}
						}
						// 서브코스 정보
#if defined(USE_MOUNTAIN_DATA)
						if ((pLink != nullptr) && pLink->trk_ext.course_cnt > 0) {
							set<uint32_t>* pCourse = m_pDataMgr->GetCourseByLink(pLink->link_id.llid);
							if (pCourse != nullptr) {
								cJSON* courses = cJSON_CreateArray();
								stCourseInfo courseInfo;
								for (const auto& course_id : *pCourse) {
									cJSON* course = cJSON_CreateObject();
									courseInfo.course_id = course_id;
									cJSON_AddItemToObject(course, "type", cJSON_CreateNumber(courseInfo.course_type));
									cJSON_AddItemToObject(course, "code", cJSON_CreateNumber(courseInfo.course_cd));
									cJSON_AddItemToArray(courses, course);
								}
								cJSON_AddItemToObject(idoff, "sub_course", courses);
							}
						}
#endif // #if defined(USE_MOUNTAIN_DATA)
					}
					// 보행자 데이터
					else if (pBaseInfo->link_type == TYPE_LINK_DATA_PEDESTRIAN) {
						stLinkPedestrianInfo* pPedInfo = reinterpret_cast<stLinkPedestrianInfo*>(&sub_info);
						cJSON_AddItemToObject(idoff, "walk_type", cJSON_CreateNumber(pPedInfo->walk_type));
						cJSON_AddItemToObject(idoff, "bicycle_type", cJSON_CreateNumber(pPedInfo->bicycle_type));
						cJSON_AddItemToObject(idoff, "facility_type", cJSON_CreateNumber(pPedInfo->facility_type));
						cJSON_AddItemToObject(idoff, "gate_type", cJSON_CreateNumber(pPedInfo->gate_type));
					}
					else {
						cJSON_AddItemToObject(idoff, "facility_type", cJSON_CreateNumber(9));
						cJSON_AddItemToObject(idoff, "gate_type", cJSON_CreateNumber(9));
					}
				}
				cJSON_AddItemToArray(links, idoff);
			} // for

			// add routes - link_info
			cJSON_AddItemToObject(route, "link_info", links);


			// - vertex_info
			cJSON* vertices = cJSON_CreateArray();
			int cntVertices = pResult->LinkVertex.size();
			for (int jj = 0; jj<cntVertices; jj++) {
				cJSON* lnglat = cJSON_CreateObject();
				cJSON_AddItemToObject(lnglat, "x", cJSON_CreateNumber(pResult->LinkVertex[jj].x));
				cJSON_AddItemToObject(lnglat, "y", cJSON_CreateNumber(pResult->LinkVertex[jj].y));
				cJSON_AddItemToArray(vertices, lnglat);

				// Local<Array> lnglat = Array::New(isolate);
				// lnglat->Set(context, 0, Number::New(isolate, pResult->LinkVertex[ii].x));
				// lnglat->Set(context, 1, Number::New(isolate, pResult->LinkVertex[ii].y));
				// coords->Set(context, ii, lnglat);
			}
			// add routes - vertex_info
			cJSON_AddItemToObject(route, "vertex_info", vertices);

			if (isJunction == true) {
				// junction_info
				cJSON* junctions = cJSON_CreateArray();
				
#if defined(TARGET_FOR_KAKAO_VX)
				vector<RouteProbablePath*> vtRpp;

				int cntRpp = GetRouteProbablePath(setVisited, pResult->LinkInfo, vtRpp);
				for (auto jj = 0; jj<cntRpp; jj++) {
					cJSON* link_info = cJSON_CreateObject();
					cJSON* links = cJSON_CreateArray();
					RouteProbablePath* pRpp = vtRpp[jj];
					// for (auto kk = 0; kk<pRpp->JctLinks.size(); kk++) {
					for (auto const& jct : pRpp->JctLinks) {
						cJSON* jct_info = cJSON_CreateObject();
						cJSON* link_vtx = cJSON_CreateArray();
						stLinkInfo* pLink = jct.second;
						for (auto ll = 0; ll<pLink->getVertexCount(); ll++) {
							cJSON* lnglat = cJSON_CreateObject();
							cJSON_AddItemToObject(lnglat, "x", cJSON_CreateNumber(pLink->getVertexX(ll)));
							cJSON_AddItemToObject(lnglat, "y", cJSON_CreateNumber(pLink->getVertexY(ll)));
							cJSON_AddItemToArray(link_vtx, lnglat);
						} // for vtx
						cJSON_AddItemToObject(jct_info, "id", cJSON_CreateNumber(pLink->link_id.nid));
						cJSON_AddItemToObject(jct_info, "vertices", link_vtx);
						cJSON_AddItemToArray(links, jct_info);
					} // for links
					cJSON_AddItemToObject(link_info, "id", cJSON_CreateNumber(pRpp->LinkId.nid));
					cJSON_AddItemToObject(link_info, "node_id", cJSON_CreateNumber(pRpp->NodeId.nid));
					cJSON_AddItemToObject(link_info, "junction", links);
					cJSON_AddItemToArray(junctions, link_info);

					//release
					SAFE_DELETE(pRpp);
				} // for junctions
#else
				vector<RouteProbablePath*> vtRpp;
				int cntRpp = GetRouteProbablePathEx(&setVisited, 0, 0, 100, vtRpp);
				stLinkInfo* pLink = nullptr;

				for (const auto& link : vtRpp) {
					//KeyID key = { key.llid = linkId };
					//pLink = m_pDataMgr->GetLinkDataById(key, TYPE_LINK_DATA_NONE);

					// ex에서는 1개만 들어있을 거임.
					pLink = link->JctLinks.begin()->second;

					if (pLink == nullptr) {
						continue;
					}

					// 링크 부가 정보 
					stLinkBaseInfo* pBaseInfo = nullptr;
					if (pLink->sub_info > 0) {
						pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&pLink->sub_info);
					}

					cJSON* link_info = cJSON_CreateObject();
					if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
						// link_id
						cJSON_AddItemToObject(link_info, "link_id", cJSON_CreateNumber(pLink->link_id.nid));
						// s_node_id
						cJSON_AddItemToObject(link_info, "snode_id", cJSON_CreateNumber(pLink->snode_id.nid));
						// e_node_id
						cJSON_AddItemToObject(link_info, "enode_id", cJSON_CreateNumber(pLink->enode_id.nid));
					} else {
						// link_id
						cJSON_AddItemToObject(link_info, "link_id", cJSON_CreateNumber(pLink->link_id.llid));
						// s_node_id
						cJSON_AddItemToObject(link_info, "snode_id", cJSON_CreateNumber(pLink->snode_id.llid));
						// e_node_id
						cJSON_AddItemToObject(link_info, "enode_id", cJSON_CreateNumber(pLink->enode_id.llid));
					}

					// vertex
					cJSON* vertices = cJSON_CreateArray();
					for (auto ll = 0; ll<pLink->getVertexCount(); ll++) {
						cJSON* lnglat = cJSON_CreateObject();
						cJSON_AddItemToObject(lnglat, "x", cJSON_CreateNumber(pLink->getVertexX(ll)));
						cJSON_AddItemToObject(lnglat, "y", cJSON_CreateNumber(pLink->getVertexY(ll)));
						cJSON_AddItemToArray(vertices, lnglat);
					} // for vtx
					cJSON_AddItemToObject(link_info, "vertices", vertices);

					cJSON_AddItemToArray(junctions, link_info);
				} // for jct
#endif

				// add routes - junction_info
				cJSON_AddItemToObject(route, "junction_info", junctions);
			} // if (isJunction == true)
#endif

			// increse route
			cJSON_AddItemToArray(routes, route);
		} // for

		// add routes
		cJSON_AddItemToObject(root, "routes", routes);
	}

	char* pJson = cJSON_Print(root);
	strJson.append(pJson);
	cJSON_free(pJson);

	cJSON_Delete(root);

#else //#if defined(USE_CJSON)
Isolate* isolate = args.GetIsolate();
   Local<Context> context = isolate->GetCurrentContext();

   Local<Object> mainobj = Object::New(isolate);
   int cnt = 0;
   const RouteResultInfo* pResult = m_pRouteMgr.GetRouteResult();

   if (pResult == nullptr) {
      LOG_TRACE(LOG_ERROR, "Error, route result pointer null");
      mainobj->Set(context, String::NewFromUtf8(isolate, "msg").ToLocalChecked(), String::NewFromUtf8(isolate, "Error, route result pointer null").ToLocalChecked());
   }
   else {
      // info
      Local<Object> routes = Object::New(isolate);
      mainobj->Set(context, String::NewFromUtf8(isolate, "user_id").ToLocalChecked(), Number::New(isolate, pResult->RequestId));
      mainobj->Set(context, String::NewFromUtf8(isolate, "result_code").ToLocalChecked(), Integer::New(isolate, pResult->ResultCode));

      // result
      Local<Object> start_coord = Object::New(isolate);
      start_coord->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->StartResultLink.Coord.x));
      start_coord->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->StartResultLink.Coord.y));
      routes->Set(context, String::NewFromUtf8(isolate, "start").ToLocalChecked(), start_coord);
      Local<Object> end_coord = Object::New(isolate);
      end_coord->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->EndResultLink.Coord.x));
      end_coord->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->EndResultLink.Coord.y));
      routes->Set(context, String::NewFromUtf8(isolate, "end").ToLocalChecked(), end_coord);
      routes->Set(context, String::NewFromUtf8(isolate, "option").ToLocalChecked(), Integer::New(isolate, pResult->RouteOption));
      //dist
      routes->Set(context, String::NewFromUtf8(isolate, "distance").ToLocalChecked(), Integer::New(isolate, pResult->TotalLinkDist));
      //time
      routes->Set(context, String::NewFromUtf8(isolate, "time").ToLocalChecked(), Integer::New(isolate, pResult->TotalLinkTime));
      time_t timer = time(NULL);
      struct tm* tmNow = localtime(&timer);
      string strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
      routes->Set(context, String::NewFromUtf8(isolate, "now").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
      timer += pResult->TotalLinkTime;
      tmNow = localtime(&timer);
      strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
      routes->Set(context, String::NewFromUtf8(isolate, "eta").ToLocalChecked(), String::NewFromUtf8(isolate, strVal.c_str()).ToLocalChecked());
      // add
      mainobj->Set(context, String::NewFromUtf8(isolate, "summary").ToLocalChecked(), routes);


      // link
      Local<Array> link = Array::New(isolate);
      Local<Object> link_list = Object::New(isolate);
      cnt = pResult->LinkInfo.size();
      for(int ii=0; ii<cnt; ii++) {
         Local<Object> idoff = Object::New(isolate);
         idoff->Set(context, String::NewFromUtf8(isolate, "linkid").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].link_id.nid));
         idoff->Set(context, String::NewFromUtf8(isolate, "length").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].length));
         idoff->Set(context, String::NewFromUtf8(isolate, "time").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].time));
         idoff->Set(context, String::NewFromUtf8(isolate, "angle").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].angle));
         idoff->Set(context, String::NewFromUtf8(isolate, "vertex_offset").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].vtx_off));
         idoff->Set(context, String::NewFromUtf8(isolate, "vertex_count").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].vtx_cnt));
         idoff->Set(context, String::NewFromUtf8(isolate, "remain_distance").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].rlength));
         idoff->Set(context, String::NewFromUtf8(isolate, "remain_time").ToLocalChecked(), Number::New(isolate, pResult->LinkInfo[ii].rtime));

         // 링크 부가 정보 
         uint64_t sub_info = pResult->LinkInfo[ii].link_info;
         if (sub_info > 0) {
            stLinkBaseInfo* pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&sub_info);
            
            // 보행자 데이터
            if (pBaseInfo && pBaseInfo->link_type == TYPE_DATA_PEDESTRIAN) {
               stLinkPedestrianInfo* pPedInfo = reinterpret_cast<stLinkPedestrianInfo*>(&sub_info);
               idoff->Set(context, String::NewFromUtf8(isolate, "facility_type").ToLocalChecked(), Number::New(isolate, pPedInfo->facility_type));
               idoff->Set(context, String::NewFromUtf8(isolate, "gate_type").ToLocalChecked(), Number::New(isolate, pPedInfo->gate_type));
            }
            else {
               idoff->Set(context, String::NewFromUtf8(isolate, "facility_type").ToLocalChecked(), Number::New(isolate, 9));
               idoff->Set(context, String::NewFromUtf8(isolate, "gate_type").ToLocalChecked(), Number::New(isolate, 9));
            }
         }
         link->Set(context, ii, idoff);
      }
      link_list->Set(context, String::NewFromUtf8(isolate, "count").ToLocalChecked(), Integer::New(isolate, cnt));
      link_list->Set(context, String::NewFromUtf8(isolate, "links").ToLocalChecked(), link);

      // add
      mainobj->Set(context, String::NewFromUtf8(isolate, "guide").ToLocalChecked(), link_list);


      // vertex
      Local<Array> coords = Array::New(isolate);
      Local<Object> vertex_list = Object::New(isolate);
      cnt = pResult->LinkVertex.size();
      for(auto ii=0; ii<cnt; ii++) {
         Local<Object> lnglat = Object::New(isolate);
         lnglat->Set(context, String::NewFromUtf8(isolate, "x").ToLocalChecked(), Number::New(isolate, pResult->LinkVertex[ii].x));
         lnglat->Set(context, String::NewFromUtf8(isolate, "y").ToLocalChecked(), Number::New(isolate, pResult->LinkVertex[ii].y));
         coords->Set(context, ii, lnglat);

         // Local<Array> lnglat = Array::New(isolate);
         // lnglat->Set(context, 0, Number::New(isolate, pResult->LinkVertex[ii].x));
         // lnglat->Set(context, 1, Number::New(isolate, pResult->LinkVertex[ii].y));
         // coords->Set(context, ii, lnglat);
      }
      // vertex_list->Set(context, String::NewFromUtf8(isolate, "type").ToLocalChecked(), String::NewFromUtf8(isolate, "LineString").ToLocalChecked());
      vertex_list->Set(context, String::NewFromUtf8(isolate, "count").ToLocalChecked(), Integer::New(isolate, cnt));
      vertex_list->Set(context, String::NewFromUtf8(isolate, "coords").ToLocalChecked(), coords);

      // add
      mainobj->Set(context, String::NewFromUtf8(isolate, "routes").ToLocalChecked(), vertex_list);
   }

   args.GetReturnValue().Set(mainobj);
#endif // #if defined(USE_CJSON)
}


void CRoutePackage::GetMultiRouteResult(IN const vector<RouteResultInfo>& vtRouteResults, IN const bool isJunction, OUT string& strJson)
{
#if defined(USE_CJSON)
	cJSON* root = cJSON_CreateObject();

	int cntRoutes = vtRouteResults.size();

	int request_id = 0;
	bool isSuccess = false;
	bool isFailed = false;
	vector<int> vtResultCodes;

	// header
	for (const auto& result : vtRouteResults) {
		request_id = result.RequestId;
		(result.ResultCode != ROUTE_RESULT_SUCCESS) ? isFailed = true : isSuccess = true;
		vtResultCodes.emplace_back(result.ResultCode);
	}

	cJSON_AddItemToObject(root, "request_id", cJSON_CreateNumber(request_id));
	if (isFailed) {
		if (vtResultCodes.empty()) {
			cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(ROUTE_RESULT_FAILED_MULTI_POS_ROUTE_ALL));
		} else {
			cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(vtResultCodes[0]));
		}
	} else {
		cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(ROUTE_RESULT_SUCCESS));
	}

	// results
	if (cntRoutes > 0) {
		time_t timer = time(NULL);
		//struct tm* tmNow;
		string strEta;

		// routes
		cJSON* routes = cJSON_CreateArray();

		for (int ii = 0; ii < cntRoutes; ii++) {
			// route 
			cJSON* route = cJSON_CreateObject();

			RouteResultInfo* pResult = const_cast<RouteResultInfo*>(&vtRouteResults[ii]);
#if 1
			GetRouteResultJson(pResult, timer, isJunction, route);
#else
			request_id = pResult->RequestId;
			result_code = pResult->ResultCode;

			// - summary
			cJSON* summary = cJSON_CreateObject();
			// - type
			cJSON_AddItemToObject(summary, "option", cJSON_CreateNumber(pResult->RouteOption));
			// - distance
			cJSON_AddItemToObject(summary, "distance", cJSON_CreateNumber(pResult->TotalLinkDist));
			// - time
			cJSON_AddItemToObject(summary, "time", cJSON_CreateNumber(pResult->TotalLinkTime));
			// - now
			timer = time(NULL);
			tmNow = localtime(&timer);
			strEta = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
			cJSON_AddItemToObject(summary, "now", cJSON_CreateString(strEta.c_str()));

			// eta
			timer += pResult->TotalLinkTime;
			tmNow = localtime(&timer);
			strEta = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
			cJSON_AddItemToObject(summary, "eta", cJSON_CreateString(strEta.c_str()));

			// add routes - summary
			cJSON_AddItemToObject(route, "summary", summary);

			unordered_set<uint64_t> setVisited;

			// - link_info
			cJSON* links = cJSON_CreateArray();
			int cntLinks = pResult->LinkInfo.size();
			int vertex_offset = 0;
			for (const auto& link : pResult->LinkInfo) {
				cJSON* idoff = cJSON_CreateObject();

				setVisited.emplace(link.link_id.llid);

				// 링크 부가 정보 
				uint64_t sub_info = link.link_info;
				stLinkBaseInfo* pBaseInfo = nullptr;
				if (sub_info > 0) {
					pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&sub_info);
				}				
				if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
					cJSON_AddItemToObject(idoff, "link_id", cJSON_CreateNumber(link.link_id.nid));
				} else {
					cJSON_AddItemToObject(idoff, "link_id", cJSON_CreateNumber(link.link_id.llid));			
				}
				cJSON_AddItemToObject(idoff, "length", cJSON_CreateNumber(link.length));
				cJSON_AddItemToObject(idoff, "time", cJSON_CreateNumber(link.time));
				cJSON_AddItemToObject(idoff, "angle", cJSON_CreateNumber(link.angle));
				cJSON_AddItemToObject(idoff, "vertex_offset", cJSON_CreateNumber(vertex_offset));
				cJSON_AddItemToObject(idoff, "vertex_count", cJSON_CreateNumber(link.vtx_cnt));
				cJSON_AddItemToObject(idoff, "remain_distance", cJSON_CreateNumber(link.rlength));
				cJSON_AddItemToObject(idoff, "remain_time", cJSON_CreateNumber(link.rtime));
				cJSON_AddItemToObject(idoff, "guide_type", cJSON_CreateNumber(link.guide_type));

				if (link.vtx_cnt > 0) {
					vertex_offset += link.vtx_cnt; // 경유지에서 offset 초기화 되지 않도록
				}

				if (pBaseInfo != nullptr) {
					// 종단 노드 정보
					stLinkInfo* pLink = m_pDataMgr->GetLinkDataById(link.link_id, pBaseInfo->link_type);
					if (pLink) {
						KeyID last_node_key;
						uint64_t last_node_id = 0;
						if (link.dir == 0) { // 정방향 enode가 종단 노드
							last_node_key = pLink->enode_id;
							if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
								last_node_id = pLink->enode_id.nid;
							} else {
								last_node_id = pLink->enode_id.llid;
							}
						}
						else { // 역방향 snode가 종단 노드
							last_node_key = pLink->snode_id;
							if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
								last_node_id = pLink->snode_id.nid;
							} else {
								last_node_id = pLink->snode_id.llid;
							}
						}
						cJSON_AddItemToObject(idoff, "node_id", cJSON_CreateNumber(last_node_id));

						stNodeInfo* pNode = m_pDataMgr->GetNodeDataById(last_node_key, pBaseInfo->link_type);
						if (pNode) {
							cJSON_AddItemToObject(idoff, "conn_count", cJSON_CreateNumber(pNode->base.connnode_count));
						}
						//if (link.type == LINK_GUIDE_TYPE_DEPARTURE) { // 시작점은 항상 노드 연결이 2개다.
						//	cJSON_AddItemToObject(idoff, "conn_count", cJSON_CreateNumber(2));
						//}
						//else {
						//	stNodeInfo* pNode = m_pDataMgr->GetNodeDataById(link.node_id, pBaseInfo->link_type);
						//	cJSON_AddItemToObject(idoff, "conn_count", cJSON_CreateNumber(pNode->base.connnode_count));
						//}
					}
#if defined(USE_FOREST_DATA)
					cJSON_AddItemToObject(idoff, "link_type", cJSON_CreateNumber(pBaseInfo->link_type));
#endif
					// 숲길 데이터
					if (pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
						stLinkTrekkingInfo* pForestInfo = reinterpret_cast<stLinkTrekkingInfo*>(&sub_info);
						cJSON_AddItemToObject(idoff, "course_type", cJSON_CreateNumber(pForestInfo->course_type));
						cJSON_AddItemToObject(idoff, "road_type", cJSON_CreateNumber(pForestInfo->road_info));
						cJSON_AddItemToObject(idoff, "difficult", cJSON_CreateNumber(pForestInfo->diff));
						cJSON_AddItemToObject(idoff, "slop", cJSON_CreateNumber(static_cast<int8_t>(pForestInfo->slop)));
						cJSON_AddItemToObject(idoff, "popular", cJSON_CreateNumber(pForestInfo->popular));
						cJSON_AddItemToObject(idoff, "legal", cJSON_CreateNumber(pForestInfo->legal));
						// 고도(altitude)
						if ((link.link_id.llid != NOT_USE) && (pLink != nullptr)) {
							stNodeInfo* pSNode = m_pDataMgr->GetFNodeDataById(pLink->snode_id);
							stNodeInfo* pENode = m_pDataMgr->GetFNodeDataById(pLink->enode_id);

							if (link.dir == 0) { // 정
								cJSON_AddItemToObject(idoff, "alt_s", cJSON_CreateNumber(static_cast<int16_t>(pSNode->trk.z_value)));
								cJSON_AddItemToObject(idoff, "alt_e", cJSON_CreateNumber(static_cast<int16_t>(pENode->trk.z_value)));
							}
							else {
								cJSON_AddItemToObject(idoff, "alt_s", cJSON_CreateNumber(static_cast<int16_t>(pENode->trk.z_value)));
								cJSON_AddItemToObject(idoff, "alt_e", cJSON_CreateNumber(static_cast<int16_t>(pSNode->trk.z_value)));
							}
						}
						// 서브코스 정보
#if defined(USE_MOUNTAIN_DATA)
						if ((pLink != nullptr) && pLink->trk_ext.course_cnt > 0) {
							set<uint32_t>* pCourse = m_pDataMgr->GetCourseByLink(pLink->link_id.llid);
							if (pCourse != nullptr) {
								cJSON* courses = cJSON_CreateArray();
								stCourseInfo courseInfo;
								for (const auto& course_id : *pCourse) {
									cJSON* course = cJSON_CreateObject();
									courseInfo.course_id = course_id;
									cJSON_AddItemToObject(course, "type", cJSON_CreateNumber(courseInfo.course_type));
									cJSON_AddItemToObject(course, "code", cJSON_CreateNumber(courseInfo.course_cd));
									cJSON_AddItemToArray(courses, course);
								}
								cJSON_AddItemToObject(idoff, "sub_course", courses);
							}
						}
#endif // #if defined(USE_MOUNTAIN_DATA)
					}
					// 보행자 데이터
					else if (pBaseInfo->link_type == TYPE_LINK_DATA_PEDESTRIAN) {
						stLinkPedestrianInfo* pPedInfo = reinterpret_cast<stLinkPedestrianInfo*>(&sub_info);
						cJSON_AddItemToObject(idoff, "walk_type", cJSON_CreateNumber(pPedInfo->walk_type));
						cJSON_AddItemToObject(idoff, "bicycle_type", cJSON_CreateNumber(pPedInfo->bicycle_type));
						cJSON_AddItemToObject(idoff, "facility_type", cJSON_CreateNumber(pPedInfo->facility_type));
						cJSON_AddItemToObject(idoff, "gate_type", cJSON_CreateNumber(pPedInfo->gate_type));
					}
					else {
						cJSON_AddItemToObject(idoff, "facility_type", cJSON_CreateNumber(9));
						cJSON_AddItemToObject(idoff, "gate_type", cJSON_CreateNumber(9));
					}
				}
				cJSON_AddItemToArray(links, idoff);
			} // for

			  // add routes - link_info
			cJSON_AddItemToObject(route, "link_info", links);


			// - vertex_info
			cJSON* vertices = cJSON_CreateArray();
			int cntVertices = pResult->LinkVertex.size();
			for (auto jj = 0; jj < cntVertices; jj++) {
				cJSON* lnglat = cJSON_CreateObject();
				cJSON_AddItemToObject(lnglat, "x", cJSON_CreateNumber(pResult->LinkVertex[jj].x));
				cJSON_AddItemToObject(lnglat, "y", cJSON_CreateNumber(pResult->LinkVertex[jj].y));
				cJSON_AddItemToArray(vertices, lnglat);

				// Local<Array> lnglat = Array::New(isolate);
				// lnglat->Set(context, 0, Number::New(isolate, pResult->LinkVertex[ii].x));
				// lnglat->Set(context, 1, Number::New(isolate, pResult->LinkVertex[ii].y));
				// coords->Set(context, ii, lnglat);
			}
			// add routes - vertex_info
			cJSON_AddItemToObject(route, "vertex_info", vertices);

			if (isJunction == true) {
				// junction_info
				cJSON* junctions = cJSON_CreateArray();

#if defined(TARGET_FOR_KAKAO_VX)
				vector<RouteProbablePath*> vtRpp;

				int cntRpp = GetRouteProbablePath(setVisited, pResult->LinkInfo, vtRpp);
				for (auto jj = 0; jj < cntRpp; jj++) {
					cJSON* link_info = cJSON_CreateObject();
					cJSON* links = cJSON_CreateArray();
					RouteProbablePath* pRpp = vtRpp[jj];
					// for (auto kk = 0; kk<pRpp->JctLinks.size(); kk++) {
					for (auto const& jct : pRpp->JctLinks) {
						cJSON* jct_info = cJSON_CreateObject();
						cJSON* link_vtx = cJSON_CreateArray();
						stLinkInfo* pLink = jct.second;
						for (auto ll = 0; ll < pLink->getVertexCount(); ll++) {
							cJSON* lnglat = cJSON_CreateObject();
							cJSON_AddItemToObject(lnglat, "x", cJSON_CreateNumber(pLink->getVertexX(ll)));
							cJSON_AddItemToObject(lnglat, "y", cJSON_CreateNumber(pLink->getVertexY(ll)));
							cJSON_AddItemToArray(link_vtx, lnglat);
						} // for vtx
						cJSON_AddItemToObject(jct_info, "id", cJSON_CreateNumber(pLink->link_id.nid));
						cJSON_AddItemToObject(jct_info, "vertices", link_vtx);
						cJSON_AddItemToArray(links, jct_info);
					} // for links
					cJSON_AddItemToObject(link_info, "id", cJSON_CreateNumber(pRpp->LinkId.nid));
					cJSON_AddItemToObject(link_info, "node_id", cJSON_CreateNumber(pRpp->NodeId.nid));
					cJSON_AddItemToObject(link_info, "junction", links);
					cJSON_AddItemToArray(junctions, link_info);

					//release
					SAFE_DELETE(pRpp);
				} // for junctions

#else
				vector<RouteProbablePath*> vtRpp;
				int cntRpp = GetRouteProbablePathEx(&setVisited, 0, 0, 100, vtRpp);
				stLinkInfo* pLink = nullptr;

				for (const auto& link : vtRpp) {
					//KeyID key = { key.llid = linkId };
					//pLink = m_pDataMgr->GetLinkDataById(key, TYPE_LINK_DATA_NONE);

					// ex에서는 1개만 들어있을 거임.
					pLink = link->JctLinks.begin()->second;

					if (pLink == nullptr) {
						continue;
					}

					// 링크 부가 정보 
					stLinkBaseInfo* pBaseInfo = nullptr;
					if (pLink->sub_info > 0) {
						pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&pLink->sub_info);
					}

					cJSON* link_info = cJSON_CreateObject();
					if (pBaseInfo && pBaseInfo->link_type == TYPE_LINK_DATA_TREKKING) {
						// link_id
						cJSON_AddItemToObject(link_info, "link_id", cJSON_CreateNumber(pLink->link_id.nid));
						// s_node_id
						cJSON_AddItemToObject(link_info, "snode_id", cJSON_CreateNumber(pLink->snode_id.nid));
						// e_node_id
						cJSON_AddItemToObject(link_info, "enode_id", cJSON_CreateNumber(pLink->enode_id.nid));
					} else {
						// link_id
						cJSON_AddItemToObject(link_info, "link_id", cJSON_CreateNumber(pLink->link_id.llid));
						// s_node_id
						cJSON_AddItemToObject(link_info, "snode_id", cJSON_CreateNumber(pLink->snode_id.llid));
						// e_node_id
						cJSON_AddItemToObject(link_info, "enode_id", cJSON_CreateNumber(pLink->enode_id.llid));
					}

					// vertex
					cJSON* vertices = cJSON_CreateArray();
					for (auto ll = 0; ll<pLink->getVertexCount(); ll++) {
						cJSON* lnglat = cJSON_CreateObject();
						cJSON_AddItemToObject(lnglat, "x", cJSON_CreateNumber(pLink->getVertexX(ll)));
						cJSON_AddItemToObject(lnglat, "y", cJSON_CreateNumber(pLink->getVertexY(ll)));
						cJSON_AddItemToArray(vertices, lnglat);
					} // for vtx
					cJSON_AddItemToObject(link_info, "vertices", vertices);

					cJSON_AddItemToArray(junctions, link_info);
				} // for jct
#endif

				  // add routes - junction_info
				cJSON_AddItemToObject(route, "junction_info", junctions);
			} // if (isJunction == true)
#endif 

			// increse route
			cJSON_AddItemToArray(routes, route);
		} // for

		// add routes
		cJSON_AddItemToObject(root, "routes", routes);
	}

	char* pJson = cJSON_Print(root);
	strJson.append(pJson);
	cJSON_free(pJson);

	cJSON_Delete(root);
#endif // #if defined(USE_CJSON)
}


int32_t CRoutePackage::GetMapsRouteResult(IN const RouteResultInfo* pResult, OUT string& strJson)
{
	int result_code = ROUTE_RESULT_FAILED;

#if defined(USE_CJSON)
	cJSON* root = cJSON_CreateObject();

	if (pResult == nullptr) {
		result_code = ROUTE_RESULT_FAILED;
	}
	else if (pResult->ResultCode != ROUTE_RESULT_SUCCESS) {
		result_code = pResult->ResultCode;
	}
	else {
		result_code = ROUTE_RESULT_SUCCESS;

		// - now
		time_t timer = time(NULL);

		cJSON* routes = cJSON_CreateArray();
		cJSON* route = cJSON_CreateObject();

#if 1
		GetMapsRouteResultJson(pResult, timer, route);
#else
		cJSON_AddItemToObject(data, "option", cJSON_CreateNumber(pResult->RouteOption));
		cJSON_AddItemToObject(data, "spend_time", cJSON_CreateNumber(pResult->TotalLinkTime));
		cJSON_AddItemToObject(data, "distance", cJSON_CreateNumber(pResult->TotalLinkDist));
		cJSON_AddItemToObject(data, "toll_fee", cJSON_CreateNumber(0));
		cJSON_AddItemToObject(data, "taxiFare", cJSON_CreateNumber(0));
		cJSON_AddItemToObject(data, "isHighWay", cJSON_CreateBool(false));

		stLinkInfo* pLink = nullptr;
		stLinkVehicleInfo vehInfo;
		char szBuff[MAX_PATH] = { 0, };

		int vertex_offset = 0;

		// 경로 정보 (Array)
		cJSON* paths = cJSON_CreateArray();

		for (const auto& link : pResult->LinkInfo) {
			// 경로 링크 정보
			cJSON* path = cJSON_CreateObject();

			// 경로선 (Array)
			cJSON* coords = cJSON_CreateArray();
			int vtxCount = link.vtx_cnt;

			memcpy(&vehInfo, &link.link_info, sizeof(vehInfo));

			// 경로선 확장
			for (int ii = 0; ii < vtxCount; ii++) {
				// 남은 거리로 vertex 확인

				cJSON* coord = cJSON_CreateObject();
				cJSON_AddItemToObject(coord, "x", cJSON_CreateNumber(pResult->LinkVertex[vertex_offset].x));
				cJSON_AddItemToObject(coord, "y", cJSON_CreateNumber(pResult->LinkVertex[vertex_offset].y));

				vertex_offset++;

				// add coord to coords
				cJSON_AddItemToArray(coords, coord);
			} // for

			  // add coords to path
			cJSON_AddItemToObject(path, "coords", coords);

			// speed
			cJSON_AddNumberToObject(path, "speed", 0);

			// time
			cJSON_AddNumberToObject(path, "spend_time", link.time);

			// distance
			cJSON_AddNumberToObject(path, "distance", link.length);

			// road_code
			cJSON_AddNumberToObject(path, "road_code", vehInfo.road_type);

			// traffic_color
			cJSON_AddStringToObject(path, "traffic_color", "green");

#if defined(USE_P2P_DATA) // P2P HD 매칭을 위한 SD 링크 ID 정보
			pLink = m_pDataMgr->GetVLinkDataById(link.link_id);
			if (pLink != nullptr && link.link_id.llid != NULL_VALUE) {
				// road_name
				if (pLink->name_idx > 0) {
#if defined(_WIN32)
					char szUTF8[MAX_PATH] = { 0, };
					MultiByteToUTF8(m_pDataMgr->GetNameDataByIdx(pLink->name_idx), szUTF8);
					cJSON_AddStringToObject(path, "road_name", szUTF8);
#else
					cJSON_AddStringToObject(path, "road_name", encoding(m_pDataMgr->GetNameDataByIdx(pLink->name_idx), "euc-kr", "utf-8"));
#endif // #if defined(_WIN32)
				}

				// p2p 추가정보
				cJSON* p2p = cJSON_CreateObject();

				// speed 재설정
				cJSON_SetNumberHelper(cJSON_GetObjectItem(path, "speed"), pLink->veh.speed_f);

				// hd matching link id
				// LOG_TRACE(LOG_DEBUG, "tile:%d, id:%d, snode:%d, enode:%d",pLink->link_id.tile_id, pLink->link_id.nid, pLink->snode_id.nid, pLink->enode_id.nid);
				sprintf(szBuff, "%d%06d%06d", pLink->link_id.tile_id, pLink->snode_id.nid, pLink->enode_id.nid);
				// cJSON_AddNumberToObject(p2p, "link_id", (pLink->link_id.tile_id * 1000000000000) + (pLink->snode_id.nid * 1000000) + pLink->enode_id.nid); // 원본 ID 사용, (snode 6자리 + enode 6자리)
				cJSON_AddStringToObject(p2p, "link_id", szBuff);

				// dir, 0:정방향, 1:역방향
				cJSON_AddNumberToObject(p2p, "dir", link.dir);

				// type 링크 타입, 0:일반, 1:출발지링크, 2:경유지링크, 3:도착지링크
				cJSON_AddNumberToObject(p2p, "guide_type", link.guide_type);

				// ang, 진행각
				cJSON_AddNumberToObject(p2p, "angle", link.angle);

				// add p2p to path
				cJSON_AddItemToObject(path, "p2p_extend", p2p);
			}
#endif // #if 1 defined(USE_P2P_DATA)

			// add path to paths
			cJSON_AddItemToArray(paths, path);

			// add paths to route
			cJSON_AddItemToObject(route, "paths", paths);

		} // for paths
#endif

		cJSON_AddItemToArray(routes, route);

		// add data to root
		cJSON_AddItemToObject(root, "routes", routes);
	}

	// add header to root
	cJSON_AddNumberToObject(root, "user_id", pResult->RequestId);
	cJSON_AddNumberToObject(root, "result_code", result_code);
	if (result_code == ROUTE_RESULT_SUCCESS) {
		cJSON_AddStringToObject(root, "error_msg", "success");
	} else {
		cJSON_AddStringToObject(root, "error_msg", "failed");
	}

	char* pJson = cJSON_Print(root);
	strJson.append(pJson);
	cJSON_free(pJson);

	cJSON_Delete(root);

#elif defined(USE_TAOJSON)
	tao::json::value root = json::empty_object;

	tao::json::value header = {
		{ "isSuccessful", false },
		{ "resultCode", ROUTE_RESULT_FAILED },
		{ "resultMessage", "Error, route result pointer null" }
	};

	root.emplace("header", header);

	strJson = tao::json::to_string(root);

#elif defined(USE_RAPIDJSON)
	Document doc(kObjectType);
	Document::AllocatorType& allocator = doc.GetAllocator();

	// Value header("header", allocator);
	// Value val(72);
	// doc.AddMember(hader, val, allocator);

	doc.AddMember("isSuccessful", false, allocator);
	doc.AddMember("resultCode", ROUTE_RESULT_FAILED, allocator);
	doc.AddMember("resultMessage", 0, allocator);

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	doc.Accept(writer);
	strJson = buffer.GetString();
#endif // #if defined(USE_CJSON)

	// add route to root
	if (!strJson.empty()) {
		// mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
	}

	return result_code;
}


int32_t CRoutePackage::GetMapsMultiRouteResult(IN const vector<RouteResultInfo>& vtRouteResults, OUT string& strJson)
{
	int result_code = ROUTE_RESULT_FAILED;

#if defined(USE_CJSON)
	cJSON* root = cJSON_CreateObject();
	
	int cntRoutes = vtRouteResults.size();

	int request_id = 0;
	bool isSuccess = false;
	bool isFailed = false;
	vector<int> vtResultCodes;

	// header
	for (const auto& result : vtRouteResults) {
		request_id = result.RequestId;
		(result.ResultCode != ROUTE_RESULT_SUCCESS) ? isFailed = true : isSuccess = true;
		vtResultCodes.emplace_back(result.ResultCode);
	}

	// add header to root
	cJSON_AddItemToObject(root, "request_id", cJSON_CreateNumber(request_id));
	if (isFailed) {
		if (vtResultCodes.empty()) {
			cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(ROUTE_RESULT_FAILED_MULTI_POS_ROUTE_ALL));
		} else {
			cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(vtResultCodes[0]));
		}
	} else {
		cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(ROUTE_RESULT_SUCCESS));
	}


	// - now
	time_t timer = time(NULL);

	cJSON* routes = cJSON_CreateArray();

	for (const auto& result : vtRouteResults) {
		const RouteResultInfo* pResult = &result;

		if (pResult == nullptr) {
			result_code = ROUTE_RESULT_FAILED;
		} else if (pResult->ResultCode != ROUTE_RESULT_SUCCESS) {
			result_code = pResult->ResultCode;
		} else {
			cJSON* route = cJSON_CreateObject();

			GetMapsRouteResultJson(pResult, timer, route);

			// add paths to routes
			cJSON_AddItemToArray(routes, route);
		}
	} // for routes

	// add routes to root
	cJSON_AddItemToObject(root, "routes", routes);

	char* pJson = cJSON_Print(root);
	strJson.append(pJson);
	cJSON_free(pJson);

	cJSON_Delete(root);

#endif // #if defined(USE_CJSON)

	return result_code;
}


void CRoutePackage::GetClusteringResult(IN const vector<stDistrict>& vtClusters, IN const vector<SPoint>& vtPositionLock, OUT string& strJson)
{
	//string strJson;
	int cntClusters = vtClusters.size();

	int result_code = ROUTE_RESULT_FAILED;

	if (cntClusters <= 0) {
		result_code = ROUTE_RESULT_FAILED;
	}
	else {
		result_code = ROUTE_RESULT_SUCCESS;

		const int cntCluster = vtClusters.size();

#if defined(USE_CJSON)
		cJSON* root = cJSON_CreateObject();
		cJSON* summary = cJSON_CreateObject();
		cJSON* clusters = cJSON_CreateArray();

		int totTime = 0;
		int totDist = 0;
		int totCount = 0;

		for (const auto& cluster : vtClusters) {
			cJSON* group = cJSON_CreateObject();

			// id
			cJSON_AddNumberToObject(group, "group", cluster.id);

			// time
			cJSON_AddNumberToObject(group, "time", static_cast<int>(cluster.time));

			// dist
			cJSON_AddNumberToObject(group, "distance", static_cast<int>(cluster.dist));

			// etd
			if (cluster.etd > 0) {
				cJSON_AddNumberToObject(group, "etd", static_cast<int>(cluster.etd));
			}

			// eta
			if (cluster.eta > 0) {
				cJSON_AddNumberToObject(group, "eta", static_cast<int>(cluster.eta));
			}

			// pois
			cJSON* pPois = cJSON_CreateArray();
			for (int ii = 0; ii < cluster.vtPois.size(); ii++) {
				cJSON* poi = cJSON_CreateObject();
				cJSON_AddNumberToObject(poi, "idx", cluster.vtPois[ii]);
				double arrLoc[2] = { cluster.vtCoord[ii].x, cluster.vtCoord[ii].y };
				cJSON* coord = cJSON_CreateDoubleArray(arrLoc, 2);
				cJSON_AddItemToObject(poi, "coord", coord);
				cJSON_AddItemToArray(pPois, poi);
			} // for
			cJSON_AddItemToObject(group, "pois", pPois);


			totTime += cluster.time;
			totDist += cluster.dist;
			totCount += cluster.vtPois.size();

			// center
			double dwCenter[2] = { cluster.center.x, cluster.center.y };
			cJSON* centerPos = cJSON_CreateDoubleArray(dwCenter, 2);
			cJSON_AddItemToObject(group, "center", centerPos);

			// boundary
			cJSON* pBoundary = cJSON_CreateArray();
			for (const auto& border : cluster.vtBorder) {
				double arrLoc[2] = { border.x, border.y };
				cJSON* coord = cJSON_CreateDoubleArray(arrLoc, 2);
				cJSON_AddItemToArray(pBoundary, coord);
			} // for
			cJSON_AddItemToObject(group, "boundary", pBoundary);

			// way time
			if (!cluster.vtTimes.empty()) {
				cJSON* pWayTimes = cJSON_CreateIntArray(&cluster.vtTimes[0], cluster.vtTimes.size());
				cJSON_AddItemToObject(group, "way_times", pWayTimes);
			}

			// way dist
			if (!cluster.vtDistances.empty()) {
				cJSON* pWayDistances = cJSON_CreateIntArray(&cluster.vtDistances[0], cluster.vtDistances.size());
				cJSON_AddItemToObject(group, "way_distances", pWayDistances);
			}

			cJSON_AddItemToArray(clusters, group);
		} // for

		cJSON_AddItemToObject(root, "clusters", clusters);


		// summary
		cJSON_AddNumberToObject(summary, "avg_count", totCount / cntCluster);
		cJSON_AddNumberToObject(summary, "avg_time", static_cast<int>(totTime) / cntCluster);
		cJSON_AddNumberToObject(summary, "avg_distance", static_cast<int>(totDist) / cntCluster);

		// position_lock
		if (!vtPositionLock.empty()) {
			if (vtPositionLock[0].x != 0 && vtPositionLock[0].y) {
				double arrLoc[2] = { vtPositionLock[0].x, vtPositionLock[0].y };
				cJSON* coord = cJSON_CreateDoubleArray(arrLoc, 2);
				cJSON_AddItemToObject(summary, "start_lock", coord);
			}

			if (vtPositionLock[1].x != 0 && vtPositionLock[1].y) {
				double arrLoc[2] = { vtPositionLock[1].x, vtPositionLock[1].y };
				cJSON* coord = cJSON_CreateDoubleArray(arrLoc, 2);
				cJSON_AddItemToObject(summary, "end_lock", coord);
			}
		}
		cJSON_AddItemToObject(root, "summary", summary);

		// result
		cJSON_AddStringToObject(root, "status", "OK");
		cJSON_AddNumberToObject(root, "result_code", result_code);
		cJSON_AddStringToObject(root, "msg", "success");

		char* pJson = cJSON_Print(root);
		strJson.append(pJson);
		cJSON_free(pJson);

		cJSON_Delete(root);
	}

	// add route to root
	if (!strJson.empty()) {
		// mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
	}
#endif // #if defined(USE_CJSON)
}


void CRoutePackage::GetBoundaryResult(IN const vector<SPoint>& vtBoundary, OUT string& strJson)
{
	//string strJson;
	int cntPolygon = vtBoundary.size();;

	int result_code = ROUTE_RESULT_FAILED;

	if (cntPolygon <= 0) {
		result_code = ROUTE_RESULT_FAILED;
	}
	else {
		result_code = ROUTE_RESULT_SUCCESS;

#if defined(USE_CJSON)
		cJSON* root = cJSON_CreateObject();
		cJSON* boundary = cJSON_CreateArray();

		for (const auto& border : vtBoundary) {
			double arrLoc[2] = { border.x, border.y };
			cJSON* coord = cJSON_CreateDoubleArray(arrLoc, 2);
			cJSON_AddItemToArray(boundary, coord);
		} // for
		cJSON_AddItemToObject(root, "boundary", boundary);

		cJSON_AddStringToObject(root, "status", "OK");
		cJSON_AddNumberToObject(root, "result_code", result_code);
		cJSON_AddStringToObject(root, "msg", "success");

		char* pJson = cJSON_Print(root);
		strJson.append(pJson);
		cJSON_free(pJson);

		cJSON_Delete(root);
	}

	// add route to root
	if (!strJson.empty()) {
		// mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
	}
#endif // #if defined(USE_CJSON)
}


void CRoutePackage::GetBestWaypointResult(IN const vector<stWaypoints>& vtWaypoints, IN const vector<uint32_t>& vtBestWaypoints, IN const double dist, IN const int32_t time, OUT string& strJson)
{
#if defined(USE_CJSON)
	cJSON* root = cJSON_CreateObject();

	int cntWaypoints = vtBestWaypoints.size();

	if (cntWaypoints > 0) {
		cJSON* summary = cJSON_CreateObject();
		cJSON* waypoints = cJSON_CreateArray();

		// best waypoints
		for(int ii=0; ii<cntWaypoints; ii++) {
			cJSON* info = cJSON_CreateObject();

			cJSON_AddNumberToObject(info, "index", vtBestWaypoints[ii]);
			double arrLoc[2] = { vtWaypoints[vtBestWaypoints[ii]].x, vtWaypoints[vtBestWaypoints[ii]].y };
			cJSON* coord = cJSON_CreateDoubleArray(arrLoc, 2);
			cJSON_AddItemToObject(info, "coord", coord);
			cJSON_AddItemToArray(waypoints, info);
		} // for
		cJSON_AddItemToObject(root, "waypoints", waypoints);

		// summary
		cJSON_AddNumberToObject(summary, "time", static_cast<int>(time));
		cJSON_AddNumberToObject(summary, "distance", static_cast<int>(dist));
		cJSON_AddItemToObject(root, "summary", summary);

		// result
		cJSON_AddStringToObject(root, "status", "OK");
		cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(ROUTE_RESULT_SUCCESS));
		cJSON_AddStringToObject(root, "msg", "success");
	} else {
		cJSON_AddStringToObject(root, "status", "UNKNOWN_ERROR ");
		cJSON_AddNumberToObject(root, "result_code", ROUTE_RESULT_FAILED);
		cJSON_AddStringToObject(root, "msg", "failed");
	}
	
	char* pJson = cJSON_Print(root);
	strJson.append(pJson);
	cJSON_free(pJson);

	cJSON_Delete(root);
#endif // #if defined(USE_CJSON)

	// add route to root
	if (!strJson.empty()) {
		// mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
	}
}

const char* getDistanceString(IN const int32_t dist)
{
   static char szBuff[128];
   if (dist <= 0) {
      return "0 Km";
   } else {
      sprintf(szBuff, "%.1f km", dist / 1000.f);
   }

   return szBuff;
}


const char* getDurationString(IN const int32_t time)
{
   static char szBuff[128];
   if (time <= 0) {
      return "0 mins";
   } else {
      if (time > 60 * 60) {
         sprintf(szBuff, "%d hours %d mins", time / 60 * 60, time / 60);
      } else if (time > 60) {
         sprintf(szBuff, "%d mins", time / 60);
      } else {
         sprintf(szBuff, "1 mins");
      }      
   }

   return szBuff;
}


void CRoutePackage::GetWeightMatrixResult(IN const vector<vector<stDistMatrix>>& vtWeightMatrix, OUT string& strJson)
{
#if defined(USE_CJSON)
   cJSON* root = cJSON_CreateObject();

	if (!vtWeightMatrix.empty()) {
		cJSON* rows = cJSON_CreateArray();

		int nCount = vtWeightMatrix.size();

		for(int ii=0; ii<nCount; ii++) {
			cJSON* elements = cJSON_CreateArray();
			cJSON* cols = cJSON_CreateObject();

			for(int jj=0; jj<nCount; jj++) {
				cJSON* element = cJSON_CreateObject();
				cJSON* distance = cJSON_CreateObject();
				cJSON* duration = cJSON_CreateObject();
				cJSON* status = cJSON_CreateObject();

				cJSON_AddNumberToObject(distance, "value", vtWeightMatrix[ii][jj].nTotalDist);
				cJSON_AddStringToObject(distance, "text", getDistanceString(vtWeightMatrix[ii][jj].nTotalDist));

				cJSON_AddNumberToObject(duration, "value", vtWeightMatrix[ii][jj].nTotalTime);
				cJSON_AddStringToObject(duration, "text", getDurationString(vtWeightMatrix[ii][jj].nTotalTime));

				cJSON_AddItemToObject(element, "distance", distance);
				cJSON_AddItemToObject(element, "duration", duration);
				cJSON_AddStringToObject(element, "status", "OK");

				cJSON_AddItemToArray(elements, element);
			} // for

			cJSON_AddItemToObject(cols, "elements", elements);
			cJSON_AddItemToArray(rows, cols);
		} // for

		cJSON_AddItemToObject(root, "rows", rows);
		cJSON_AddStringToObject(root, "status", "OK");
	
		cJSON_AddNumberToObject(root, "result_code", ROUTE_RESULT_SUCCESS);
		cJSON_AddStringToObject(root, "msg", "success");
	} else {
		cJSON_AddStringToObject(root, "status", "UNKNOWN_ERROR ");
		cJSON_AddNumberToObject(root, "result_code", ROUTE_RESULT_FAILED);
		cJSON_AddStringToObject(root, "msg", "failed");
	}

	/*
	https://developers.google.com/maps/documentation/distance-matrix/distance-matrix?hl=ko

	OK indicates the response contains a valid result.
	INVALID_REQUEST indicates that the provided request was invalid.
	MAX_ELEMENTS_EXCEEDED indicates that the product of origins and destinations exceeds the per-query limit.
	MAX_DIMENSIONS_EXCEEDED indicates that the number of origins or destinations exceeds the per-query limit.
	OVER_DAILY_LIMIT indicates any of the following:
	The API key is missing or invalid.
	Billing has not been enabled on your account.
	A self-imposed usage cap has been exceeded.
	The provided method of payment is no longer valid (for example, a credit card has expired).
	OVER_QUERY_LIMIT indicates the service has received too many requests from your application within the allowed time period.
	REQUEST_DENIED indicates that the service denied use of the Distance Matrix service by your application.
	UNKNOWN_ERROR indicates a Distance Matrix request could not be processed due to a server error. The request may succeed if you try again.
	*/
		
	char* pJson = cJSON_Print(root);
	strJson.append(pJson);
	cJSON_free(pJson);

	cJSON_Delete(root);
#endif // #if defined(USE_CJSON)
}


//const size_t CRoutePackage::GetRouteProbablePath(IN const RouteResultInfo* pResult, OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length, IN const int32_t expansion, IN const double branchLength)
const size_t CRoutePackage::GetRouteProbablePath(IN unordered_map<uint64_t, int>& mapVisited, IN const vector<RouteResultLinkEx>& vtLinkInfo, OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length, IN const int32_t expansion, IN const double branchLength)
{
	// 경로선 노드의 경로선 외 링크 정보
	//const RouteResultInfo* pRouteResult = pResult;

	stLinkInfo* pLinkBefore = nullptr;
	stLinkInfo* pLinkNext = nullptr;

	if (vtLinkInfo.empty()) {
		return 0;
	}

	//for (int ii = 0; ii < pRouteResult->LinkInfo.size() - 1; ii++) {
	for (int ii = 0; ii < vtLinkInfo.size() - 1; ii++) {
		pLinkBefore = m_pDataMgr->GetLinkDataById(vtLinkInfo[ii].link_id, TYPE_LINK_DATA_NONE);
		pLinkNext = m_pDataMgr->GetLinkDataById(vtLinkInfo[ii + 1].link_id, TYPE_LINK_DATA_NONE);

		if (pLinkBefore == nullptr) {
			LOG_TRACE(LOG_WARNING, "failed, can't find fore link, idx:%d, id:%d", ii, vtLinkInfo[ii].link_id);
			// return 0;
			continue;
		} else if (pLinkNext == nullptr) {
			LOG_TRACE(LOG_WARNING, "failed, can't find next link, idx:%d, id:%d", ii + 1, vtLinkInfo[ii + 1].link_id);
			// return 0;
			continue;
		}

		stNodeInfo* pJctNode = nullptr;
		stLinkInfo* pJctLink = nullptr;
		int nJctDir = 0;

		// get junction node
		// <--- --->, <--- <---
		if (((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(0)) &&
			(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(0))) ||
			((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) &&
				(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1)))) {
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->snode_id, TYPE_NODE_DATA_NONE);
			nJctDir = 0; // 정
		}
		// ---> <--- , ---> --->
		else if (((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) &&
			(pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1))) ||
			((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(0)) &&
				(pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(0)))) {
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->enode_id, TYPE_NODE_DATA_NONE);
			nJctDir = 1; // 역
		}

		if (pJctNode == nullptr) {
			LOG_TRACE(LOG_WARNING, "failed, can't find junction node info, fore link:%d, next link:%d", pLinkBefore->link_id.nid, pLinkNext->link_id.nid);
			continue;
		}
		
		RouteProbablePath* pJctInfo = new RouteProbablePath;
		pJctInfo->LinkId = pLinkBefore->link_id;
		pJctInfo->SNodeId = pLinkBefore->snode_id;
		pJctInfo->ENodeId = pLinkBefore->enode_id;
		pJctInfo->NodeDir = nJctDir;
		//pJctInfo->NodeId = pJctNode->node_id;
		
		if (pJctNode->base.connnode_count >= 2) {
			for (int jj = 0; jj<pJctNode->base.connnode_count; jj++) {
				// 경로에 사용된 링크는 제외
				// 2회 이상 재귀되는 경우 추가 확장되는 링크는 검사 안함 
				if (!expansion && (vtLinkInfo[ii].node_id.llid != NOT_USE) && (pJctNode->connnodes[jj] == pLinkBefore->link_id || pJctNode->connnodes[jj] == pLinkNext->link_id)) {
					continue;
				}

				// 경로선 외 링크
				pJctLink = m_pDataMgr->GetLinkDataById(pJctNode->connnodes[jj], TYPE_LINK_DATA_NONE);

				// 이미 추가된 링크 제외
				if (pJctLink == nullptr || (mapVisited.find(pJctNode->connnodes[jj].llid) != mapVisited.end())) {
					continue;
				}

				// 의미 없는 링크 제외
#if defined(USE_PEDESTRIAN_DATA)
				// 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
				if ((pJctLink->ped.facility_type == 2) ||
					(pJctLink->ped.facility_type == 6) ||
					(pJctLink->ped.facility_type == 7) ||
					(pJctLink->ped.facility_type == 9) ||
					(pJctLink->ped.facility_type == 10)) {
					//LOG_TRACE(LOG_DEBUG, "---------------juntion snode slink(%d) sub continue: %lld, %.5f, %.5f", pNextLink->link_id.nid, pNextLink->link_id.llid, pNode->coord.x, pNode->coord.y);
					continue;
				}
#endif

#if defined(USE_P2P_DATA)
				// 4차선 미만 도로는 제외
				if (pJctLink->veh.lane_cnt < 4)
					continue;
#endif

				stLinkInfo* pLinkInfo = new stLinkInfo;
				pLinkInfo->link_id = pJctLink->link_id;
				pLinkInfo->length = pJctLink->length;
				pLinkInfo->snode_id = pJctLink->snode_id;
				pLinkInfo->enode_id = pJctLink->enode_id;
				
				bool isReverse = false;

				// 링크 방향성 // 버텍스 순서 변경
				// enode, * <---
				if ((pJctNode->coord.x == pJctLink->getVertexX(pJctLink->getVertexCount() - 1)) &&
					(pJctNode->coord.y == pJctLink->getVertexY(pJctLink->getVertexCount() - 1))) {
					isReverse = true;
				}
				//// snode, <--- --->, <--- <---
				//else if (((vtxNextNode.x == pJctLink->getVertexX(0)) &&
				//	(vtxNextNode.y == pJctLink->getVertexY(0))) ||
				//	((vtxNextNode.x == pJctLink->getVertexX(pJctLink->getVertexCount() - 1)) &&
				//		(vtxNextNode.y == pJctLink->getVertexY(pJctLink->getVertexCount() - 1)))) {
				//	;
				//}

				// 노드로부터 일정 거리 만큼만 사용
				if (expansion || branchLength <= 0 || pJctLink->length <= branchLength || pJctLink->getVertexCount() <= 2)  {
					pLinkInfo->setVertex(pJctLink->getVertex(), pJctLink->getVertexCount());
					if (isReverse) {
						pLinkInfo->reverseVertex();
					}
					pJctInfo->JctLinks.emplace(pLinkInfo->link_id.llid, pLinkInfo);
					mapVisited.emplace(pLinkInfo->link_id.llid, nJctDir); // 방문 처리
				}
				else {
					// 거리 따져서 링크 계산하는 코드는 앱에서 진행하자
					double jctLength = 0.f;
					int idxVtx = -1;
					int cntVertex = 1; // 기본 시작 vtx는 포함됨
					
					if (isReverse) {
						for (idxVtx = pJctLink->getVertexCount() - 2; idxVtx >= 0; --idxVtx, cntVertex++) {
							jctLength += getRealWorldDistance(pJctLink->getVertexX(idxVtx + 1), pJctLink->getVertexY(idxVtx + 1), pJctLink->getVertexX(idxVtx), pJctLink->getVertexY(idxVtx));
							if (jctLength >= branchLength) {
								cntVertex++;
								break;
							}
						} // for kk
					}
					else {
						for (idxVtx = 1; idxVtx < pJctLink->getVertexCount() - 1; idxVtx++, cntVertex++) {
							jctLength += getRealWorldDistance(pJctLink->getVertexX(idxVtx - 1), pJctLink->getVertexY(idxVtx - 1), pJctLink->getVertexX(idxVtx), pJctLink->getVertexY(idxVtx));
							if (jctLength >= branchLength) {
								cntVertex++;
								break;
							}
						} // for kk
					}

					if (idxVtx >= 0 && cntVertex > 0) {
						if (isReverse) {
							pLinkInfo->setVertex(pJctLink->getVertex() + idxVtx, cntVertex);
							pLinkInfo->reverseVertex();
						} else {
							pLinkInfo->setVertex(pJctLink->getVertex(), cntVertex);
						}
						pJctInfo->JctLinks.emplace(pLinkInfo->link_id.llid, pLinkInfo);
						mapVisited.emplace(pLinkInfo->link_id.llid, nJctDir); // 방문 처리
					}
				}

				// 정션이 요청된 길이에 미치치 못하면 남은 길이 만큼 추가 
				if (pJctLink->length < branchLength) {
					stNodeInfo* pNextJctNode = nullptr;

					if (pJctNode->node_id.llid == pJctLink->enode_id.llid) {
						pNextJctNode = m_pDataMgr->GetNodeDataById(pJctLink->snode_id, TYPE_NODE_DATA_NONE);
					} else {
						pNextJctNode = m_pDataMgr->GetNodeDataById(pJctLink->enode_id, TYPE_NODE_DATA_NONE);
					}
					
					// 다음 링크가 단일 링크 연결일 경우
					if (pNextJctNode && 
						(expansion && pNextJctNode->base.connnode_count >= 2) ||
						(!expansion && pNextJctNode->base.connnode_count == 2)) {
						// 현재 링크 저장
						vector<RouteResultLinkEx> vtNewLinkInfo;
						RouteResultLinkEx addLinkEx;
						addLinkEx.link_id = pJctLink->link_id;
						addLinkEx.node_id.llid = NOT_USE;
						vtNewLinkInfo.emplace_back(addLinkEx);

						if (pNextJctNode->base.connnode_count == 2) // kvx 스타일(다다음 링크는 단일 링크까지만)
						{
							addLinkEx.link_id = (pNextJctNode->connnodes[0] == pJctLink->link_id) ? pNextJctNode->connnodes[1] : pNextJctNode->connnodes[0];
							addLinkEx.node_id.llid = NOT_USE;
							vtNewLinkInfo.emplace_back(addLinkEx);
						}
						else {
							// 다음 링크 저장
							for (int idxNexLink = 0; idxNexLink < pNextJctNode->base.connnode_count; idxNexLink++) {
								// 이미 추가된 링크 제외
								if (mapVisited.find(pNextJctNode->connnodes[idxNexLink].llid) != mapVisited.end()) {
									continue;
								}
								//if (pNextJctNode->connnodes[idxNexLink] == pJctLink->link_id) {
								//	continue;
								//}
								addLinkEx.link_id = pNextJctNode->connnodes[idxNexLink];
								addLinkEx.node_id.llid = NOT_USE;
								vtNewLinkInfo.emplace_back(addLinkEx);
							}
						}

						// 재귀 함수 호출
						GetRouteProbablePath(mapVisited, vtNewLinkInfo, vtJctInfo, length, expansion, branchLength - pLinkInfo->length);
					}
				}
			} // for jj
		}
		else {
			// 정션 없지만 link_info 인덱스 맞추기 위해 삽입
			;
		}

		vtJctInfo.emplace_back(pJctInfo);
	} // for ii

	return vtJctInfo.size();
}


double getJctPropagation(IN CDataManager* pDataMgr, IN const stLinkInfo* pLink, IN const stNodeInfo* pNode, IN const unordered_map<uint64_t, int>* pmapVisited, IN unordered_set<uint64_t>& setVisited, IN double addedlength, IN const int32_t depth, IN const double branchLength)
{
	if (pDataMgr == nullptr || pLink == nullptr || pNode == nullptr || pmapVisited == nullptr || pmapVisited->empty()) {
		return addedlength;
	}

	unordered_set<uint64_t> setNewVisited;

	// node
	if ((pNode != nullptr) && (pNode->base.connnode_count >= 2)) {
		stLinkInfo* pJctLink = nullptr;
		stNodeInfo* pJctNode = nullptr;

		for (int jj = 0; jj<pNode->base.connnode_count; jj++) {

			if (pLink->link_id.llid == pNode->connnodes[jj].llid) { // 확장링크와 같은 링크 무시
				continue;
			}

			// 경로선 외 링크
			pJctLink = pDataMgr->GetLinkDataById(pNode->connnodes[jj], TYPE_LINK_DATA_NONE);

			// 이미 추가된 링크 제외
			if ((pJctLink == nullptr) || (pLink->link_id == pJctLink->link_id) || (pmapVisited->find(pNode->connnodes[jj].llid) != pmapVisited->end())) {
				continue;
			}

			// 의미 없는 링크 제외
#if defined(USE_PEDESTRIAN_DATA)
			// 시설물 타입, 0:미정의, 1:토끼굴, 2:지하보도, 3:육교, 4:고가도로, 5:교량, 6:지하철역, 7:철도, 8:중앙버스정류장, 9:지하상가, 10:건물관통도로, 11:단지도로_공원, 12:단지도로_주거시설, 13:단지도로_관광지, 14:단지도로_기타
			if ((pJctLink->ped.facility_type == 2) ||
				(pJctLink->ped.facility_type == 6) ||
				(pJctLink->ped.facility_type == 7) ||
				(pJctLink->ped.facility_type == 9) ||
				(pJctLink->ped.facility_type == 10)) {
				//LOG_TRACE(LOG_DEBUG, "---------------juntion snode slink(%d) sub continue: %lld, %.5f, %.5f", pNextLink->link_id.nid, pNextLink->link_id.llid, pNode->coord.x, pNode->coord.y);
				continue;
			}
#endif

#if defined(USE_P2P_DATA)
			// 4차선 미만 도로는 제외
			if (pJctLink->veh.lane_cnt < 4)
				continue;
#endif

			setVisited.emplace(pJctLink->link_id.llid); // 방문 처리

			// 지정된 길이까지만 확장
			if (branchLength >= 0 && addedlength + pJctLink->length >= branchLength) {
				continue;
			}
			
			stNodeInfo* pSnode = pDataMgr->GetNodeDataById(pJctLink->snode_id, TYPE_NODE_DATA_NONE);
			stNodeInfo* pEnode = pDataMgr->GetNodeDataById(pJctLink->enode_id, TYPE_NODE_DATA_NONE);

			if (pNode->coord == pSnode->coord) {
				pJctNode = pEnode;
			} else {
				pJctNode = pSnode;
			}

			// 재귀 함수 호출
			getJctPropagation(pDataMgr, pJctLink, pJctNode, pmapVisited, setVisited, addedlength + pJctLink->length, depth + 1, branchLength);
		} // for jj
	} // if node

	return addedlength;
}

const size_t CRoutePackage::GetRouteProbablePathEx(IN const unordered_map<uint64_t, int>* pmapVisited, IN double addedlength, IN const int32_t depth, IN const double branchLength, OUT vector<RouteProbablePath*>& vtJctInfo)
{
	if (pmapVisited == nullptr || pmapVisited->empty()) {
		return 0;
	}

	// 경로선 노드의 경로선 외 링크 정보
	stLinkInfo* pLink = nullptr;
	stNodeInfo* pNode = nullptr;

	unordered_set<uint64_t> setVisited;

	for (const auto linkId : *pmapVisited) {
		KeyID key = { key.llid = linkId.first };
		pLink = m_pDataMgr->GetLinkDataById(key, TYPE_LINK_DATA_NONE);

		if (pLink == nullptr) {
			LOG_TRACE(LOG_WARNING, "failed, can't find fore link, id:%d", linkId);
			// return 0;
			continue;
		}

		if (linkId.second == 0) { // 정 
			pNode = m_pDataMgr->GetNodeDataById(pLink->enode_id, TYPE_NODE_DATA_NONE);
		} else {
			pNode = m_pDataMgr->GetNodeDataById(pLink->snode_id, TYPE_NODE_DATA_NONE);;
		}

		getJctPropagation(m_pDataMgr, pLink, pNode, pmapVisited, setVisited, addedlength, depth, branchLength);
	} // 

	for (const auto& link : setVisited) {
		RouteProbablePath* pJctInfo = new RouteProbablePath;
		pJctInfo->LinkId.llid = link;
		//pJctInfo->NodeId = pJctNode->node_id;

		pLink = m_pDataMgr->GetLinkDataById(pJctInfo->LinkId, TYPE_LINK_DATA_NONE);

		if (pLink != nullptr) {
			stLinkInfo* pJctLink = new stLinkInfo;
			pJctLink->link_id = pLink->link_id;
			pJctLink->length = pLink->length;
			pJctLink->snode_id = pLink->snode_id;
			pJctLink->enode_id = pLink->enode_id;
			pJctLink->setVertex(pLink->getVertex(), pLink->getVertexCount());

			pJctInfo->JctLinks.emplace(pJctInfo->LinkId.llid, pJctLink);
		}

		vtJctInfo.emplace_back(pJctInfo);
	}

	return setVisited.size();
}


void CRoutePackage::GetOptimalPosition(IN const stReqOptimal* pRequest, IN const stOptimalPointInfo* pResult, OUT string& strJson)
{
   	int result_code = ROUTE_RESULT_FAILED;

#if defined(USE_CJSON)
	cJSON* root = cJSON_CreateObject();

	if (pRequest == nullptr || pResult == nullptr) {
		// header
		cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(ROUTE_RESULT_FAILED));
		cJSON_AddItemToObject(root, "error_msg", cJSON_CreateString("failed"));
	}
	else {
		int cntItems = pResult->vtEntryPoint.size();

		if (cntItems <= 0) {
			LOG_TRACE(LOG_ERROR, "Error, Optimal position result null");
			// mainobj->Set(context, String::NewFromUtf8(isolate, "msg").ToLocalChecked(), String::NewFromUtf8(isolate, "Error, Can not find optimal location").ToLocalChecked());
			// header
			cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(OPTIMAL_RESULT_FAILED));
			cJSON_AddItemToObject(root, "error_msg", cJSON_CreateString("no result"));
		}
		else {
			// header
			cJSON_AddItemToObject(root, "result_code", cJSON_CreateNumber(OPTIMAL_RESULT_SUCCESS));
			cJSON_AddItemToObject(root, "error_msg", cJSON_CreateString("success"));

			// data
			cJSON* data = cJSON_CreateObject();

			cJSON_AddItemToObject(data, "result", cJSON_CreateBool(true));
			cJSON_AddItemToObject(data, "count", cJSON_CreateNumber(cntItems));

			// items
			cJSON* items = cJSON_CreateArray();
			for(const auto & it : pResult->vtEntryPoint) {
				cJSON* item = cJSON_CreateObject();
				cJSON_AddItemToObject(item, "x", cJSON_CreateNumber(it.x));
				cJSON_AddItemToObject(item, "y", cJSON_CreateNumber(it.y));
				cJSON_AddItemToObject(item, "type", cJSON_CreateNumber(it.nAttribute));
				cJSON_AddItemToObject(item, "angle", cJSON_CreateNumber(it.nAngle));
				cJSON_AddItemToArray(items, item);
			}
			// entrypoints
			cJSON_AddItemToObject(data, "entrypoints", items);

			// add data
			cJSON_AddItemToObject(root, "data", data);


			// add more expand
			if (pRequest->isExpand == true) {
				// expand
				cJSON* expand = cJSON_CreateObject();
				
				// request position
				cJSON_AddItemToObject(expand, "x", cJSON_CreateNumber(pRequest->x));
				cJSON_AddItemToObject(expand, "y", cJSON_CreateNumber(pRequest->y));

				// type
				cJSON_AddItemToObject(expand, "type", cJSON_CreateNumber(pResult->nType));
				
				// name
				if (!pResult->name.empty()) {
		#if defined(_WIN32)
					char szUTF8[MAX_PATH] = {0,};
					MultiByteToUTF8(pResult->name.c_str(), szUTF8);
					cJSON_AddStringToObject(expand, "name", szUTF8);
		#else
					cJSON_AddStringToObject(expand, "name", pResult->name.c_str());
		#endif
				}

				// vertices
				cJSON* vertices = cJSON_CreateArray();
				for(int ii=pResult->vtPolygon.size() - 1; ii >= 0; --ii) {
					cJSON* vertex = cJSON_CreateArray();
					cJSON_AddItemToArray(vertex, cJSON_CreateNumber(pResult->vtPolygon[ii].x));
					cJSON_AddItemToArray(vertex, cJSON_CreateNumber(pResult->vtPolygon[ii].y));
					cJSON_AddItemToArray(vertices, vertex);
				}
				cJSON_AddItemToObject(expand, "vertices", vertices);

				// id
				cJSON_AddItemToObject(expand, "id", cJSON_CreateNumber(pResult->id));

				// add expand
				cJSON_AddItemToObject(root, "expand", expand);
			} // is expand
		}
	}

	char* pJson = cJSON_Print(root);
	strJson.append(pJson);
	cJSON_free(pJson);
	
	cJSON_Delete(root);
	
#else //#if defined(USE_CJSON)


#endif // #if defined(USE_CJSON)

	if (!strJson.empty()) {
		// mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
	}
}