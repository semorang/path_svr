#include "RoutePackage.h"

#include "../utils/Strings.h"

#define USE_CJSON
#if defined(USE_CJSON)
#include "../libjson/cjson/cJSON.h"
#endif

CRoutePackage::CRoutePackage()
{
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


void CRoutePackage::GetMultiRouteResultForiNavi(IN const RouteResultInfo* pResult, OUT string& strJson)
{
	//string strJson;

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
					cJSON_AddStringToObject(path, "road_name", encoding(m_pDataMgr.GetNameDataByIdx(pLink->name_idx), "euc-kr", "utf-8"));
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


		// add header to root
		cJSON_AddNumberToObject(root, "user_id", pResult->RequestId);
		cJSON_AddNumberToObject(root, "result_code", result_code);
		cJSON_AddStringToObject(root, "error_msg", str_msg.c_str());

		cJSON_AddItemToObject(root, "routes", routes);

		strJson = cJSON_Print(root);

		cJSON_Delete(root);
	}

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