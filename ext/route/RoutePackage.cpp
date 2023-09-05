#include "../include/MapDef.h"

#include "RoutePackage.h"

#define USE_CJSON
#if defined(USE_CJSON)
#include "../libjson/cjson/cJSON.h"
#endif

#include "../utils/Strings.h"



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


void CRoutePackage::GetMultiRouteResult(IN const RouteResultInfo* pResult, OUT string& strJson)
{
	int result_code = ROUTE_RESULT_FAILED;
	int target = 0;
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
		cJSON* summarys = cJSON_CreateArray();
		int cntRoutes = pResult->RouteSummarys.size();
		uint32_t remained_dist = pResult->TotalLinkDist;
		uint32_t remained_time = pResult->TotalLinkTime;
		// - now
		time_t timer = time(NULL);
		struct tm* tmNow;
		string strEta;
		for (const auto& summary : pResult->RouteSummarys) {
			// - summary
			cJSON* pSummary = cJSON_CreateObject();
			// - distance
			cJSON_AddItemToObject(pSummary, "distance", cJSON_CreateNumber(summary.TotalDist));
			// - time
			cJSON_AddItemToObject(pSummary, "time", cJSON_CreateNumber(summary.TotalTime));

			// - remained distance
			cJSON_AddItemToObject(pSummary, "remain_distance", cJSON_CreateNumber(remained_dist));
			if (remained_dist < summary.TotalDist) {
				remained_dist = 0;
			} else {
				remained_dist -= summary.TotalDist;
			}

			// - remained time
			cJSON_AddItemToObject(pSummary, "remain_time", cJSON_CreateNumber(remained_time));
			if (remained_time < summary.TotalTime) {
				remained_time = 0;
			} else {
				remained_time -= summary.TotalTime;
			}

			//// now
			//tmNow = localtime(&timer);
			//string strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
			//cJSON_AddItemToObject(summary, "now", cJSON_CreateString(strVal.c_str()));

			// eta
			timer += summary.TotalTime;
			tmNow = localtime(&timer);
			//string strVal = string_format("%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
			strEta = string_format("%02d:%02d:%02d", tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
			cJSON_AddItemToObject(pSummary, "eta", cJSON_CreateString(strEta.c_str()));

			cJSON_AddItemToArray(summarys, pSummary);
		} // for
		cJSON_AddItemToObject(root, "summarys", summarys);

		// routes
		cJSON* routes = cJSON_CreateArray();
		cntRoutes = 1;
		for (int ii = 0; ii<cntRoutes; ii++) {
			// - summary
			cJSON* summary = cJSON_CreateObject();
			// - type
			cJSON_AddItemToObject(summary, "type", cJSON_CreateNumber(pResult->LinkInfo[ii].link_info));
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
			for (int jj = 0; jj<cntLinks; jj++) {
				cJSON* idoff = cJSON_CreateObject();
				cJSON_AddItemToObject(idoff, "id", cJSON_CreateNumber(pResult->LinkInfo[jj].link_id.nid));
				cJSON_AddItemToObject(idoff, "length", cJSON_CreateNumber(pResult->LinkInfo[jj].length));
				cJSON_AddItemToObject(idoff, "time", cJSON_CreateNumber(pResult->LinkInfo[jj].time));
				cJSON_AddItemToObject(idoff, "angle", cJSON_CreateNumber(pResult->LinkInfo[jj].angle));
				cJSON_AddItemToObject(idoff, "vertex_offset", cJSON_CreateNumber(pResult->LinkInfo[jj].vtx_off));
				cJSON_AddItemToObject(idoff, "vertex_count", cJSON_CreateNumber(pResult->LinkInfo[jj].vtx_cnt));
				cJSON_AddItemToObject(idoff, "remain_distance", cJSON_CreateNumber(pResult->LinkInfo[jj].rlength));
				cJSON_AddItemToObject(idoff, "remain_time", cJSON_CreateNumber(pResult->LinkInfo[jj].rtime));
				cJSON_AddItemToObject(idoff, "guide_type", cJSON_CreateNumber(pResult->LinkInfo[jj].type));

				// 링크 부가 정보 
				uint64_t sub_info = pResult->LinkInfo[ii].link_info;
				if (sub_info > 0) {
					stLinkBaseInfo* pBaseInfo = reinterpret_cast<stLinkBaseInfo*>(&sub_info);

					cJSON_AddItemToObject(idoff, "type", cJSON_CreateNumber(pBaseInfo->link_type));
					// 보행자 데이터
					if (pBaseInfo && pBaseInfo->link_type == TYPE_DATA_PEDESTRIAN) {
						stLinkPedestrianInfo* pPedInfo = reinterpret_cast<stLinkPedestrianInfo*>(&sub_info);
						cJSON_AddItemToObject(idoff, "facility_type", cJSON_CreateNumber(pPedInfo->facility_type));
						cJSON_AddItemToObject(idoff, "gate_type", cJSON_CreateNumber(pPedInfo->gate_type));
					}
					else {
						cJSON_AddItemToObject(idoff, "facility_type", cJSON_CreateNumber(9));
						cJSON_AddItemToObject(idoff, "gate_type", cJSON_CreateNumber(9));
					}
				}
				cJSON_AddItemToArray(links, idoff);
			}
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

			/*
			if (target == ROUTE_TARGET_KAKAOVX) {
				// junction_info
				cJSON* junctions = cJSON_CreateArray();
				vector<RouteProbablePath*> vtRpp;
				int cntRpp = pResult->GetRouteProbablePath(vtRpp);
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
			*/


			// increse route
			cJSON_AddItemToArray(routes, route);
		}

		// add routes
		cJSON_AddItemToObject(root, "routes", routes);

		strJson = cJSON_Print(root);

		cJSON_Delete(root);
	}

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