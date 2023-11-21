#include "../include/MapDef.h"

#include "RoutePackage.h"

#define USE_CJSON
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


void CRoutePackage::GetErrorResult(IN const int32_t err_code, IN const char* err_msg,  OUT string& strJson)
{
	// string strJson;

#if defined(USE_CJSON)
	cJSON* root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "status", "failed");
	cJSON_AddNumberToObject(root, "result_code", err_code);
	cJSON_AddStringToObject(root, "msg", err_msg);

	strJson = cJSON_Print(root);

	cJSON_Delete(root);
#endif // #if defined(USE_CJSON)
}


void CRoutePackage::GetMultiRouteResult(IN const RouteResultInfo* pResult, IN const int target, OUT string& strJson)
{
	int result_code = ROUTE_RESULT_FAILED;
	string str_msg = "";

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
		// uint32_t remained_dist = pResult->TotalLinkDist;
		// uint32_t remained_time = pResult->TotalLinkTime;
		// - now
		time_t timer = time(NULL);
		struct tm* tmNow;
		string strEta;
		// for (const auto& summary : pResult->RouteSummarys) {
		// 	// - summary
		// 	cJSON* pSummary = cJSON_CreateObject();
		// 	// - distance
		// 	cJSON_AddItemToObject(pSummary, "distance", cJSON_CreateNumber(summary.TotalDist));
		// 	// - time
		// 	cJSON_AddItemToObject(pSummary, "time", cJSON_CreateNumber(summary.TotalTime));

		// 	// - remained distance
		// 	cJSON_AddItemToObject(pSummary, "remain_distance", cJSON_CreateNumber(remained_dist));
		// 	if (remained_dist < summary.TotalDist) {
		// 		remained_dist = 0;
		// 	} else {
		// 		remained_dist -= summary.TotalDist;
		// 	}

		// 	// - remained time
		// 	cJSON_AddItemToObject(pSummary, "remain_time", cJSON_CreateNumber(remained_time));
		// 	if (remained_time < summary.TotalTime) {
		// 		remained_time = 0;
		// 	} else {
		// 		remained_time -= summary.TotalTime;
		// 	}

		// 	//// now
		// 	//tmNow = localtime(&timer);
		// 	//string strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
		// 	//cJSON_AddItemToObject(summary, "now", cJSON_CreateString(strVal.c_str()));

		// 	// eta
		// 	timer += summary.TotalTime;
		// 	tmNow = localtime(&timer);
		// 	//string strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
		// 	strEta = string_format("%02d:%02d:%02d", tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
		// 	cJSON_AddItemToObject(pSummary, "eta", cJSON_CreateString(strEta.c_str()));

		// 	cJSON_AddItemToArray(summarys, pSummary);
		// } // for
		// cJSON_AddItemToObject(root, "summarys", summarys);

		// routes
		cJSON* routes = cJSON_CreateArray();
		cntRoutes = 1;
		for (int ii = 0; ii<cntRoutes; ii++) {
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
			cJSON* route = cJSON_CreateObject();
			cJSON_AddItemToObject(route, "summary", summary);
			

			// - link_info
			cJSON* links = cJSON_CreateArray();
			int cntLinks = pResult->LinkInfo.size();
			int vertex_offset = 0;
			for (int jj = 0; jj<cntLinks; jj++) {
				cJSON* idoff = cJSON_CreateObject();
				cJSON_AddItemToObject(idoff, "id", cJSON_CreateNumber(pResult->LinkInfo[jj].link_id.nid));
				cJSON_AddItemToObject(idoff, "length", cJSON_CreateNumber(pResult->LinkInfo[jj].length));
				cJSON_AddItemToObject(idoff, "time", cJSON_CreateNumber(pResult->LinkInfo[jj].time));
				cJSON_AddItemToObject(idoff, "angle", cJSON_CreateNumber(pResult->LinkInfo[jj].angle));
				//cJSON_AddItemToObject(idoff, "vertex_offset", cJSON_CreateNumber(pResult->LinkInfo[jj].vtx_off));
				cJSON_AddItemToObject(idoff, "vertex_offset", cJSON_CreateNumber(vertex_offset));
				cJSON_AddItemToObject(idoff, "vertex_count", cJSON_CreateNumber(pResult->LinkInfo[jj].vtx_cnt));
				cJSON_AddItemToObject(idoff, "remain_distance", cJSON_CreateNumber(pResult->LinkInfo[jj].rlength));
				cJSON_AddItemToObject(idoff, "remain_time", cJSON_CreateNumber(pResult->LinkInfo[jj].rtime));
				cJSON_AddItemToObject(idoff, "guide_type", cJSON_CreateNumber(pResult->LinkInfo[jj].type));

				vertex_offset += pResult->LinkInfo[jj].vtx_cnt; // 경유지에서 offset 초기화 되지 않도록

				// 링크 부가 정보 
				uint64_t sub_info = pResult->LinkInfo[jj].link_info;
				if (sub_info > 0) {
					stLinkBaseInfo* pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&sub_info);

					// 숲길 데이터
					if (pBaseInfo && pBaseInfo->link_type == TYPE_DATA_TREKKING) {
						stLinkTrekkingInfo* pForestInfo = reinterpret_cast<stLinkTrekkingInfo*>(&sub_info);
						cJSON_AddItemToObject(idoff, "type", cJSON_CreateNumber(ROUTE_TYPE_TREKKING));
						cJSON_AddItemToObject(idoff, "course_type", cJSON_CreateNumber(pForestInfo->course_type));
						cJSON_AddItemToObject(idoff, "road_type", cJSON_CreateNumber(pForestInfo->road_info));
						cJSON_AddItemToObject(idoff, "difficult", cJSON_CreateNumber(pForestInfo->diff));
						cJSON_AddItemToObject(idoff, "slop", cJSON_CreateNumber(pForestInfo->slop));
						cJSON_AddItemToObject(idoff, "popular", cJSON_CreateNumber(pForestInfo->popular));
					}
					// 보행자 데이터
					else if (pBaseInfo && pBaseInfo->link_type == TYPE_DATA_PEDESTRIAN) {
						stLinkPedestrianInfo* pPedInfo = reinterpret_cast<stLinkPedestrianInfo*>(&sub_info);
						cJSON_AddItemToObject(idoff, "type", cJSON_CreateNumber(ROUTE_TYPE_PEDESTRIAN));
						cJSON_AddItemToObject(idoff, "facility_type", cJSON_CreateNumber(pPedInfo->facility_type));
						cJSON_AddItemToObject(idoff, "gate_type", cJSON_CreateNumber(pPedInfo->gate_type));
					}					
					else {
						cJSON_AddItemToObject(idoff, "type", cJSON_CreateNumber(0));
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
			for (auto jj = 0; jj<cntVertices; jj++) {
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

			if (target == ROUTE_TARGET_KAKAOVX) {
				// junction_info
				cJSON* junctions = cJSON_CreateArray();
				vector<RouteProbablePath*> vtRpp;
				int cntRpp = GetRouteProbablePath(pResult->LinkInfo, vtRpp);
				for (auto jj = 0; jj<cntRpp; jj++) {
					cJSON* link_info = cJSON_CreateObject();
					cJSON* links = cJSON_CreateArray();
					RouteProbablePath* pRpp = vtRpp[jj];
					for (auto kk = 0; kk<pRpp->JctLinks.size(); kk++) {
						cJSON* jct_info = cJSON_CreateObject();
						cJSON* link_vtx = cJSON_CreateArray();
						stLinkInfo* pLink = pRpp->JctLinks[kk];
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

				  // add routes - junction_info
				cJSON_AddItemToObject(route, "junction_info", junctions);
			}

			// increse route
			cJSON_AddItemToArray(routes, route);
		} // for

		// add routes
		cJSON_AddItemToObject(root, "routes", routes);
	}

	strJson = cJSON_Print(root);

	cJSON_Delete(root);

#else //#if defined(USE_CJSON)


#endif // #if defined(USE_CJSON)
}


void CRoutePackage::GetMultiRouteResultForiNavi(IN const RouteResultInfo* pResult, OUT string& strJson)
{
	int result_code = ROUTE_RESULT_FAILED;
	string str_msg = "";

#if defined(USE_CJSON)
	cJSON* root = cJSON_CreateObject();

	if (pResult == nullptr) {
		result_code = ROUTE_RESULT_FAILED;
		str_msg = "failed";
	}
	else if (pResult->ResultCode != ROUTE_RESULT_SUCCESS) {
		result_code = pResult->ResultCode;
		str_msg = "failed";
	}
	else {
		result_code = ROUTE_RESULT_SUCCESS;
		str_msg = "success";

		cJSON* routes = cJSON_CreateArray();
		cJSON* data = cJSON_CreateObject();

		cJSON_AddItemToObject(data, "option", cJSON_CreateNumber(pResult->RouteOption));
		cJSON_AddItemToObject(data, "spend_time", cJSON_CreateNumber(pResult->TotalLinkTime));
		cJSON_AddItemToObject(data, "distance", cJSON_CreateNumber(pResult->TotalLinkDist));
		cJSON_AddItemToObject(data, "toll_fee", cJSON_CreateNumber(0));
		cJSON_AddItemToObject(data, "taxiFare", cJSON_CreateNumber(0));
		cJSON_AddItemToObject(data, "isHighWay", cJSON_CreateBool(false));

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

				// type 링크 타입, 0:일반, 1:출발지링크, 2:도착지링크, 3:경유지링크
				cJSON_AddNumberToObject(p2p, "type", link.type);

				// ang, 진행각
				cJSON_AddNumberToObject(p2p, "ang", link.angle);

				// add p2p to path
				cJSON_AddItemToObject(path, "p2p_extend", p2p);
			}
#endif // #if 1 defined(USE_P2P_DATA)

			// add path to paths
			cJSON_AddItemToArray(paths, path);

		} // for paths

		// add paths to data
		cJSON_AddItemToObject(data, "paths", paths);
		cJSON_AddItemToArray(routes, data);

		// add data to root
		cJSON_AddItemToObject(root, "routes", routes);
	}

	// add header to root
	cJSON_AddNumberToObject(root, "user_id", pResult->RequestId);
	cJSON_AddNumberToObject(root, "result_code", result_code);
	cJSON_AddStringToObject(root, "error_msg", str_msg.c_str());

	strJson = cJSON_Print(root);

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
#endif

	// add route to root
	if (!strJson.empty()) {
		// mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
	}
}


void CRoutePackage::GetClusteringResult(IN const vector<stDistrict>& vtClusters, OUT string& strJson)
{
	//string strJson;
	int cntClusters = vtClusters.size();;

	int result_code = ROUTE_RESULT_FAILED;
	string str_msg = "";

	if (cntClusters <= 0) {
		result_code = ROUTE_RESULT_FAILED;
		str_msg = "failed";
	}
	else {
		result_code = ROUTE_RESULT_SUCCESS;
		str_msg = "success";

#if defined(USE_CJSON)
		cJSON* root = cJSON_CreateObject();
		cJSON* clusters = cJSON_CreateArray();

		for (const auto& cluster : vtClusters) {
			cJSON* group = cJSON_CreateObject();

			// id
			cJSON_AddStringToObject(group, "group", cluster.name.c_str());

			// pois
			cJSON* pois = cJSON_CreateArray();
			for (const auto& poi : cluster.pois) {
				double arrLoc[2] = { poi.coord.x, poi.coord.y };
				cJSON* coord = cJSON_CreateDoubleArray(arrLoc, 2);
				cJSON_AddItemToArray(pois, coord);
			} // for
			cJSON_AddItemToObject(group, "pois", pois);

			  // boundary
			cJSON* boundary = cJSON_CreateArray();
			for (const auto& border : cluster.border) {
				double arrLoc[2] = { border.x, border.y };
				cJSON* coord = cJSON_CreateDoubleArray(arrLoc, 2);
				cJSON_AddItemToArray(boundary, coord);
			} // for
			cJSON_AddItemToObject(group, "boundary", boundary);

			cJSON_AddItemToArray(clusters, group);
		} // for

		cJSON_AddItemToObject(root, "clusters", clusters);

		cJSON_AddStringToObject(root, "status", "OK");
		cJSON_AddNumberToObject(root, "result_code", result_code);
		cJSON_AddStringToObject(root, "msg", "success");

		strJson = cJSON_Print(root);

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
	string str_msg = "";

	if (cntPolygon <= 0) {
		result_code = ROUTE_RESULT_FAILED;
		str_msg = "failed";
	}
	else {
		result_code = ROUTE_RESULT_SUCCESS;
		str_msg = "success";

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

		strJson = cJSON_Print(root);

		cJSON_Delete(root);
	}

	// add route to root
	if (!strJson.empty()) {
		// mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
	}
#endif // #if defined(USE_CJSON)
}


//const size_t CRoutePackage::GetRouteProbablePath(IN const RouteResultInfo* pResult, OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length, IN const int32_t expansion, IN const double branchLength)
const size_t CRoutePackage::GetRouteProbablePath(IN const vector<RouteResultLinkEx>& vtLinkInfo, OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length, IN const int32_t expansion, IN const double branchLength)
{
	// 경로선 노드의 경로선 외 링크 정보
	//const RouteResultInfo* pRouteResult = pResult;

	stLinkInfo* pLinkBefore = nullptr;
	stLinkInfo* pLinkNext = nullptr;

	//for (int ii = 0; ii < pRouteResult->LinkInfo.size() - 1; ii++) {
	for (int ii = 0; ii < vtLinkInfo.size() - 1; ii++) {
#if defined(USE_TREKKING_DATA)
		pLinkBefore = m_pDataMgr->GetLinkDataById(vtLinkInfo[ii].link_id);
		pLinkNext = m_pDataMgr->GetLinkDataById(vtLinkInfo[ii + 1].link_id);
#elif defined(USE_PEDESTRIAN_DATA)
		pLinkBefore = m_pDataMgr->GetWLinkDataById(vtLinkInfo[ii].link_id);
		pLinkNext = m_pDataMgr->GetWLinkDataById(vtLinkInfo[ii + 1].link_id);
#elif defined(USE_VEHICLE_DATA)
		pLinkBefore = m_pDataMgr->GetVLinkDataById(vtLinkInfo[ii].link_id);
		pLinkNext = m_pDataMgr->GetVLinkDataById(vtLinkInfo[ii + 1].link_id);
#else
		pLinkBefore = m_pDataMgr->GetLinkDataById(vtLinkInfo[ii].link_id);
		pLinkNext = m_pDataMgr->GetLinkDataById(vtLinkInfo[ii + 1].link_id);
#endif

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

		// get junction node
		// <--- --->, <--- <---
		if (((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(0)) &&
			(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(0))) ||
			((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) &&
				(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1)))) {
#if defined(USE_TREKKING_DATA)
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->snode_id);
#elif defined(USE_PEDESTRIAN_DATA)
			pJctNode = m_pDataMgr->GetWNodeDataById(pLinkBefore->snode_id);
#elif defined(USE_VEHICLE_DATA)
			pJctNode = m_pDataMgr->GetVNodeDataById(pLinkBefore->snode_id);
#else
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->snode_id);
#endif			
		}
		// ---> <--- , ---> --->
		else if (((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) &&
			(pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1))) ||
			((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(0)) &&
				(pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(0)))) {
#if defined(USE_TREKKING_DATA)
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->enode_id);
#elif defined(USE_PEDESTRIAN_DATA)
			pJctNode = m_pDataMgr->GetWNodeDataById(pLinkBefore->enode_id);
#elif defined(USE_VEHICLE_DATA)
			pJctNode = m_pDataMgr->GetVNodeDataById(pLinkBefore->enode_id);
#else
			pJctNode = m_pDataMgr->GetNodeDataById(pLinkBefore->enode_id);
#endif							
		}

		if (pJctNode == nullptr) {
			LOG_TRACE(LOG_WARNING, "failed, can't find junction node info, fore link:%d, next link:%d", pLinkBefore->link_id.nid, pLinkNext->link_id.nid);
			continue;
		}
		
		RouteProbablePath* pJctInfo = new RouteProbablePath;
		pJctInfo->LinkId = pLinkBefore->link_id;
		pJctInfo->NodeId = pJctNode->node_id;
		
		if (pJctNode->base.connnode_count >= 2) {
			for (int jj = 0; jj<pJctNode->base.connnode_count; jj++) {
				// 경로에 사용된 링크는 제외
				// 2회 이상 재귀되는 경우 추가 확장되는 링크는 검사 안함 
				if (vtLinkInfo[ii].node_id.llid != NOT_USE && (pJctNode->connnodes[jj] == pLinkBefore->link_id || pJctNode->connnodes[jj] == pLinkNext->link_id)) {
					continue;
				}			

				// 경로선 외 링크
#if defined(USE_TREKKING_DATA)
				pJctLink = m_pDataMgr->GetLinkDataById(pJctNode->connnodes[jj]);
#elif defined(USE_PEDESTRIAN_DATA)
				pJctLink = m_pDataMgr->GetWLinkDataById(pJctNode->connnodes[jj]);
#elif defined(USE_VEHICLE_DATA)
				pJctLink = m_pDataMgr->GetVLinkDataById(pJctNode->connnodes[jj]);
#else
				pJctLink = m_pDataMgr->GetLinkDataById(pJctNode->connnodes[jj]);
#endif				

#if defined(USE_P2P_DATA)
				// 4차선 미만 도로는 제외
				if (pJctLink->veh.lane_cnt < 4)
					continue;
#endif

				stLinkInfo* pLinkInfo = new stLinkInfo;
				pLinkInfo->link_id = pJctLink->link_id;
				pLinkInfo->length = pJctLink->length;
				pLinkInfo->snode_id = pJctNode->node_id;
				
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
				if (branchLength <= 0 || pJctLink->length <= branchLength || pJctLink->getVertexCount() <= 2)  {
					pLinkInfo->setVertex(pJctLink->getVertex(), pJctLink->getVertexCount());
					if (isReverse) {
						pLinkInfo->reverseVertex();
					}
					pJctInfo->JctLinks.push_back(pLinkInfo);
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
						}
						else {
							pLinkInfo->setVertex(pJctLink->getVertex(), cntVertex);
						}
						pJctInfo->JctLinks.push_back(pLinkInfo);
					}
				}

				// 요청 길이에 못미치는 정션이 요청된 길이에 미치치 못하면 남은 길이 만큼 추가 
				if (pJctLink->length < branchLength) {
					stNodeInfo* pNextJctNode = nullptr;

					if (pJctNode->node_id.llid == pJctLink->enode_id.llid) {
#if defined(USE_TREKKING_DATA)
						pNextJctNode = m_pDataMgr->GetNodeDataById(pJctLink->snode_id);
#elif defined(USE_PEDESTRIAN_DATA)
						pNextJctNode = m_pDataMgr->GetWNodeDataById(pJctLink->snode_id);
#elif defined(USE_VEHICLE_DATA)
						pNextJctNode = m_pDataMgr->GetVNodeDataById(pJctLink->snode_id);
#else
						pNextJctNode = m_pDataMgr->GetNodeDataById(pJctLink->snode_id);
#endif
					}
					else {
#if defined(USE_TREKKING_DATA)
						pNextJctNode = m_pDataMgr->GetNodeDataById(pJctLink->enode_id);
#elif defined(USE_PEDESTRIAN_DATA)
						pNextJctNode = m_pDataMgr->GetWNodeDataById(pJctLink->enode_id);
#elif defined(USE_VEHICLE_DATA)
						pNextJctNode = m_pDataMgr->GetVNodeDataById(pJctLink->enode_id);
#else
						pNextJctNode = m_pDataMgr->GetNodeDataById(pJctLink->enode_id);
#endif
					}
					
					// 다음 링크가 단일 링크 연결일 경우
					if (pNextJctNode && pNextJctNode->base.connnode_count == 2) {
						// 현재 링크 저장
						vector<RouteResultLinkEx> vtNewLinkInfo;
						RouteResultLinkEx addLinkEx;
						addLinkEx.link_id = pJctLink->link_id;
						addLinkEx.node_id.llid = NOT_USE;
						vtNewLinkInfo.emplace_back(addLinkEx);

						// 다음 링크 저장
						addLinkEx.link_id = (pNextJctNode->connnodes[0] == pJctLink->link_id) ? pNextJctNode->connnodes[1] : pNextJctNode->connnodes[0];
						addLinkEx.node_id.llid = NOT_USE;
						vtNewLinkInfo.emplace_back(addLinkEx);

						// 재귀 함수 호출
						GetRouteProbablePath(vtNewLinkInfo, vtJctInfo, length, expansion, branchLength - pLinkInfo->length);
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


void CRoutePackage::GetOptimalPosition(IN const stReqOptimal* pRequest, IN const stOptimalPointInfo* pResult, OUT string& strJson)
{
   	int result_code = ROUTE_RESULT_FAILED;
	string str_msg = "";

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

	strJson = cJSON_Print(root);
	
	cJSON_Delete(root);
	
#else //#if defined(USE_CJSON)


#endif // #if defined(USE_CJSON)

	if (!strJson.empty()) {
		// mainobj->Set(context, String::NewFromUtf8(isolate, "route").ToLocalChecked(), String::NewFromUtf8(isolate, strJson.c_str()).ToLocalChecked());
	}
}