#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "DataManager.h"

#if defined(USE_INAVI_STATIC_DATA)
#include "../thlib/ReadTTLSpd.h"
#endif

#include "../shp/shpio.h"
#include "../utils/UserLog.h"
#include "../utils/GeoTools.h"
#include "../utils/Strings.h"
#include "../utils/DataConvertor.h"
#include "MMPoint.hpp"
#if defined(USE_CJSON)
#include "../libjson/cjson/cJSON.h"
#endif



#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CDataManager::CDataManager()
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	memset(m_szDataPath, 0x00, sizeof(m_szDataPath));

	m_pMapName = nullptr;
	m_pMapMesh = nullptr;
	m_pMapFNode = nullptr;
	m_pMapFLink = nullptr;
	m_pMapFExtend = nullptr;
	m_pMapWNode = nullptr;
	m_pMapWLink = nullptr;
	m_pMapWExtend = nullptr;
	m_pMapVNode = nullptr;
	m_pMapVLink = nullptr;
	m_pMapVExtend = nullptr;
	m_pMapComplex = nullptr;
	m_pMapBuilding = nullptr;
	m_pMapTraffic = nullptr;
	m_pMapCourse = nullptr;
#if defined(USE_INAVI_STATIC_DATA)
	m_pStaticMgr = nullptr;
#endif

	m_pMapName = new MapName();
	m_pMapMesh = new MapMesh();
#if defined(USE_FOREST_DATA)
	m_pMapFNode = new MapNode();
	m_pMapFLink = new MapLink();
	m_pMapFExtend = new MapExtend();
	m_pMapCourse = new MapCourse();
#endif
#if defined(USE_PEDESTRIAN_DATA)
	m_pMapWNode = new MapNode();
	m_pMapWLink = new MapLink();
	m_pMapWExtend = new MapExtend();
#endif
#if defined(USE_VEHICLE_DATA)
	m_pMapVNode = new MapNode();
	m_pMapVLink = new MapLink();
	m_pMapVExtend = new MapExtend();

	m_pMapTraffic = new MapTraffic();
#endif
#if defined(USE_COMPLEX_DATA) || defined(USE_MOUNTAIN_DATA)
	m_pMapComplex = new MapPolygon();
#endif
#if defined(USE_BUILDING_DATA)
	m_pMapBuilding = new MapPolygon();
#endif

//#if defined (USE_PEDESTRIAN_DATA)
//	m_trackShpMgr.SetFileManager(this);
//#endif
#if defined(USE_INAVI_STATIC_DATA)
	m_pStaticMgr = new ReadTTLSpd();
#endif
}

CDataManager::~CDataManager()
{
	Release();

	if (m_pMapName) {
		SAFE_DELETE(m_pMapName);
	}
	if (m_pMapMesh) {
		SAFE_DELETE(m_pMapMesh);
	}
	if (m_pMapFNode) {
		SAFE_DELETE(m_pMapFNode);
	}
	if (m_pMapFLink) {
		SAFE_DELETE(m_pMapFLink);
	}
	if (m_pMapFExtend) {
		SAFE_DELETE(m_pMapFExtend);
	}
	if (m_pMapWNode) {
		SAFE_DELETE(m_pMapWNode);
	}
	if (m_pMapWLink) {
		SAFE_DELETE(m_pMapWLink);
	}
	if (m_pMapWExtend) {
		SAFE_DELETE(m_pMapWExtend);
	}
	if (m_pMapVNode) {
		SAFE_DELETE(m_pMapVNode);
	}
	if (m_pMapVLink) {
		SAFE_DELETE(m_pMapVLink);
	}
	if (m_pMapVExtend) {
		SAFE_DELETE(m_pMapVExtend);
	}
	if (m_pMapComplex) {
		SAFE_DELETE(m_pMapComplex);
	}
	if (m_pMapBuilding) {
		SAFE_DELETE(m_pMapBuilding);
	}
	if (m_pMapTraffic) {
		SAFE_DELETE(m_pMapTraffic);
	}
	if (m_pMapCourse) {
		SAFE_DELETE(m_pMapCourse);
	}
#if defined(USE_INAVI_STATIC_DATA)
	if (m_pStaticMgr) {
		SAFE_DELETE(m_pStaticMgr);
	}
#endif
	if (!m_mapFileIndex.empty()) {
		unordered_map<uint32_t, FileIndex>().swap(m_mapFileIndex);
		m_mapFileIndex.clear();
	}

	//_CrtDumpMemoryLeaks();
}


bool CDataManager::Initialize(void)
{
	if (m_pMapName) {
		m_pMapName->Initialize();
	}
	if (m_pMapMesh) {
		m_pMapMesh->Initialize();
	}
	if (m_pMapFNode) {
		m_pMapFNode->Initialize();
	}
	if (m_pMapFLink) {
		m_pMapFLink->Initialize();
	}
	if (m_pMapFExtend) {
		m_pMapFExtend->Initialize();
	}
	if (m_pMapWNode) {
		m_pMapWNode->Initialize();
	}
	if (m_pMapWLink) {
		m_pMapWLink->Initialize();
	}
	if (m_pMapWExtend) {
		m_pMapWExtend->Initialize();
	}
	if (m_pMapVNode) {
		m_pMapVNode->Initialize();
	}
	if (m_pMapVLink) {
		m_pMapVLink->Initialize();
	}
	if (m_pMapVExtend) {
		m_pMapVExtend->Initialize();
	}
	if (m_pMapComplex) {
		m_pMapComplex->Initialize();
	}
	if (m_pMapBuilding) {
		m_pMapBuilding->Initialize();
	}
	if (m_pMapTraffic) {
		m_pMapTraffic->Initialize();
	}
	if (m_pMapCourse) {
		m_pMapCourse->Initialize();
	}	
#if defined(USE_INAVI_STATIC_DATA)
	if (m_pStaticMgr) {
		m_pStaticMgr->Initialize();
	}
#endif
	m_rtBox.Xmin = INT_MAX;
	m_rtBox.Ymin = INT_MAX;
	m_rtBox.Xmax = INT_MIN;
	m_rtBox.Ymax = INT_MIN;

#if defined(USE_DATA_CACHE)
	m_cntMaxCache = MAX_CASH_COUNT;
#endif

	// for optimal api
	memset(&m_dataCost, 0x00, sizeof(m_dataCost));

	m_dataCost.optimal.cost_lv[0] = 100; // 0단계 검색 범위
	m_dataCost.optimal.cost_lv[1] = 300; // 1단계 검색 범위
	m_dataCost.optimal.cost_lv[2] = 500; // 2단계 검색 범위
	m_dataCost.optimal.cost_lv[3] = 700; // 3단계 검색 범위
	m_dataCost.optimal.cost_lv[4] = 1000; // 4단계 검색 범위
	m_dataCost.optimal.cost_mes = 500; // 이웃메쉬 포함 영역
	m_dataCost.optimal.cost_bld = 300; // 이웃빌딩 포함 영역
	m_dataCost.optimal.cost_cpx = 500; // 이웃단지 포함 영역

	return true;
}


void CDataManager::Release(void)
{
	RemoveCacheItem();

	if (m_pMapName) {
		m_pMapName->Release();
	}
	if (m_pMapMesh) {
		m_pMapMesh->Release();
	}
	if (m_pMapFNode) {
		m_pMapFNode->Release();
	}
	if (m_pMapFLink) {
		m_pMapFLink->Release();
	}
	if (m_pMapFExtend) {
		m_pMapFExtend->Release();
	}
	if (m_pMapWNode) {
		m_pMapWNode->Release();
	}
	if (m_pMapWLink) {
		m_pMapWLink->Release();
	}
	if (m_pMapWExtend) {
		m_pMapWExtend->Release();
	}
	if (m_pMapVNode) {
		m_pMapVNode->Release();
	}
	if (m_pMapVLink) {
		m_pMapVLink->Release();
	}
	if (m_pMapVExtend) {
		m_pMapVExtend->Release();
	}
	if (m_pMapComplex) {
		m_pMapComplex->Release();
	}
	if (m_pMapBuilding) {
		m_pMapBuilding->Release();
	}
	if (m_pMapTraffic) {
		m_pMapTraffic->Release();
	}
	if (m_pMapCourse) {
		m_pMapCourse->Release();
	}


	if (!m_mapFileIndex.empty()) {
		unordered_map<uint32_t, FileIndex>().swap(m_mapFileIndex);
		m_mapFileIndex.clear();
	}
}


void CDataManager::SetMeshBox(IN const SBox* pBox)
{
	if (pBox) {
		// extend region
		if (pBox->Xmin < m_rtBox.Xmin) m_rtBox.Xmin = pBox->Xmin;
		if (pBox->Ymin < m_rtBox.Ymin) m_rtBox.Ymin = pBox->Ymin;
		if (pBox->Xmax > m_rtBox.Xmax) m_rtBox.Xmax = pBox->Xmax;
		if (pBox->Ymax > m_rtBox.Ymax) m_rtBox.Ymax = pBox->Ymax;
	}

	m_pMapMesh->SetBox(&m_rtBox);
}


//void CDataManager::SetNeighborMesh(void)
//{
//	m_pMapMesh->CheckNeighborMesh();
//}


void CDataManager::ArrangementMesh(void)
{
	m_pMapMesh->ArrangementMesh();
}


bool CDataManager::AddMeshData(IN const stMeshInfo * pData)
{
	return m_pMapMesh->AddData(pData);

}

//bool CDataManager::AddMeshDataByNode(IN const stMeshInfo * pInfo, IN const stNodeInfo * pData)
//{
//	if (AddMeshData(pInfo)) {
//		return m_pMapMesh->InsertNode(pData->node_id);
//	}
//
//	return false;
//}
//
//
//bool CDataManager::AddMeshDataByLink(IN const stMeshInfo * pInfo, IN const stLinkInfo * pData)
//{
//	if (AddMeshData(pInfo)) {
//		return m_pMapMesh->InsertLink(pData);
//	}
//
//	return false;
//}
//
//
//bool CDataManager::AddMeshDataByBuilding(IN const stMeshInfo * pInfo, IN const stPolygonInfo * pData)
//{
//	if (AddMeshData(pInfo)) {
//		return m_pMapMesh->InsertBuilding(pData);
//	}
//
//	return false;
//}
//
//
//bool CDataManager::AddMeshDataByComplex(IN const stMeshInfo * pInfo, IN const stPolygonInfo * pData)
//{
//	if (AddMeshData(pInfo)) {
//		return m_pMapMesh->InsertComplex(pData);
//	}
//
//	return false;
//}


bool CDataManager::DeleteData(IN const uint64_t id)
{
	bool ret = false;
	stMeshInfo* pMesh = nullptr;

	if (m_pMapMesh != nullptr && ((pMesh = m_pMapMesh->GetMeshById(id)) != nullptr)) {
#if defined(USE_FOREST_DATA)
#ifdef TEST_SPATIALINDEX
		// delete wnodes
		if (!pMesh->setFNodeDuplicateCheck.empty()) {
			for (const auto& key : pMesh->setFNodeDuplicateCheck) {
				m_pMapFNode->DeleteData(key);
			}

			pMesh->setFNodeDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setFNodeDuplicateCheck);
		}

		// delete wlinks
		if (!pMesh->setFLinkDuplicateCheck.empty()) {
			for (const auto& key : pMesh->setFLinkDuplicateCheck) {
				m_pMapFLink->DeleteData(key);
			}

			pMesh->setFLinkDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setFLinkDuplicateCheck);
		}

		// reset
		m_pMapMesh->DeleteSpatialindex(pMesh);
		//m_pMapMesh->CreateSpatialindex(pMesh);
#else
		// delete nodes
		if (!pMesh->nodes.empty()) {
			for (vector<KeyID>::const_iterator it = pMesh->nodes.begin(); it != pMesh->nodes.end(); it++) {
				m_pMapNode->DeleteData(*it);
			}
			pMesh->nodes.clear();
			vector<KeyID>().swap(pMesh->nodes);

			pMesh->setNodeDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setNodeDuplicateCheck);
		}

		// delete links
		if (!pMesh->links.empty()) {
			for (vector<KeyID>::const_iterator it = pMesh->links.begin(); it != pMesh->links.end(); it++) {
				m_pMapLink->DeleteData(*it);
			}
			pMesh->links.clear();
			vector<KeyID>().swap(pMesh->links);

			pMesh->setLinkDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setLinkDuplicateCheck);
		}
#endif
#endif // #if defined(USE_FOREST_DATA)

#if defined(USE_PEDESTRIAN_DATA)
#ifdef TEST_SPATIALINDEX
		// delete wnodes
		if (!pMesh->setWNodeDuplicateCheck.empty()) {
			for (const auto& key : pMesh->setWNodeDuplicateCheck) {
				m_pMapWNode->DeleteData(key);
			}

			pMesh->setWNodeDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setWNodeDuplicateCheck);
		}

		// delete wlinks
		if (!pMesh->setWLinkDuplicateCheck.empty()) {
			for (const auto& key : pMesh->setWLinkDuplicateCheck) {
				m_pMapWLink->DeleteData(key);
			}

			pMesh->setWLinkDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setWLinkDuplicateCheck);
		}

		// reset
		m_pMapMesh->DeleteSpatialindex(pMesh);
		//m_pMapMesh->CreateSpatialindex(pMesh);
#else
		// delete wnodes
		if (!pMesh->wnodes.empty()) {
			for (vector<KeyID>::const_iterator it = pMesh->wnodes.begin(); it != pMesh->wnodes.end(); it++) {
				m_pMapWNode->DeleteData(*it);
			}
			pMesh->wnodes.clear();
			vector<KeyID>().swap(pMesh->wnodes);

			pMesh->setWNodeDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setWNodeDuplicateCheck);
		}

		// delete wlinks
		if (!pMesh->wlinks.empty()) {
			for (vector<KeyID>::const_iterator it = pMesh->wlinks.begin(); it != pMesh->wlinks.end(); it++) {
				m_pMapWLink->DeleteData(*it);
			}
			pMesh->wlinks.clear();
			vector<KeyID>().swap(pMesh->wlinks);

			pMesh->setWLinkDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setWLinkDuplicateCheck);
		}
#endif
#endif
#if defined(USE_VEHICLE_DATA)
#ifdef TEST_SPATIALINDEX
		// delete wnodes
		if (!pMesh->setVNodeDuplicateCheck.empty()) {
			for (const auto& key : pMesh->setVNodeDuplicateCheck) {
				m_pMapVNode->DeleteData(key);
			}

			pMesh->setVNodeDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setVNodeDuplicateCheck);
		}

		// delete wlinks
		if (!pMesh->setVLinkDuplicateCheck.empty()) {
			for (const auto& key : pMesh->setVLinkDuplicateCheck) {
				m_pMapVLink->DeleteData(key);
			}

			pMesh->setVLinkDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setVLinkDuplicateCheck);
		}

		// reset
		m_pMapMesh->DeleteSpatialindex(pMesh);
		//m_pMapMesh->CreateSpatialindex(pMesh);
#else
		// delete wnodes
		if (!pMesh->vnodes.empty()) {
			for (vector<KeyID>::const_iterator it = pMesh->vnodes.begin(); it != pMesh->vnodes.end(); it++) {
				m_pMapVNode->DeleteData(*it);
			}
			pMesh->vnodes.clear();
			vector<KeyID>().swap(pMesh->vnodes);

			pMesh->setVNodeDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setVNodeDuplicateCheck);
		}

		// delete vlinks
		if (!pMesh->vlinks.empty()) {
			for (vector<KeyID>::const_iterator it = pMesh->vlinks.begin(); it != pMesh->vlinks.end(); it++) {
				m_pMapVLink->DeleteData(*it);
			}
			pMesh->vlinks.clear();
			vector<KeyID>().swap(pMesh->vlinks);

			pMesh->setVLinkDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setVLinkDuplicateCheck);
		}
#endif
#endif
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
#ifdef TEST_SPATIALINDEX
		// delete complex
		if (!pMesh->setCpxDuplicateCheck.empty()) {
			for (const auto& key : pMesh->setCpxDuplicateCheck) {
				m_pMapComplex->DeleteData(key);
			}

			pMesh->setCpxDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setCpxDuplicateCheck);
		}

		// delete building
		if (!pMesh->setBldDuplicateCheck.empty()) {
			for (const auto& key : pMesh->setBldDuplicateCheck) {
				m_pMapBuilding->DeleteData(key);
			}

			pMesh->setBldDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setBldDuplicateCheck);
		}

		// reset
		m_pMapMesh->DeleteSpatialindex(pMesh);
		//m_pMapMesh->CreateSpatialindex(pMesh);
#else
		// delete complex
		if (!pMesh->complexs.empty()) {
			for (vector<KeyID>::const_iterator it = pMesh->complexs.begin(); it != pMesh->complexs.end(); it++) {
				m_pMapComplex->DeleteData(*it);
			}
			pMesh->complexs.clear();
			vector<KeyID>().swap(pMesh->complexs);

			pMesh->setCpxDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setCpxDuplicateCheck);
		}

		// delete building
		if (!pMesh->buildings.empty()) {
			for (vector<KeyID>::const_iterator it = pMesh->buildings.begin(); it != pMesh->buildings.end(); it++) {
				m_pMapBuilding->DeleteData(*it);
			}
			pMesh->buildings.clear();
			vector<KeyID>().swap(pMesh->buildings);

			pMesh->setBldDuplicateCheck.clear();
			set<KeyID>().swap(pMesh->setBldDuplicateCheck);
		}
#endif
#endif

		// delete mesh
		ret = m_pMapMesh->DeleteData(id);
	}

	return ret;
}



bool CDataManager::AddNodeExData(IN const stExtendInfo * pData, IN const int32_t dataType)
{
	bool ret = false;

#if defined(USE_FOREST_DATA)
	if (dataType == TYPE_DATA_TREKKING) {
		ret = m_pMapFExtend->AddData(pData, TYPE_KEY_NODE);
	} else if (dataType == TYPE_DATA_PEDESTRIAN) {
		ret = m_pMapWExtend->AddData(pData, TYPE_KEY_NODE);
	} else {
		LOG_TRACE(LOG_ERROR, "-------------- should check dataType, %d", dataType);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	ret = m_pMapWExtend->AddData(pData, TYPE_KEY_NODE);
#elif defined(USE_VEHICLE_DATA)
	ret = m_pMapVExtend->AddData(pData, TYPE_KEY_NODE);
#else
	ret = m_pMapVExtend->AddData(pData, TYPE_KEY_NODE);
#endif

	return ret;
}


bool CDataManager::AddNodeExData(IN const KeyID keyId, IN const int8_t type, IN const double value, IN const int32_t dataType)
{
	bool ret = false;

#if defined(USE_FOREST_DATA)
	if (dataType == TYPE_DATA_TREKKING) {
		ret = m_pMapFExtend->AddData(keyId, type, value, TYPE_KEY_NODE);
	} else if (dataType == TYPE_DATA_PEDESTRIAN) {
		ret = m_pMapWExtend->AddData(keyId, type, value, TYPE_KEY_NODE);
	} else {
		LOG_TRACE(LOG_ERROR, "-------------- should check dataType, %d", dataType);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	ret = m_pMapWExtend->AddData(keyId, type, value, TYPE_KEY_NODE);
#elif defined(USE_VEHICLE_DATA)
	ret = m_pMapVExtend->AddData(keyId, type, value, TYPE_KEY_NODE);
#else
	ret = m_pMapVExtend->AddData(keyId, type, value, TYPE_KEY_NODE);
#endif

	return ret;
}


bool CDataManager::AddLinkExData(IN const stExtendInfo * pData, IN const int32_t dataType)
{
	bool ret = false;

#if defined(USE_FOREST_DATA)
	if (dataType == TYPE_DATA_TREKKING) {
		ret = m_pMapFExtend->AddData(pData, TYPE_KEY_LINK);
	} else if (dataType == TYPE_DATA_PEDESTRIAN) {
		ret = m_pMapWExtend->AddData(pData, TYPE_KEY_LINK);
	} else {
		LOG_TRACE(LOG_ERROR, "-------------- should check dataType, %d", dataType);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	ret = m_pMapWExtend->AddData(pData, TYPE_KEY_LINK);
#elif defined(USE_VEHICLE_DATA)
	ret = m_pMapVExtend->AddData(pData, TYPE_KEY_LINK);
#else
	ret = m_pMapVExtend->AddData(pData, TYPE_KEY_LINK);
#endif

	return ret;
}


bool CDataManager::AddLinkExData(IN const KeyID keyId, IN const int8_t type, IN const double value, IN const int32_t dataType)
{
	bool ret = false;

#if defined(USE_FOREST_DATA)
	if (dataType == TYPE_DATA_TREKKING) {
		ret = m_pMapVExtend->AddData(keyId, type, value, TYPE_KEY_LINK);
	} else if (dataType == TYPE_DATA_PEDESTRIAN) {
		ret = m_pMapVExtend->AddData(keyId, type, value, TYPE_KEY_LINK);
	} else {
		LOG_TRACE(LOG_ERROR, "-------------- should check dataType, %d", dataType);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	ret = m_pMapVExtend->AddData(keyId, type, value, TYPE_KEY_LINK);
#elif defined(USE_VEHICLE_DATA)
	ret = m_pMapVExtend->AddData(keyId, type, value, TYPE_KEY_LINK);
#else
	ret = m_pMapVExtend->AddData(keyId, type, value, TYPE_KEY_LINK);
#endif

	return ret;
}


bool CDataManager::AddFNodeData(IN const stNodeInfo * pData)
{
#if defined(USE_FOREST_DATA)
	if (m_pMapFNode->AddData(pData)) {
		return m_pMapMesh->InsertFNode(pData->node_id);
	}
#endif
	return false;
}

bool CDataManager::AddFLinkData(IN const stLinkInfo * pData)
{
#if defined(USE_FOREST_DATA)
	if (m_pMapFLink->AddData(pData))
	{
		return m_pMapMesh->InsertFLink(pData);
	}
#endif
	return false;
}


bool CDataManager::AddWNodeData(IN const stNodeInfo * pData)
{
#if defined(USE_PEDESTRIAN_DATA)
	if (m_pMapWNode->AddData(pData))
	{
		return m_pMapMesh->InsertWNode(pData->node_id);
	}
#endif
	return false;
}


bool CDataManager::AddWLinkData(IN const stLinkInfo * pData)
{
#if defined(USE_PEDESTRIAN_DATA)
	if (m_pMapWLink->AddData(pData)) 
	{
		return m_pMapMesh->InsertWLink(pData);
	}
#endif
	return false;
}


bool CDataManager::AddVNodeData(IN const stNodeInfo * pData)
{
#if defined(USE_VEHICLE_DATA)
	if (m_pMapVNode->AddData(pData))
	{
		return m_pMapMesh->InsertVNode(pData->node_id);
	}
#endif
	return false;
}


bool CDataManager::AddVLinkData(IN const stLinkInfo * pData)
{
#if defined(USE_VEHICLE_DATA)
	if (m_pMapVLink->AddData(pData))
	{
		return m_pMapMesh->InsertVLink(pData);
	}
#endif
	return false;
}


bool  CDataManager::AddComplexData(IN const stPolygonInfo * pData)
{
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
#if defined(__USE_TEMPLETE)
	m_MapBase.AddData(pData);
#endif

	if (m_pMapComplex->AddData(pData))
	{
		return m_pMapMesh->InsertComplex(pData);
	}
#endif
	return false;
}


bool  CDataManager::AddBuildingData(IN const stPolygonInfo * pData)
{
# if defined(USE_OPTIMAL_POINT_API)
	if (m_pMapBuilding->AddData(pData))
	{
		return m_pMapMesh->InsertBuilding(pData);
	}
#endif
	return false;
}


bool CDataManager::AddEntranceData(IN const KeyID keyId, IN const stEntranceInfo* pData)
{
	bool ret = false;

#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
	if (pData) {
		stPolygonInfo* pPoly = nullptr;
		// 1:빌딩, 2:단지, 3:산바운더리
		if (pData->poly_type == TYPE_POLYGON_BUILDING) {
			pPoly = m_pMapBuilding->GetDataById(keyId);				
		}
		else if ((pData->poly_type == TYPE_POLYGON_COMPLEX) || (pData->poly_type == TYPE_POLYGON_MOUNTAIN)) {
			pPoly = m_pMapComplex->GetDataById(keyId);
		}

		if (pPoly != nullptr) {
			//pPoly->vtEnt.emplace_back(*pData);
			pPoly->setAttribute(TYPE_POLYGON_DATA_ATTR_ENT, pData, 1);
			ret = true;
		}
	}
#endif
	return ret;
}


bool CDataManager::AddNameData(IN const stNameInfo * pData)
{
	return m_pMapName->AddData(pData);
}


// Traffic
uint8_t CDataManager::GetTrafficSpeed(IN const KeyID link, IN const uint8_t dir, IN const uint32_t timestamp, IN OUT uint8_t& type)
{
	uint8_t retType = type;
	uint8_t retSpeed = SPEED_NOT_AVALABLE;

	if ((type == TYPE_TRAFFIC_REAL) || (type == TYPE_TRAFFIC_REAL_STATIC)) {
		retSpeed = m_pMapTraffic->GetSpeed(link, dir, retType); // TYPE_REAL_TTL or TYPE_REAL_KS
	}

	if ((type == TYPE_TRAFFIC_STATIC) || (type == TYPE_TRAFFIC_REAL_STATIC && retSpeed == SPEED_NOT_AVALABLE)) {
		retSpeed = GetTrafficStaticSpeed(link, dir, timestamp, retType); // TYPE_STATIC_TTL or TYPE_STATIC_KS
	}

#if 0 // defined(USE_TMS_API)
	if (type == TYPE_TRAFFIC_NONE && retSpeed == SPEED_NOT_AVALABLE) {
		retSpeed = GetTrafficStaticSpeed(link, dir, timestamp, retType); // TYPE_STATIC_TTL or TYPE_STATIC_KS
	}
#endif

	type = retType;

	return retSpeed;
}


uint64_t CDataManager::GetTrafficId(IN const KeyID link, IN const uint8_t dir, IN const uint8_t type)
{
	return m_pMapTraffic->GetTrafficId(link, dir, type);
}


bool CDataManager::CheckTrafficAlive(uint32_t limit_timestamp)
{
	m_pMapTraffic->CheckAliveKSData(limit_timestamp);

	vector<stTrafficInfoTTL> vtCheckedResetLink;
	m_pMapTraffic->CheckAliveTTLData(limit_timestamp, vtCheckedResetLink);
#if USE_TRAFFIC_LINK_ATTRIBUTE
	if (!vtCheckedResetLink.empty()) {
#pragma omp for
		for (int ii = 0; ii < vtCheckedResetLink.size(); ii++) {
			KeyID keyLink;
			keyLink.tile_id = vtCheckedResetLink[ii].ttl_nid;
			keyLink.dir = vtCheckedResetLink[ii].link_dir;
			keyLink.nid = vtCheckedResetLink[ii].link_dir;
			stLinkInfo* pLink = GetVLinkDataById(keyLink);
			if (pLink != nullptr) {
				if (vtCheckedResetLink[ii].ttl_dir == DIR_POSITIVE) {
					pLink->veh_ext.spd_p = SPEED_NOT_AVALABLE;
					pLink->veh_ext.spd_type_p = TYPE_TRAFFIC_NONE;
				} else if (vtCheckedResetLink[ii].ttl_dir == DIR_NAGATIVE) {
					pLink->veh_ext.spd_n = SPEED_NOT_AVALABLE;
					pLink->veh_ext.spd_type_n = TYPE_TRAFFIC_NONE;
				} else {
					pLink->veh_ext.spd_p = pLink->veh_ext.spd_n = SPEED_NOT_AVALABLE;
					pLink->veh_ext.spd_type_p = pLink->veh_ext.spd_type_n = TYPE_TRAFFIC_NONE;
				}
			}
		}
	}
#endif // #if USE_TRAFFIC_LINK_ATTRIBUTE

	return true;
}


// KS
bool CDataManager::AddTrafficKSData(IN const uint32_t ks_id, IN const uint32_t tile_nid, IN const uint32_t link_nid, IN const uint8_t dir)
{
	return m_pMapTraffic->AddKSData(ks_id, tile_nid, link_nid, dir);
}


bool CDataManager::UpdateTrafficKSData(IN const uint32_t ksId, IN const uint8_t speed, uint32_t timestamp)
{
	return m_pMapTraffic->UpdateKSData(ksId, speed, timestamp);
}


// TTL
bool CDataManager::UpdateTrafficTTLData(IN const uint64_t ttlId, IN const uint8_t speed, uint32_t timestamp)
{
	bool ret = false;
	uint8_t retDir;
	KeyID retLink;
	
	if ((ret = m_pMapTraffic->UpdateTTLData(ttlId, speed, timestamp, retDir, retLink)) == true)
	{
#if USE_TRAFFIC_LINK_ATTRIBUTE
		KeyID keyLink = retLink;
		stLinkInfo* pLink = GetVLinkDataById(keyLink);
		if (pLink != nullptr) {
			if (retDir == DIR_POSITIVE) {
				pLink->veh_ext.spd_p = speed;
				pLink->veh_ext.spd_type_p = TYPE_SPEED_STATIC_TTL;
			} else if (retDir == DIR_NAGATIVE) {
				pLink->veh_ext.spd_n = speed;
				pLink->veh_ext.spd_type_n = TYPE_SPEED_STATIC_TTL;
			} else {
				pLink->veh_ext.spd_p = pLink->veh_ext.spd_n = speed;
				pLink->veh_ext.spd_type_p = pLink->veh_ext.spd_type_n = TYPE_SPEED_STATIC_TTL;
			}			
		}
#endif
	}

	return ret;
}


bool CDataManager::AddTrafficTTLData(IN const uint32_t ttl_nid, IN const uint8_t ttl_dir, IN const uint32_t tile_nid, IN const uint32_t link_nid, IN const uint8_t link_dir)
{
	return m_pMapTraffic->AddTTLData(ttl_nid, ttl_dir, tile_nid, link_nid, link_dir);
}


// STATIC
uint8_t CDataManager::GetTrafficStaticSpeed(IN const KeyID link, IN const uint8_t dir, IN const uint32_t timestamp, IN OUT uint8_t& type)
{
	uint8_t retType = TYPE_SPEED_NONE;
	uint8_t retSpeed = SPEED_NOT_AVALABLE;

#if defined(USE_INAVI_STATIC_DATA)
	if (type == TYPE_TRAFFIC_REAL) {
		retType = TYPE_SPEED_REAL_TTL;
	} else {
		retType = TYPE_SPEED_STATIC_TTL;
	}

	uint64_t ttlId = m_pMapTraffic->GetTrafficId(link, dir, retType);
	if (ttlId > 0 && m_pStaticMgr != nullptr) {
		float length = 0.f;

		stLinkInfo* pLink = GetVLinkDataById(link);
		if (pLink) {
			length = pLink->length;
		}

		time_t tmNow = timestamp;
		if (tmNow == 0) {
			tmNow = time(NULL);
		}

		retSpeed = m_pStaticMgr->GetSpd(ttlId, tmNow, length);
		type = retType;
	}
#endif

	return retSpeed;
}


bool CDataManager::GetTrafficStaticSpeedBlock(IN const uint32_t timestamp, OUT std::unordered_map<uint64_t, uint8_t>& umapBlock)
{
	bool ret = false;

	return ret = m_pStaticMgr->GetStaticSpeedBlock(timestamp, umapBlock);

	if (!umapBlock.empty()) {
		ret = true;
	}

	return ret;
}


bool CDataManager::AddCourseDataByLink(IN const uint64_t linkId, IN const uint32_t courseId)
{
	return m_pMapCourse->AddCourseDataByLink(linkId, courseId);
}


bool CDataManager::AddLinkDataByCourse(IN const uint32_t courseId, IN const uint64_t linkId)
{
	return m_pMapCourse->AddLinkDataByCourse(courseId, linkId);
}


stNodeInfo* CDataManager::GetNodeDataById(IN const KeyID keyId, IN const int32_t type, IN const bool force)
{
	stNodeInfo* pNode = nullptr;

#if defined(USE_FOREST_DATA)
	if (type == TYPE_NODE_DATA_NONE) {
		pNode = GetFNodeDataById(keyId, force);
		if (pNode == nullptr) {
			pNode = GetWNodeDataById(keyId, force);
		}
	} else if (type == TYPE_NODE_DATA_PEDESTRIAN) {
		pNode = GetWNodeDataById(keyId, force);
	} else {
		pNode = GetFNodeDataById(keyId, force);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	pNode = GetWNodeDataById(keyId, force);
#elif defined(USE_VEHICLE_DATA)
	pNode = GetVNodeDataById(keyId, force);
#else
	pNode = GetFNodeDataById(keyId, force);
#endif

	return pNode;
}


stLinkInfo* CDataManager::GetLinkDataById(IN const KeyID keyId, IN const int32_t type, IN const bool force)
{
	stLinkInfo* pLink = nullptr;

#if defined(USE_FOREST_DATA)
	if (type == TYPE_LINK_DATA_NONE) {
		pLink = GetFLinkDataById(keyId, force);
		if (pLink == nullptr) {
			pLink = GetWLinkDataById(keyId, force);
		}
	} else if (type == TYPE_LINK_DATA_PEDESTRIAN) {
		pLink = GetWLinkDataById(keyId, force);
	} else {
		pLink = GetFLinkDataById(keyId, force);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	pLink = GetWLinkDataById(keyId, force);
#elif defined(USE_VEHICLE_DATA)
	pLink = GetVLinkDataById(keyId, force);
#else
	pLink = GetFLinkDataById(keyId, force);
#endif

	return pLink;
}


stMeshInfo * CDataManager::GetMeshDataById(IN const uint32_t id, IN const bool force)
{
	stMeshInfo* pData = nullptr;

	if (m_pMapMesh && !(pData = m_pMapMesh->GetMeshById(id)) && force)
	{
		if (GetNewData(id)) {
			pData = m_pMapMesh->GetMeshById(id);
		}
	}

	return pData;
}


stNodeInfo * CDataManager::GetFNodeDataById(IN const KeyID keyId, IN const bool force)
{
	stNodeInfo* pData = nullptr;

	if (m_pMapFNode && !(pData = m_pMapFNode->GetNodeById(keyId)) && force)
	{
		if (GetNewData(keyId.tile_id)) {
			pData = m_pMapFNode->GetNodeById(keyId);
		}
	}

	return pData;
}


stLinkInfo * CDataManager::GetFLinkDataById(IN const KeyID keyId, IN const bool force)
{
	stLinkInfo* pData = nullptr;

	if (m_pMapFLink && !(pData = m_pMapFLink->GetLinkById(keyId)) && force)
	{
		if (GetNewData(keyId.tile_id)) {
			pData = m_pMapFLink->GetLinkById(keyId);
		}
	}

	return pData;
}


stNodeInfo * CDataManager::GetWNodeDataById(IN const KeyID keyId, IN const bool force)
{
	stNodeInfo* pData = nullptr;

#if defined(USE_PEDESTRIAN_DATA)
	if (m_pMapWNode && !(pData = m_pMapWNode->GetNodeById(keyId)) && force)
	{
		if (GetNewData(keyId.tile_id)) {
			pData = m_pMapWNode->GetNodeById(keyId);
		}
	}
#endif

	return pData;
}


stLinkInfo * CDataManager::GetWLinkDataById(IN const KeyID keyId, IN const bool force)
{
	stLinkInfo* pData = nullptr;

#if defined(USE_PEDESTRIAN_DATA)
	if (m_pMapWLink && !(pData = m_pMapWLink->GetLinkById(keyId)) && force)
	{
		if (GetNewData(keyId.tile_id)) {
			pData = m_pMapWLink->GetLinkById(keyId);
		}
	}
#endif

	return pData;
}


stNodeInfo * CDataManager::GetVNodeDataById(IN const KeyID keyId, IN const bool force)
{
	stNodeInfo* pData = nullptr;

#if defined(USE_VEHICLE_DATA)
	if (m_pMapVNode && !(pData = m_pMapVNode->GetNodeById(keyId)) && force)
	{
		if (GetNewData(keyId.tile_id)) {
			pData = m_pMapVNode->GetNodeById(keyId);
		}
	}
#endif

	return pData;
}

stLinkInfo * CDataManager::GetVLinkDataById(IN const KeyID keyId, IN const bool force)
{
	stLinkInfo* pData = nullptr;

#if defined(USE_VEHICLE_DATA)
	if (m_pMapVLink && !(pData = m_pMapVLink->GetLinkById(keyId)) && force)
	{
		if (GetNewData(keyId.tile_id)) {
			pData = m_pMapVLink->GetLinkById(keyId);
		}
	}
#endif

	return pData;
}


stPolygonInfo* CDataManager::GetBuildingDataById(IN const KeyID keyId, IN const bool force)
{
	stPolygonInfo* pData = nullptr;

	if (m_pMapBuilding && !(pData = m_pMapBuilding->GetDataById(keyId)) && force)
	{
		if (GetNewData(keyId.tile_id)) {
			pData = m_pMapBuilding->GetDataById(keyId);
		}
	}

	return pData;
}
	

stPolygonInfo* CDataManager::GetComplexDataById(IN const KeyID keyId, IN const bool force)
{
	stPolygonInfo* pData = nullptr;

	if (m_pMapComplex && !(pData = m_pMapComplex->GetDataById(keyId)) && force)
	{
		if (GetNewData(keyId.tile_id)) {
			pData = m_pMapComplex->GetDataById(keyId);
		}
	}

	return pData;
}


const char* CDataManager::GetNameDataByIdx(IN const uint32_t idx, IN const bool force)
{
	if (!m_pMapName) {
		return nullptr;
	}

	return m_pMapName->GetNameData(idx);
}


const unordered_map<uint32_t, stTrafficInfoKS*>* CDataManager::GetTrafficKSMapData(void) const
{
	if (!m_pMapTraffic) {
		return nullptr; 
	}

	return m_pMapTraffic->GetTrafficKSMapData();
}


const unordered_map<uint32_t, stTrafficMesh*>* CDataManager::GetTrafficMeshData(void)
{
	if (!m_pMapTraffic) {
		return nullptr;
	}

	return m_pMapTraffic->GetTrafficMeshData();
}


MapExtend* CDataManager::GetMapExtend(void) const
{
#if defined(USE_PEDESTRIAN_DATA)
	return m_pMapWExtend;
#elif defined(USE_VEHICLE_DATA)
	return m_pMapVExtend;
#else
	return nullptr;
#endif
}


stExtendInfo * CDataManager::GetExtendInfoById(IN const KeyID keyId, IN const int32_t keyType)
{
	stExtendInfo* pData = nullptr;

#if defined(USE_PEDESTRIAN_DATA)
	if (m_pMapWExtend) {
		pData = m_pMapWExtend->GetExtendById(keyId, keyType);
	}
#elif defined(USE_VEHICLE_DATA)
	if (m_pMapVExtend) {
		pData = m_pMapVExtend->GetExtendById(keyId, keyType);
	}
#endif

	return pData;
}


double CDataManager::GetExtendDataById(IN const KeyID keyId, IN const int8_t type, IN const int32_t dataType)
{
	double ret = 0.f;

#if defined(USE_PEDESTRIAN_DATA)
	if (m_pMapWExtend) {
		ret = m_pMapWExtend->GetExtendDataById(keyId, type, dataType);
	}
#elif defined(USE_VEHICLE_DATA)
	if (m_pMapVExtend) {
		ret = m_pMapVExtend->GetExtendDataById(keyId, type, dataType);
	}
#endif

	return ret;
}



set<uint32_t>* CDataManager::GetCourseByLink(IN const uint64_t linkId)
{
	return m_pMapCourse->GetCourseByLink(linkId);
}


set<uint64_t>* CDataManager::GetLinkByCourse(IN const uint32_t courseId)
{
	return m_pMapCourse->GetLinkByCourse(courseId);
}


const int32_t CDataManager::GetVLinkDataCount(void)
{
	if (!m_pMapVLink) {
		return 0;
	}

	return m_pMapVLink->GetCount();
}


const int32_t CDataManager::GetComplexDataCount(void)
{
	if (!m_pMapComplex) {
		return 0;
	}

	return m_pMapComplex->GetCount();
}


const int32_t CDataManager::GetBuildingDataCount(void)
{
	if (!m_pMapBuilding) {
		return 0;
	}

	return m_pMapBuilding->GetCount();
}


// mesh
uint32_t CDataManager::GetMeshCount(void)
{
	if (!m_pMapMesh)
		return 0;
	
	return m_pMapMesh->GetCount();
}


stMeshInfo * CDataManager::GetMeshData(IN const uint32_t idx)
{
	if (!m_pMapMesh)
		return nullptr;

	return m_pMapMesh->GetMeshData(idx);
}


stMeshInfo * CDataManager::GetMeshDataByPoint(IN const double lng, IN const double lat)
{
	stMeshInfo* pMesh = nullptr;

	if (m_pMapMesh && ((pMesh = m_pMapMesh->GetMeshByPoint(lng, lat)) == nullptr))
	{
		GetNewData(lng, lat);

		pMesh = m_pMapMesh->GetMeshByPoint(lng, lat);
	}

	return pMesh;
}


// 단지 화면에 일정 영역을 그리기 위한 함수로 성능 최적화는 포기하고 사용 중
uint32_t CDataManager::GetMeshDataByRegion(IN const SBox& pRegion)
{
	return GetNewData(pRegion);
}


uint32_t CDataManager::GetMeshDataByPoint(IN const double lng, IN const double lat, IN const uint32_t cntMaxBuff, OUT stMeshInfo** pMeshInfo)
{
	int cntBuff = cntMaxBuff;

	if (m_pMapMesh && !(cntBuff = m_pMapMesh->GetPitInMesh(lng, lat, cntMaxBuff, pMeshInfo)))
	{
		if (GetNewData(lng, lat)) {
			cntBuff = m_pMapMesh->GetPitInMesh(lng, lat, cntMaxBuff, pMeshInfo);
		}
	}

	return cntBuff;
}


uint32_t CDataManager::GetMeshDataByRegion(IN const SBox& pRegion, IN const uint32_t cntMaxBuff, OUT stMeshInfo** pMeshInfo)
{
#if 1 // new
	return m_pMapMesh->GetPitInRegion(pRegion, cntMaxBuff, pMeshInfo);
#else
	uint32_t cntPitIn = 0;
	uint32_t cntMesh = GetMeshCount();

	if (cntMaxBuff <= 0 || pMeshInfo == nullptr) {
		LOG_TRACE(LOG_ERROR, "param value null");
		return 0;
	}
	
	for (uint32_t ii = 0; ii < cntMesh; ii++)
	{
		if (isInPitBox(GetMeshData(ii)->data_box, pRegion)) {
			pMeshInfo[cntPitIn++] = GetMeshData(ii);
		}

		if (cntPitIn >= cntMaxBuff) {
			break;
		}
	}

	return cntPitIn;
#endif
}


const SBox* CDataManager::GetMeshRegion(IN const int32_t idx) const
{
	if (idx < 0)
	{
		// total area
		return &m_rtBox;
	}
	else
	{
		return &m_pMapMesh->GetMeshById(idx)->data_box;
	}
}


const size_t CDataManager::GetJunctionData(IN const KeyID beforeLinkId, IN const KeyID nextLinkId, IN const uint32_t cntMaxBuff, OUT stLinkInfo** pLinkInfo)
{
	size_t cntBuff = 0;

	// get befor link
	stLinkInfo* pLinkBefore = GetFLinkDataById(beforeLinkId);
	stLinkInfo* pLinkNext = GetFLinkDataById(nextLinkId);

	stNodeInfo* pNode = nullptr;

	// get junction node
	// <--- --->, <--- <---
	if (((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(0)) && 
		(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(0))) ||
		((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) && 
		(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1)))) {
		pNode = GetFNodeDataById(pLinkBefore->snode_id);
	}
	// ---> <--- , ---> --->
	else if (((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) && 
			 (pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1))) ||
			 ((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(0)) && 
			 (pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(0)))) {
		pNode = GetFNodeDataById(pLinkBefore->enode_id);	
	}

	if (pNode == nullptr) {
		LOG_TRACE(LOG_WARNING, "failed, can't find junction node info, fore link:%d, next link:%d", beforeLinkId.nid, nextLinkId.nid);
		return cntBuff;
	} else if (pNode->base.connnode_count <= 2) {
		return cntBuff;
	}

	for (int ii=0; ii<pNode->base.connnode_count; ii++) {
		// 경로에 사용된 링크는 제외
		if (pNode->connnodes[ii] == pLinkBefore->link_id || pNode->connnodes[ii] == pLinkNext->link_id) {
			continue;
		}

		pLinkInfo[cntBuff++] = GetFLinkDataById(pNode->connnodes[ii]);
	}

	return cntBuff;
}


void CDataManager::SetVersion(IN const uint32_t nDataType, IN const uint32_t nMajor, IN const uint32_t nMinor, IN const uint32_t nPatch, IN const uint32_t nBuild)
{
	MapBase* pBase = nullptr;

	switch(nDataType) {
	case 	TYPE_DATA_NAME: // 명칭사전
	if (m_pMapName != nullptr) {
		pBase = m_pMapName;
	}
	break;
	case 	TYPE_DATA_MESH: // 메쉬
	if (m_pMapMesh != nullptr) {
		pBase = m_pMapMesh;
	}
	break;
	case 	TYPE_DATA_TREKKING: // 숲길
	if (m_pMapFLink != nullptr) {
		pBase = m_pMapFLink;
	}
	break;
	case 	TYPE_DATA_PEDESTRIAN: // 보행자/자전거
	if (m_pMapWLink != nullptr) {
		pBase = m_pMapWLink;
	}
	break;
	case 	TYPE_DATA_VEHICLE: // 자동차
	if (m_pMapVLink != nullptr) {
		pBase = m_pMapVLink;
	}
	break;
	case 	TYPE_DATA_BUILDING: // 건물
	if (m_pMapBuilding != nullptr) {
		pBase = m_pMapBuilding;
	}
	break;
	case 	TYPE_DATA_COMPLEX: // 단지
	if (m_pMapComplex != nullptr) {
		pBase = m_pMapComplex;
	}
	break;

	default:
	break;
	} // switch

	if (pBase != nullptr) {
		pBase->SetVersion(nMajor, nMinor, nPatch, nBuild);
		// LOG_TRACE(LOG_DEBUG, "set version, type:%d, version:%s", nDataType, pBase->GetVersionString());
	}
}


const char* CDataManager::GetVersionString(IN const uint32_t nDataType)
{
	const char* pVersion = nullptr;
	MapBase* pBase = nullptr;

	switch(nDataType) {
	case 	TYPE_DATA_NAME: // 명칭사전
	if (m_pMapName != nullptr) {
		pBase = m_pMapName;
	}
	break;
	case 	TYPE_DATA_MESH: // 메쉬
	if (m_pMapMesh != nullptr) {
		pBase = m_pMapMesh;
	}
	break;
	case 	TYPE_DATA_TREKKING: // 숲길
	if (m_pMapFLink != nullptr) {
		pBase = m_pMapFLink;
	}
	break;
	case 	TYPE_DATA_PEDESTRIAN: // 보행자/자전거
	if (m_pMapWLink != nullptr) {
		pBase = m_pMapWLink;
	}
	break;
	case 	TYPE_DATA_VEHICLE: // 자동차
	if (m_pMapVLink != nullptr) {
		pBase = m_pMapVLink;
	}
	break;
	case 	TYPE_DATA_BUILDING: // 건물
	if (m_pMapBuilding != nullptr) {
		pBase = m_pMapBuilding;
	}
	break;
	case 	TYPE_DATA_COMPLEX: // 단지
	if (m_pMapComplex != nullptr) {
		pBase = m_pMapComplex;
	}
	break;

	default: // 엔진
		char szVersion[256] = {0,};
		if (ENGINE_VERSION_BUILD > 0) {
			snprintf(szVersion, sizeof(szVersion), "%d.%d.%d.%d", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH, ENGINE_VERSION_BUILD);
		} else {
			snprintf(szVersion, sizeof(szVersion), "%d.%d.%d", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH);
		}
		m_strMsg = szVersion;
		pVersion = m_strMsg.c_str();
	break;
	} // switch

	if (pBase != nullptr) {
		pVersion = pBase->GetVersionString();
		// LOG_TRACE(LOG_DEBUG, "get version, type:%d, version:%s", nDataType, pBase->GetVersionString());
	}
	
	return pVersion;
}


stLinkInfo * CDataManager::GetLinkDataByPointAround(IN const double lng, IN const double lat, IN const int32_t nMaxDist, OUT double& retLng, OUT double& retLat, OUT double& retDist, IN const int32_t nMatchType, IN const int32_t nLinkDataType, IN const int32_t nLinkLimitLevel, IN const TruckOption* pTruckOption, OUT int32_t* pMatchVtxIdx)
{
	stMeshInfo* pMesh = nullptr;
	stLinkInfo* retLink = nullptr;

#if defined(USE_FOREST_DATA)
	stLinkInfo* minLinkTrekking = nullptr;
	double minDistTrekking = INT_MAX;
	int32_t minMatchVtxIndexTrekking = -1;
	SPoint minCoordTrekking = { 0, };
#endif
#if defined(USE_PEDESTRIAN_DATA)
	stLinkInfo* minLinkPedestrian = nullptr;
	double minDistPedestrian = INT_MAX;
	int32_t minMatchVtxIndexPedestrian = -1;
	SPoint minCoordPedestrian = { 0, };
#endif

#if defined(USE_FOREST_DATA)
	static const uint32_t s_maxMesh = 10; // 숲길 네트워크는 숲길, 보행자길 두가지의 메쉬가 함께 로딩된다.
#else
	static const uint32_t s_maxMesh = 9;
#endif
	stMeshInfo* s_ppMesh[s_maxMesh];
	uint32_t nMaxMesh = 0;


#if defined(USE_DATA_CACHE)
#	if defined(USE_FOREST_DATA)
	nMaxMesh = GetMeshDataByPoint(lng, lat, s_maxMesh, s_ppMesh);
#	else
	if (!m_mapFileIndex.empty() && (pMesh = GetMeshDataByPoint(lng, lat)) != nullptr) {
		s_ppMesh[nMaxMesh++] = pMesh;
	} else if (m_mapFileIndex.empty() && GetMeshCount() > 0) {
		nMaxMesh = GetMeshDataByPoint(lng, lat, s_maxMesh, s_ppMesh);
	} 
#	endif
#else
	if (m_mapFileIndex.empty() && GetMeshCount() > 0) {
		// 파일 캐쉬가 아닌 데이터 버퍼 로딩 상태일 경우
		nMaxMesh = GetMeshDataByPoint(lng, lat, s_maxMesh, s_ppMesh);
	}
#endif

	// 주변 메쉬를 추가
#if defined(USE_FOREST_DATA)
	int cntNewMesh = nMaxMesh;
	for (int32_t ii = 0; ii < nMaxMesh; ii++) { // 숲길 네트워크는 숲길, 보행자길 두가지의 메쉬가 함께 로딩된다.
		// 숲길 단일 메쉬의 검색 속도 향상을 위해 다중  메쉬 사용, 단일 메쉬는 검색에서 제외, 2024-05-29
		//if (s_ppMesh[ii]->mesh_id.tile_id == g_forestMeshId) {
		//	continue;
		//}

		for (unordered_set<uint32_t>::const_iterator it = s_ppMesh[ii]->neighbors.begin(); it != s_ppMesh[ii]->neighbors.end(); it++) {
			// 주변 메쉬의 중심 거리가 최대 요청 거리 이하일 경우만 추가
			if ((pMesh = GetMeshDataById(*it)) != nullptr && isInBox(lng, lat, pMesh->data_box, nMaxDist) && (cntNewMesh < s_maxMesh)) {
				s_ppMesh[cntNewMesh++] = pMesh; // 주변 메쉬 추가
			}
		} // for
	} // for
	nMaxMesh = cntNewMesh; // 최대 검색 메쉬 수 변경
#else
	if (nMaxMesh == 1 && !s_ppMesh[0]->neighbors.empty()) {
		for (unordered_set<uint32_t>::const_iterator it = s_ppMesh[0]->neighbors.begin(); it != s_ppMesh[0]->neighbors.end(); it++) {
			// 주변 메쉬의 중심 거리가 최대 요청 거리 이하일 경우만 추가
			if ((pMesh = GetMeshDataById(*it)) != nullptr && isInBox(lng, lat, pMesh->data_box, nMaxDist)) {
				s_ppMesh[nMaxMesh++] = pMesh;

				//const int32_t nMxMeshDiff = 500; // 설정 거리안의 주변 메쉬만 확인
				//SBox arrSquare[4] = {
				//	// 좌변
				//	{ pMesh->data_box.Xmin, pMesh->data_box.Ymin, pMesh->data_box.Xmin, pMesh->data_box.Ymax },
				//	// 우변
				//	{ pMesh->data_box.Xmax, pMesh->data_box.Ymin, pMesh->data_box.Xmax, pMesh->data_box.Ymax },
				//	// 상단
				//	{ pMesh->data_box.Xmin, pMesh->data_box.Ymax, pMesh->data_box.Xmax, pMesh->data_box.Ymax },
				//	// 하단
				//	{ pMesh->data_box.Xmin, pMesh->data_box.Ymin, pMesh->data_box.Xmax, pMesh->data_box.Ymin }
				//};
				//double minLng, minLat, minIr;
				//// 네변 중 한번이라도 요청 거리 이내면 포함
				//for (int32_t ii = 0; ii < 4; ii++) {
				//	getClosestPoint(arrSquare[ii].Xmin, arrSquare[ii].Ymin, pMesh->data_box.Xmax, pMesh->data_box.Ymax, lng, lat, &minLng, &minLat, &minIr);
				//	if ((0 < minIr && minIr < IR_PRECISION) && (getRealWorldDistance(lng, lat, minLng, minLat) <= nMxMeshDiff)) {
				//		s_ppMesh[nMaxMesh++] = pMesh;
				//		break;
				//	}
				//} // for
			}
		} // for
	}
#endif // #	if defined(USE_FOREST_DATA)

#if defined(USE_MULTIPROCESS)
	volatile bool flag = false;
#endif

	int LimitLevel = nLinkLimitLevel;
	if (LimitLevel <= -1) {
		if (nMatchType == TYPE_LINK_MATCH_FOR_TABLE) {
			LimitLevel = USE_ROUTE_TABLE_LEVEL;
		} else {
			LimitLevel = 9;
		}
	}

//#pragma omp parallel for
	for (int32_t ii = 0; ii < nMaxMesh; ii++)
	{
#if defined(USE_MULTIPROCESS)
		if (flag) continue;
#endif

		stMeshInfo* pMesh = s_ppMesh[ii];
		if (pMesh == nullptr) {
			continue;
		}

		stLinkInfo* pLink = nullptr;
		double minLng = 0.f;
		double minLat = 0.f;
		double minDist = nMaxDist;
		double minIr = 0.f;
		int minVtxIdx = -1;

#if defined(USE_FOREST_DATA)
		// 숲길 단일 메쉬의 검색 속도 향상을 위해 다중  메쉬 사용, 단일 메쉬는 검색에서 제외, 2024-05-29
		if (pMesh->mesh_id.tile_id == g_forestMeshId) {
			continue;
		}

#ifdef TEST_SPATIALINDEX
		if ((pMesh->flinkTree != nullptr) && ((nLinkDataType == TYPE_LINK_DATA_NONE) || (nLinkDataType == TYPE_LINK_DATA_TREKKING))) {
			vector<KeyID> vtLinkId;
#if defined(USE_MULTIPROCESS)
#pragma omp critical
#endif
			m_pMapMesh->GetNearLinksfromSpatialindex(pMesh, lng, lat, nMaxDist, vtLinkId);
			for (int ii = 0; ii < vtLinkId.size(); ii++) {
				if (!(pLink = GetFLinkDataById(vtLinkId[ii])) ||
#else // #ifdef TEST_SPATIALINDEX
		if (!pMesh->links.empty() && ((nLinkDataType == TYPE_LINK_DATA_NONE) || (nLinkDataType == TYPE_LINK_DATA_TREKKING)))
		{
#if defined(USE_MULTIPROCESS)
#pragma omp parallel firstprivate(pLink, minLng, minLat, minDist, minVtxIdx, minIr)
#pragma omp for
#endif
			for (int ii = 0; ii < pMesh->links.size(); ii++) {
				if (!(pLink = GetLinkDataById(pMesh->links[ii], nLinkDataType)) ||
#endif
					((nLinkDataType != TYPE_LINK_DATA_NONE) && (nLinkDataType != pLink->base.link_type)) ||
					((minVtxIdx = linkProjection(pLink, lng, lat, nMaxDist, minLng, minLat, minDist, minIr)) < 0)) {
					continue;
				}

				// 매칭하자
				// 등산은 등산로에만
				// 걷기 자전거는 등산 제외하고
				if ((pLink->base.link_type == TYPE_LINK_DATA_TREKKING) && (nMatchType != TYPE_TRE_NONE)) {
					if (((nMatchType == TYPE_TRE_HIKING && pLink->trk.course_type != TYPE_TRE_HIKING)) || 
						((nMatchType != TYPE_TRE_HIKING && pLink->trk.course_type == TYPE_TRE_HIKING)))	{

						// 중용 코스 데이터도 확인
						bool isMatch = false;
						set<uint32_t>* psetCourse = GetCourseByLink(pLink->link_id.llid);
						if (psetCourse != nullptr && !psetCourse->empty()) {
							stCourseInfo courseInfo;
							for (const auto& course : *psetCourse) {
								courseInfo.course_value = course;
								// 중용코스가 매칭 타입에 속하면 통과
								if (nMatchType == courseInfo.course_type) {
									isMatch = true;
									break;
								}
							} // for
						}

						if (!isMatch) {
							continue;
						}
					}
				}
#if defined(USE_MULTIPROCESS)
#pragma omp critical
#endif
				if (minDist < minDistTrekking) {
					minLinkTrekking = pLink;
					minCoordTrekking.x = minLng;
					minCoordTrekking.y = minLat;
					minDistTrekking = minDist;
					minMatchVtxIndexTrekking = minVtxIdx;
				}
			} // for
		}
#endif

#if defined(USE_PEDESTRIAN_DATA)
#ifdef TEST_SPATIALINDEX
		if ((pMesh->wlinkTree != nullptr) && ((nLinkDataType == TYPE_LINK_DATA_NONE) || (nLinkDataType == TYPE_LINK_DATA_PEDESTRIAN))) {
			vector<KeyID> vtLinkId;
#if defined(USE_MULTIPROCESS)
#pragma omp critical
#endif
			m_pMapMesh->GetNearLinksfromSpatialindex(pMesh, lng, lat, nMaxDist, vtLinkId);
			for (int ii = 0; ii < vtLinkId.size(); ii++) {
				if (!(pLink = GetWLinkDataById(vtLinkId[ii])) ||
#else // #ifdef TEST_SPATIALINDEX
		if (!pMesh->wlinks.empty() && ((nLinkDataType == TYPE_LINK_DATA_NONE) || (nLinkDataType == TYPE_LINK_DATA_PEDESTRIAN))) {
#if defined(USE_MULTIPROCESS)
#pragma omp parallel firstprivate(pLink, minLng, minLat, minDist, minVtxIdx, minIr)
#pragma omp for
#endif
			for (int ii = 0; ii < pMesh->wlinks.size(); ii++) {
				if (!(pLink = GetWLinkDataById(pMesh->wlinks[ii])) ||
#endif // #ifdef TEST_SPATIALINDEX
					((nLinkDataType != TYPE_LINK_DATA_NONE) && (nLinkDataType != pLink->base.link_type)) ||
					((nMatchType == TYPE_LINK_MATCH_FOR_BICYCLE) && (
						(pLink->ped.bicycle_control == 3) ||
						(pLink->ped.gate_type == TYPE_GATE_STAIRS) ||
						(pLink->ped.gate_type == TYPE_GATE_ESCALATOR) ||
						(pLink->ped.gate_type == TYPE_GATE_STAIRS_ESCALATOR) ||
						(pLink->ped.gate_type == TYPE_GATE_ESCALATOR) ||
						(pLink->ped.gate_type == TYPE_GATE_ELEVATOR) ||
						(pLink->ped.gate_type == TYPE_GATE_MOVINGWALK) ||
						(pLink->ped.gate_type == TYPE_GATE_STEPPINGSTONES) ||
						(pLink->ped.facility_type == TYPE_OBJECT_UNDERPASS) ||
						(pLink->ped.facility_type == TYPE_OBJECT_FOOTBRIDGE) ||
						(pLink->ped.facility_type == TYPE_OBJECT_SUBWAY) ||
						(pLink->ped.facility_type == TYPE_OBJECT_RAILROAD) ||
						(pLink->ped.facility_type == TYPE_OBJECT_UNDERGROUNDMALL) ||
						(pLink->ped.facility_type == TYPE_OBJECT_THROUGHBUILDING))) ||
					((nMatchType != TYPE_LINK_MATCH_NONE) && (
						(pLink->ped.facility_type == TYPE_OBJECT_UNDERPASS) ||
						(pLink->ped.facility_type == TYPE_OBJECT_SUBWAY) ||
						(pLink->ped.facility_type == TYPE_OBJECT_RAILROAD) ||
						(pLink->ped.facility_type == TYPE_OBJECT_UNDERGROUNDMALL) ||
						(pLink->ped.facility_type == TYPE_OBJECT_THROUGHBUILDING))) ||
					((minVtxIdx = linkProjection(pLink, lng, lat, nMaxDist, minLng, minLat, minDist, minIr)) < 0)) {
					continue;
				}
#if defined(USE_MULTIPROCESS)
#pragma omp critical
#endif
				if (minDist < minDistPedestrian) {
					minLinkPedestrian = pLink;
					minCoordPedestrian.x = minLng;
					minCoordPedestrian.y = minLat;
					minDistPedestrian = minDist;
					minMatchVtxIndexPedestrian = minVtxIdx;
				}
			} // for
		}
#endif

#if defined(USE_VEHICLE_DATA)
		if (retLink != nullptr && retDist < 10) {
			// 현재 메쉬에서 10m이내 탐색 된 결과면 현재 메쉬에서 종료
#if defined(USE_MULTIPROCESS)
			flag = true;
			continue;
#else 
			break;
#endif
		}
#ifdef TEST_SPATIALINDEX
		if ((pMesh->vlinkTree != nullptr) && ((nLinkDataType == TYPE_LINK_DATA_NONE) || (nLinkDataType == TYPE_LINK_DATA_VEHICLE))) {
			vector<KeyID> vtLinkId;
#if defined(USE_MULTIPROCESS)
#pragma omp critical
#endif
			m_pMapMesh->GetNearLinksfromSpatialindex(pMesh, lng, lat, nMaxDist, vtLinkId);
			for (int ii = 0; ii < vtLinkId.size(); ii++) {
				if (!(pLink = GetVLinkDataById(vtLinkId[ii])) ||
#else // #ifdef TEST_SPATIALINDEX
		else if (/*retLink == nullptr && */!pMesh->vlinks.empty())
		{
#if defined(USE_MULTIPROCESS)
#pragma omp parallel firstprivate(pLink, minLng, minLat, minDist, minVtxIdx, minIr)
#pragma omp for
#endif
			for (int ii = 0; ii < pMesh->vlinks.size(); ii++) {
				if (!(pLink = GetVLinkDataById(pMesh->vlinks[ii])) ||
#endif // #ifdef TEST_SPATIALINDEX
					(
					// link_dtype : 2; // 링크세부종별(dk3), 0:미정의, 1:고가도로,지하차도 옆길, 2:비포장도로, 3:단지내도로
					// level : 4; // 경로레벨, 0:고속도로, 1:도시고속도로, 자동차전용 국도/지방도, 2:국도, 3:지방도/일반도로8차선이상, 4:일반도로6차선이상, 5:일반도로4차선이상, 6:일반도로2차선이상, 7:일반도로1차선이상, 8:SS도로, 9:GSS도로/단지내도로/통행금지도로/비포장도로
					// road_type: 4; // 도로종별, 1:고속도록, 2:도시고속도로, 3:일반국도, 4:페리항로, 5:지방도, 6:일반도로, 7:소로, 8:골목길, 9:시장길
					// pass_code : 4;// 규제코드, 1:통행가능, 2:통행불가, 3:공사구간, 4:공사계획구간, 5:일방통행_정, 6:일방통행_역
					// link_type : 4; // 링크종별, 0:입구점, 1:본선비분리, 2:본선분리, 3:연결로, 4:교차로, 5:램프, 6:로터리, 7:휴계소(SA), 8:유턴
					// 고가도로,지하차도 옆길, 단지내 도로는 옆단지 도로를 가져올 수 있기에 제외
					// 고속도로, 도시고속, 페리 제외
					// 통행불가, 공사구간 제외
					// 연결로, 교차로, 램프, 로터리 제외
					// 터널, 지하차도 제외

					// 2023-10-11
					// link_dtype: 링크세부종별(dk3), 1:고가도로,지하차도 옆길 -> 허용, 
					// link_type: 링크종별 5:램프 는 level: 경로레벨, 2:국도 이하일 경우 -> 허용 

					(nMatchType == TYPE_LINK_MATCH_CARSTOP) &&
					(/*pLink->veh.link_dtype == 1 || */pLink->veh.link_dtype == 3 || pLink->veh.level > LimitLevel ||
					 pLink->veh.road_type == 1 || pLink->veh.road_type == 2 || pLink->veh.road_type == 4 ||
					 pLink->veh.pass_code == 2 || pLink->veh.pass_code == 4 ||
					 pLink->veh.link_type == 3 || pLink->veh.link_type == 4 || (pLink->veh.link_type == 5 && pLink->veh.level <= 1)  ||
					 pLink->veh.link_type == 6 || pLink->veh.link_type == 8 ||
					 pLink->veh.tunnel == 1 || pLink->veh.under_pass == 1)) ||

					((nMatchType == TYPE_LINK_MATCH_CARSTOP_EX) &&
					(/*pLink->veh.link_dtype == 1 || */ pLink->veh.level > LimitLevel || // 차량 승하자 + 단지내도로(건물입구점 재확인시, 단지도로에 매칭된 입구점을 확인하기 위해 포함) 
					 pLink->veh.road_type == 1 || pLink->veh.road_type == 2 || pLink->veh.road_type == 4 ||
					 pLink->veh.pass_code == 2 || pLink->veh.pass_code == 4 ||
#if defined(USE_P2P_DATA)
					pLink->veh.over_pass == 1 || pLink->veh.hd_flag != 1 || // 전체 링크가 HD로 구성된 링크만 사용(경유지)
#endif
					 pLink->veh.link_type == 3 || pLink->veh.link_type == 4 || (pLink->veh.link_type == 5 && pLink->veh.level <= 1) ||
					 pLink->veh.link_type == 6 || pLink->veh.link_type == 8 ||
					 pLink->veh.tunnel == 1 || pLink->veh.under_pass == 1)) ||

					((nMatchType == TYPE_LINK_MATCH_FOR_TABLE) &&
					(/*pLink->veh.link_dtype == 1 || */ pLink->veh.link_dtype == 3 || pLink->veh.level > LimitLevel ||
					//pLink->veh.lane_cnt <= 2 || // 2차선 이하 제외
					pLink->veh.road_type == 1 || pLink->veh.road_type == 2 || pLink->veh.road_type == 4 ||
					pLink->veh.pass_code == 2 || pLink->veh.pass_code == 4 ||
					pLink->veh.link_type == 3 || pLink->veh.link_type == 4 || (pLink->veh.link_type == 5 && pLink->veh.level <= 1) ||
					pLink->veh.link_type == 6 || pLink->veh.link_type == 8 ||
					pLink->veh.tunnel == 1 || pLink->veh.under_pass == 1)) ||

					((nMatchType == TYPE_LINK_MATCH_FOR_HD) &&
					(/*pLink->veh.link_dtype == 1 || */ /*pLink->veh.link_dtype == 3 || pLink->veh.level >= 9 ||*/ pLink->veh.level > LimitLevel ||
					// 대구 경북 과학 기술원내 도로가 단지내 도로이기에, p2p는 단지내 도로는 포함
					pLink->veh.road_type == 1 || pLink->veh.road_type == 2 || pLink->veh.road_type == 4 ||
					pLink->veh.pass_code == 2 || pLink->veh.pass_code == 4 ||
#if defined(USE_P2P_DATA)
					pLink->veh.over_pass == 1 || pLink->veh.hd_flag == 0 || // HD 링크와 매칭 안되는 링크는 제외
#endif
					pLink->veh.link_type == 3 || pLink->veh.link_type == 4 || (pLink->veh.link_type == 5 && pLink->veh.level <= 1) ||
					pLink->veh.link_type == 6 || pLink->veh.link_type == 8 ||
					pLink->veh.tunnel == 1 || pLink->veh.under_pass == 1)) ||
					
					((minVtxIdx = linkProjection(pLink, lng, lat, nMaxDist, minLng, minLat, minDist, minIr)) < 0)) {
					continue;
				}

#if defined(USE_VEHICLE_DATA)
				if (pTruckOption && (IsAvoidTruckLink(pTruckOption, pLink))) {
					continue;
				}
#endif

				// 링크 고립 체크
#if defined(USE_TMS_API)
				if (checkNextLinkIsolated(pLink, this) != true) {
					if (checkPrevLinkIsolated(pLink, this) != true) {
						LOG_TRACE(LOG_WARNING, "current link isolated, id:%lld, tile:%d, link:%d", pLink->link_id.llid, pLink->link_id.tile_id, pLink->link_id.nid);
						continue;
					}
				}
#endif

#if defined(USE_MULTIPROCESS)
#pragma omp critical
#endif
				if (minDist < retDist) {
					retLink = pLink;
					retLng = minLng;
					retLat = minLat;
					retDist = minDist;

					if (pMatchVtxIdx != nullptr) {
						*pMatchVtxIdx = minVtxIdx;
					}
				}
			} // for
		}
#endif
	} // for

//#pragma omp parallel for
	/*for (int ii = 0; ii < m_vtIndex.size(); ii++)
	{
		stMeshInfo* pMesh = nullptr;
		stLinkInfo* pLink = nullptr;

		if (!isInBox(lng, lat, m_vtIndex[ii].rtData) ||
			!(pMesh = GetMeshDataById(m_vtIndex[ii].idTile))) {
			continue;
		}

		for (int jj = 0; jj < pMesh->links.size(); jj++)
		{
			if (!(pLink = GetLinkDataById(pMesh->links[jj])) ||
				(linkProjection(pLink, lng, lat, minLng, minLat, maxDist) < 0)) {
				continue;
			}

			if (minDist > maxDist) {
				minDist = maxDist;
				minLink = pLink;
			}
		}
	}*/


#if defined(USE_FOREST_DATA)
	int32_t minDistGap = 20; // 20m, 숲길과 보행자길이 동시에 존재하면 n미터 이내의 근접 링크는 숲길을 우선시 한다
	if (nMatchType == TYPE_LINK_MATCH_NONE) {
		minDistGap = 0;
	}

	if (minLinkTrekking != nullptr) {
		retLink = minLinkTrekking;
		retLng = minCoordTrekking.x;
		retLat = minCoordTrekking.y;
		retDist = minDistTrekking;
		if (pMatchVtxIdx != nullptr) {
			*pMatchVtxIdx = minMatchVtxIndexTrekking;
		}
	}

#if defined(USE_PEDESTRIAN_DATA)
	if ((minLinkPedestrian != nullptr) && ((minLinkTrekking == nullptr) || (minDistPedestrian < minDistTrekking - minDistGap))) {
		retLink = minLinkPedestrian;
		retLng = minCoordPedestrian.x;
		retLat = minCoordPedestrian.y;
		retDist = minDistPedestrian;
		if (pMatchVtxIdx != nullptr) {
			*pMatchVtxIdx = minMatchVtxIndexPedestrian;
		}
	}
#endif
#endif

#if defined(USE_PEDESTRIAN_DATA)
	if (minLinkPedestrian != nullptr) {
		retLink = minLinkPedestrian;
		retLng = minCoordPedestrian.x;
		retLat = minCoordPedestrian.y;
		retDist = minDistPedestrian;
		if (pMatchVtxIdx != nullptr) {
			*pMatchVtxIdx = minMatchVtxIndexPedestrian;
		}
	}
#endif // #	if defined(USE_FOREST_DATA)

	return retLink;
}


int32_t CDataManager::GetLinkVertexDataByPoint(IN const double lng, IN const double lat, IN const int32_t nMaxDist, IN const KeyID linkId, OUT double& retLng, OUT double& retLat, OUT double& retDist)
{	
	int32_t idxVertex = -1;
	double minLng = 0;
	double minLat = 0;
	double minDist;
	double retIr = 0;
	stLinkInfo* pLink = nullptr;

	pLink = GetLinkDataById(linkId, TYPE_LINK_DATA_NONE);

	if (pLink) {
		idxVertex = (linkProjection(pLink, lng, lat, nMaxDist, minLng, minLat, minDist, retIr));
	}

	if (idxVertex >= 0) {
		retLng = minLng;
		retLat = minLat;
	}

	return idxVertex;
}


stPolygonInfo* CDataManager::GetPolygonDataByPoint(IN const double lng, IN const double lat, IN const int32_t nType, IN const bool useNeighborMesh)
{
	//static const int32_t nMaxDistBuilding = 1000; // 최대 1km
	//static const int32_t nMaxDistComplex = 5000; // 최대 5km, 골프장이 꽤 넓음.

	stPolygonInfo* pRetPoly = nullptr;
	stMeshInfo* pMesh = nullptr;

#if defined(USE_MOUNTAIN_DATA)
	// 숲길일 경우 폴리곤 정보(산바운더리)는 숲길에만 존재
	pMesh = GetMeshDataById(g_forestMeshId);
#else
	pMesh = GetMeshDataByPoint(lng, lat);
#endif

	if (pMesh != nullptr) {
		vector<stMeshInfo*> vtMeshes = { pMesh };

#if 0//defined(_DEBUG) && defined(USE_BUILDING_DATA)
		// 디버그 공간검색은 로딩 시간이 많이 걸려 주변 메쉬 로딩은 일단 하지 말자 // 2025-08-14
#else
#if defined(USE_FOREST_DATA) && !defined(USE_PEDESTRIAN_DATA)
		if ((useNeighborMesh) && (pMesh->mesh_id.tile_id != g_forestMeshId))
#else
		if (useNeighborMesh) 
#endif
		{
			stMeshInfo* pMeshNeighbor = nullptr;
			double dwDist = m_dataCost.optimal.cost_mes / 100000.;
			SBox meshBoxCheck = { lng - dwDist, lat - dwDist, lng + dwDist, lat + dwDist };

			for (const auto& id : pMesh->neighbors) {
				if ((id != pMesh->mesh_id.tile_id) && ((pMeshNeighbor = GetMeshDataById(id)) != nullptr) && isInPitBox(meshBoxCheck, pMeshNeighbor->data_box)) {
					vtMeshes.emplace_back(pMeshNeighbor);
				}
			} // for
		}
#endif

		stPolygonInfo* pPoly = nullptr;

#if defined(USE_BUILDING_DATA)
		// find in building
		if (m_pMapBuilding && (nType != TYPE_ENT_COMPLEX)) {
			for (const auto& mesh : vtMeshes) {
#ifdef TEST_SPATIALINDEX
				vector<KeyID> vtPolygonId;
				m_pMapMesh->GetPitinPolygonfromSpatialindex(mesh, lng, lat, TYPE_DATA_BUILDING, vtPolygonId);
				for (const auto& key : vtPolygonId) {
					if ((pPoly = m_pMapBuilding->GetPitInPolygon(key, lng, lat, m_dataCost.optimal.cost_bld)) != nullptr) {
						if (nType == TYPE_ENT_NONE && pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT) <= 0) {
							// 입구점이 없으면
							pPoly = nullptr;
							continue;
						}
						pRetPoly = pPoly;
					}
				}
#else // #ifdef TEST_SPATIALINDEX
				bool loopAbort = false;
#if defined(USE_MULTIPROCESS)
#pragma omp parallel firstprivate(pPoly)
#pragma omp for
#endif
				for (int ii = 0; ii < mesh->buildings.size(); ii++) {
#if defined(USE_MULTIPROCESS)
#pragma omp flush(loopAbort)
#endif
					if (!loopAbort) {
						if ((pPoly = m_pMapBuilding->GetPitInPolygon(mesh->buildings[ii], lng, lat, m_dataCost.optimal.cost_bld)) != nullptr) {
							if (nType == TYPE_ENT_NONE && pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT) <= 0) {
								// 입구점이 없으면
								pPoly = nullptr;
								continue;
							}
#if defined(USE_MULTIPROCESS)
#pragma omp critical
#endif
							pRetPoly = pPoly;
							loopAbort = true;
#if defined(USE_MULTIPROCESS)
#pragma omp flush(loopAbort)
#endif
						}
					}
				} // for building
#endif // #ifdef TEST_SPATIALINDEX

				if (pRetPoly != nullptr) {
					break;
				}
			} // for mesh
		}
#endif
#if defined(USE_COMPLEX_DATA) || defined(USE_MOUNTAIN_DATA)
		// find in complex
		if ((pRetPoly == nullptr) && m_pMapComplex && (nType != TYPE_ENT_BUILDING)) {
			for (const auto& mesh : vtMeshes) {
#ifdef TEST_SPATIALINDEX
				vector<KeyID> vtPolygonId;
				m_pMapMesh->GetPitinPolygonfromSpatialindex(mesh, lng, lat, TYPE_DATA_COMPLEX, vtPolygonId);
				for (const auto& key : vtPolygonId) {
					if ((pPoly = m_pMapComplex->GetPitInPolygon(key, lng, lat, m_dataCost.optimal.cost_cpx)) != nullptr) {
						if (nType == TYPE_ENT_NONE && pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT) <= 0 && pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_LINK) <= 0) {
							// 입구점, 단지도로 모두 없으면
							pPoly = nullptr;
							continue;
						} else if (nType != TYPE_ENT_NONE && pPoly->poly_id.poly.type == 0) {
							pPoly = nullptr;
							continue;
						}
						pRetPoly = pPoly;
					}
				} // for
#else // #ifdef TEST_SPATIALINDEX
				bool loopAbort = false;
#if defined(USE_MULTIPROCESS)
#pragma omp parallel firstprivate(pPoly)
#pragma omp for
#endif
				for (int ii = 0; ii < mesh->complexs.size(); ii++) {
#if defined(USE_MULTIPROCESS)
#pragma omp flush(loopAbort)
#endif
					if (!loopAbort) {
						if ((pPoly = m_pMapComplex->GetPitInPolygon(mesh->complexs[ii], lng, lat, m_dataCost.optimal.cost_cpx)) != nullptr) {
							if (nType == TYPE_ENT_NONE && pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT) <= 0 && pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_LINK) <= 0) {
								// 입구점, 단지도로 모두 없으면
								pPoly = nullptr;
								continue;
							} else if (nType != TYPE_ENT_NONE && pPoly->poly_id.poly.type == 0) {
								pPoly = nullptr;
								continue;
							}
#if defined(USE_MULTIPROCESS)
#pragma omp critical
#endif
							pRetPoly = pPoly;
							loopAbort = true;
#if defined(USE_MULTIPROCESS)
#pragma omp flush(loopAbort)
#endif
						}
					}
				} // for complex
#endif // #ifdef TEST_SPATIALINDEX

				if (pRetPoly != nullptr) {
					break;
				}
			} // for mesh
		}
#endif
	}

	return pRetPoly;
}


bool cmpEntPoint(stEntryPointInfo lhs, stEntryPointInfo rhs) {
	return lhs.dwDist < rhs.dwDist;
}


// sort (0:none, 1:dist)
// maxdist (0:not using, else:meeter)
int32_t checkEntType(IN const double dwLng, IN const double dwLat, IN const stEntranceInfo* pEnt, IN const int32_t cntEnt, 
	IN const int32_t nMaxDist, IN const int32_t nEntType, IN const int32_t nSort, OUT vector<stEntryPointInfo>& vtEntInfo) {
	
	int32_t retCnt = 0;
	int32_t idxInfo = vtEntInfo.size();
	double dwDist = 0.f;

	for (int ii = 0; ii < cntEnt; ii++) {
		if ((nEntType != 0 && nEntType != pEnt[ii].ent_code) || nEntType < 0 || 15 < nEntType) {
			continue;
		}

		dwDist = getRealWorldDistance(dwLng, dwLat, pEnt[ii].x, pEnt[ii].y);

		if (nMaxDist == 0 || dwDist <= nMaxDist) {
			stEntryPointInfo entInfo = { 0, };

			entInfo.nAttribute = pEnt[ii].ent_code;
			entInfo.x = pEnt[ii].x;
			entInfo.y = pEnt[ii].y;
			entInfo.dwDist = dwDist;
			entInfo.nAngle = pEnt[ii].angle;

			if (nEntType == TYPE_OPTIMAL_ENTRANCE_MOUNTAIN) {
				entInfo.nID_1 = pEnt[ii].mnt.fnode_id;
				entInfo.nID_2 = pEnt[ii].mnt.wnode_id;
			}

			vtEntInfo.emplace_back(entInfo);
			retCnt++;
		}
	} // for

	 if (retCnt > 1 && nSort == 1) {
		 sort(vtEntInfo.begin() + idxInfo, vtEntInfo.end(), cmpEntPoint);
	 }

	return retCnt;
}


const int getAngle(IN const stLinkInfo* pLink, IN const int32_t nDir, IN const int32_t idxVtx)
{
	int retAng = 0;

	if (pLink == nullptr) {
		LOG_TRACE(LOG_WARNING, "warning, get link angle param null");
		return retAng;
	}

	// 구조체의 자릿수 부족으로 필요 시점에 직접 계산
	MMPoint<double> coord1;
	MMPoint<double> coord2;

	int cntLinkVtx = pLink->getVertexCount();

	// ang
	if ((cntLinkVtx >= 2) && (idxVtx < cntLinkVtx - 1)) {
		if (nDir == 1) { // 정, to enode
			// enode ang
			coord1 = { pLink->getVertexX(idxVtx), pLink->getVertexY(idxVtx) };
			coord2 = { pLink->getVertexX(idxVtx + 1), pLink->getVertexY(idxVtx + 1) };
		}
		else { // 역, to snode			
			// snode ang
			coord1 = { pLink->getVertexX(idxVtx + 1), pLink->getVertexY(idxVtx + 1) };
			coord2 = { pLink->getVertexX(idxVtx), pLink->getVertexY(idxVtx) };
		}
		retAng = coord1.azimuth(coord2);
	}

	return retAng;
}


stLinkInfo * CDataManager::GetNearRoadByPoint(IN const double lng, IN const double lat, IN const int32_t maxDist, IN const int32_t nMatchType, IN const int32_t nLinkDataType, IN const int32_t nLinkLimitLevel, OUT stEntryPointInfo& entInfo)
{
	stLinkInfo * pRetLink = nullptr;

	double retDist = INT_MAX;
	int32_t idxLinkVtx = -1;
	const SPoint reqPos = {lng, lat};
	SPoint retPos = {0,};
	int32_t nDir = 0;
	
	static const double LaneLowDist = 3.0; // 60 미만 차선 기본 거리 미터
	static const double LaneMidDist = 3.25; // 60 이상 차선 기본 거리 미터
	static const double LaneHighDist = 3.5; // 80 이상 차선 기본 거리 미터
	
	stLinkInfo* pLink = GetLinkDataByPointAround(lng, lat, maxDist, retPos.x, retPos.y, retDist, nMatchType, nLinkDataType, nLinkLimitLevel, nullptr, &idxLinkVtx);
	if (pLink != nullptr) {
		double dwDist = getRealWorldDistance(lng, lat, retPos.x, retPos.y);

		// 도로에서 일정 거리 떨어진 지점을 주자 - 경탐시 방향성 적용이 원할하도록 하기 위함
		double dwLaneDist = pLink->veh.lane_cnt;
		if (pLink->veh.lane_cnt >= 6 || pLink->veh.level <= 1) {
			dwLaneDist = pLink->veh.lane_cnt * LaneHighDist;
		} else if (pLink->veh.lane_cnt >= 4 || pLink->veh.level <= 4) {
			dwLaneDist = pLink->veh.lane_cnt * LaneMidDist;
		} else { //if () {
			dwLaneDist = pLink->veh.lane_cnt * LaneLowDist;
		}
		
		// 근접 링크 좌표에서 일정거리 띄워진 좌표
		SPoint newRetPos = retPos;
		
		// 분리도로, 5:일방통행_정, 6 : 일방통행_역 
		if ( (pLink->veh.link_type == 2) 
			//&& ((pLink->veh.pass_code == 5) || (pLink->veh.pass_code == 6))
			&& (idxLinkVtx >= 0 && idxLinkVtx < pLink->getVertexCount() - 1) ) {
			bool isLeft = false;

			nDir = (pLink->veh.pass_code == 5) ? 1 : 2;

			if (nDir == 1) { // 정
				isLeft = isLeftSide(pLink->getVertex(idxLinkVtx), pLink->getVertex(idxLinkVtx + 1), &reqPos);
			} else { // 역
				isLeft = isLeftSide(pLink->getVertex(idxLinkVtx + 1), pLink->getVertex(idxLinkVtx), &reqPos);
			}

			// 분리도로일 경우 도로 이격에 마진을 조금 줄여주자
			if (pLink->veh.lane_cnt > 1)
				dwLaneDist *= .65;

			if (isLeft) { // 무조건 도로 우측 방향으로 위치시켜 주자
				// 일방에서 좌측에 위치하면 우측 도로변으로 위치시켜 주자
				if (getPointByDistance(lng, lat, newRetPos.x, newRetPos.y, dwDist + dwLaneDist) == true) {
					entInfo.dwDist = dwDist + dwLaneDist;
				} else {
					entInfo.dwDist = dwDist;
				}

				nDir = (nDir == 1) ? 2 : 1; // 좌우가 바뀌면 링크 방향성도 변경
			} else { // 우측에 있으면
				// 도로로부터 지정된거리(도로너비) 만큼 띄워 도로변으로 위치시켜 주자
				if (getPointByDistance(lng, lat, newRetPos.x, newRetPos.y, dwDist - dwLaneDist) == true) {
					entInfo.dwDist = abs(dwDist - dwLaneDist);
				} else {
					entInfo.dwDist = dwDist;
				}
			}
		} else { // if (pLink->veh.link_type == 1) { // 비분리도로일경우 {
			bool isLeft = isLeftSide(pLink->getVertex(idxLinkVtx), pLink->getVertex(idxLinkVtx + 1), &reqPos);
			if (isLeft) { // 양방향 왼쪽이면 역
				nDir = 2; 
			} else { // 양방향 오른쪽이면 정
				nDir = 1;
			}

			dwLaneDist *= .5;

			// 도로내에 매칭되면 도로변으로 위치시키자
			if (dwDist <= dwLaneDist) {
				// 지정거리 이내(도로 내)면 도로변으로 위치시켜 주자
				double lngNext = lng;
				double latNext = lat;

				if (getPointByDistance(lng, lat, newRetPos.x, newRetPos.y, dwDist - dwLaneDist) == true) {
					entInfo.dwDist = abs(dwDist - dwLaneDist);
				} else {
					entInfo.dwDist = dwDist;
				}
			}
			else {
				// 도로 중심으로부터 지정된거리(도로너비) 만큼 띄워 도로밖으로 위치시켜 주자
				if (getPointByDistance(lng, lat, newRetPos.x, newRetPos.y, dwDist - dwLaneDist) == true) {
					entInfo.dwDist = abs(dwDist - dwLaneDist);
				} else {
					entInfo.dwDist = dwDist;
				}							
			}
		}

		entInfo.nAngle = getAngle(pLink, nDir, idxLinkVtx);

		// 새로운 지점 주변에 원래의 도로 말고, 또다른 가까운 도로가 있다면 원래의 도로에서 도로너비만큼 띄우지 말고, 최소의 너비만 띄운 좌표로 사용하자
		// 근접 링크 좌표에서 일정거리 띄워진 좌표
		double firstLinkDist = retDist;
		retDist = dwLaneDist;
		SPoint nearRetPos = newRetPos;
		//stLinkInfo* pNewLink = GetLinkDataByPointAround(newRetPos.x, newRetPos.y, dwLaneDist, nearRetPos.x, nearRetPos.y, retDist, TYPE_LINK_MATCH_NONE, &idxLinkVtx);
		stLinkInfo* pNewLink = GetLinkDataByPointAround(newRetPos.x, newRetPos.y, dwLaneDist + LaneLowDist, nearRetPos.x, nearRetPos.y, retDist, TYPE_LINK_MATCH_NONE, nLinkDataType, nLinkLimitLevel, nullptr, &idxLinkVtx);

		if (pNewLink != nullptr && pNewLink != pLink) {
			// 이격 거리 주변 도로보다 가까운 거리로 재설정 하자
			static const double minLimitDist = 1.0;
			const double dwMinDist = abs(dwLaneDist - retDist) * .3f; // 주변 가까운 도로보다 70% 더 가깝게 위치시키자.
			// 최소한 0.7m는 유지시키자, 너무 작아지면 실제 사용자 매칭시 도로 반대편에 적용될 수 있음.
			if (dwMinDist < minLimitDist) {
				if (getPointByDistance(retPos.x, retPos.y, newRetPos.x, newRetPos.y, minLimitDist) == true) {
					entInfo.dwDist = abs(dwDist - minLimitDist);
				} else {
					entInfo.dwDist = dwDist;
				}

				entInfo.x = newRetPos.x;
				entInfo.y = newRetPos.y;
			} else {
				if (getPointByDistance(retPos.x, retPos.y, nearRetPos.x, nearRetPos.y, dwMinDist) == true) {
					entInfo.dwDist = abs(dwDist - dwMinDist);
				} else {
					entInfo.dwDist = dwDist;
				}

				entInfo.x = nearRetPos.x;
				entInfo.y = nearRetPos.y;
			}

			//if (getPointByDistance(retPos.x, retPos.y, nearRetPos.x, nearRetPos.y, dwMinDist) == true) {
			//	entInfo.dwDist = abs(dwDist - dwMinDist);
			//} else {
			//	entInfo.dwDist = dwDist;
			//}
		} 
		else {
			entInfo.x = newRetPos.x;
			entInfo.y = newRetPos.y;
		}

		pRetLink = pLink;
	} // if

	return pRetLink;
}


int32_t CDataManager::GetOptimalPointDataByPoint(IN const double lng, IN const double lat, OUT stOptimalPointInfo* pOptInfo, IN const int32_t nEntType, IN const int32_t nReqCount, IN const int32_t nMatchType, IN const int32_t nLinkDataType, IN const int32_t nSubOption)
{
	int32_t cntRet = 0;
	stReqOptimal reqOpt;
	reqOpt.x = lng;
	reqOpt.y = lat;
	reqOpt.typeAll = nEntType;

	if (pOptInfo != nullptr)
	{
		const SPoint reqCoord = { lng, lat };
		vector<stEntryPointInfo>  vtEntInfo; // 입구점
		vector<stEntryPointInfo>  vtExpInfo; // 추가 속성 (단지내도로, 최근접도로)

		stPolygonInfo* pPoly = nullptr;
		
		pOptInfo->id = NULL_VALUE;
		pOptInfo->nType = TYPE_ENT_NONE;
		pOptInfo->x = lng;
		pOptInfo->y = lat;

		if ( reqOpt.typeAll != 99 ) { // 폴리곤무시
			pPoly = GetPolygonDataByPoint(lng, lat/*, reqOpt.typeAll*/);
		}


		if (pPoly != nullptr) {	
			// 폴리곤 정보 복사
			//pOptInfo->vtPolygon.assign(pPoly->vtVtx.begin(), pPoly->vtVtx.end());
			const SPoint* pPolygon = pPoly->getAttributeVertex();
			const stEntranceInfo* pEnt = pPoly->getAttributeEntrance();
			const KeyID* pLinkKey = pPoly->getAttributeLink();

			int cntEnt = pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT);
			int cntLinkKey = pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_LINK);

			pOptInfo->id = pPoly->poly_id.llid;

			//std::copy(pData, pData + pPoly->getAttributeSize(TYPE_POLYGON_DATA_ATTR_VTX), pOptInfo->vtPolygon);
			pOptInfo->vtPolygon.insert(pOptInfo->vtPolygon.end(), &pPolygon[0], &pPolygon[pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX)]);

			if (pPoly->poly_id.poly.type == TYPE_POLYGON_BUILDING) { // 빌딩
				const int32_t nMaxDist = 1000; // 최대 1km
				//if (pPoly->bld.name > 0) {
				//	pOptInfo->name = GetNameDataByIdx(pPoly->bld.name);
				//}

				// 입구점 매칭
				//for (vector<stEntranceInfo>::const_iterator it = pPoly->vtEnt.begin(); it != pPoly->vtEnt.end(); it++) {
				if (reqOpt.typeAll == 0) // 전체 타입
				{
					cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, 0, 0, vtEntInfo);
				}
				else
				{
					// 최대 4번의 스텝까지 허용
					// nEntType, 입구점 타입 // 0:알아서, 1: 차량 입구점, 2: 택시 승하차 지점(건물), 3: 택시 승하차 지점(건물군), 4: 택배 차량 하차 거점, 5: 보행자 입구점, 	6: 배달 하차점(차량, 오토바이), 7: 배달 하차점(자전거, 도보)
					// 1st
					if (reqOpt.type1st != 0) {
						cntRet += checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqOpt.type1st, 1, vtEntInfo);
					}
					// 2nd
					if (cntRet <= 0 && reqOpt.type2nd != 0) {
						cntRet += checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqOpt.type2nd, 1, vtEntInfo);
					}
					// 3rd
					if (cntRet <= 0 && reqOpt.type3rd != 0) {
						cntRet += checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqOpt.type3rd, 1, vtEntInfo);
					}
					// 4th
					if (cntRet <= 0 && reqOpt.type4th != 0) {
						cntRet += checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqOpt.type4th, 1, vtEntInfo);
					}
				}


				// // 일부 건물입구점이 도로가 아닌 건물 폴리곤에 접하는 경우, 인접 도로로 좌표 변경
				if (cntRet == 1) {
				// 	const int32_t maxDist = 100;// 최대 100 내에서 검색
				// 	const int32_t maxDistFromRoad = 15; // 입구점이 도로로부터 최대 15m 내에 있어야 함
				// 	SPoint retPos = {0,};
				// 	const SPoint newReqPos = {vtEntInfo[0].x, vtEntInfo[0].y};
				// 	stEntryPointInfo entInfo = {0,};
				// 	int bldApt = (pPoly->bld.code == TYPE_BUILDING_APT) ? TYPE_LINK_MATCH_CARSTOP_EX : TYPE_LINK_MATCH_CARSTOP;

				// 	if ((GetNearRoadByPoint(newReqPos.x, newReqPos.y, maxDist, bldApt, entInfo)) == false) {
				// 		// 건물 입구점 주변에 매칭되는 도로가 없으면 이건 무시해야 되는 데이터로 판단하자
				// 		vtEntInfo.clear();
				// 		vector<stEntryPointInfo>().swap(vtEntInfo);

				// 		LOG_TRACE(LOG_WARNING, "dosen't have near(%dm) road for building ent, x:%.5f, y:%.5f", maxDist, vtEntInfo[0].x, vtEntInfo[0].y);
				// 	} else {
				// 		double dwDist = getRealWorldDistance(newReqPos.x, newReqPos.y, entInfo.x, entInfo.y);

				// 		// 입구점이 주변 도로로 부터 너무 멀리 떨어져 있으면
				// 		if (maxDistFromRoad < dwDist) {
				// 			// 검색된 가장 가까운 도로를 사용하자
				// 			vtEntInfo.insert(vtEntInfo.begin(), entInfo);
				// 			pOptInfo->nType = TYPE_ENT_NEAR_ROAD;
				// 			LOG_TRACE(LOG_WARNING, "entrance point too far from near road, change to near road point, building ent, x:%.5f, y:%.5f, near road x:%.5f, y:%.5f", maxDist, vtEntInfo[0].x, vtEntInfo[0].y, entInfo.x, entInfo.y);
				// 		}
						pOptInfo->nType = TYPE_ENT_BUILDING;
				// 	}
				} else if (cntRet > 1) {
					pOptInfo->nType = TYPE_ENT_BUILDING;
				} else {
					// 빌딩에서 아무것도 얻지 못하면, 단지에서 얻어보자(단, 단지 타입을 요청했을 경우)
					if ((reqOpt.typeAll == 0) || (reqOpt.type1st == 3) || (reqOpt.type2nd == 3) || (reqOpt.type3rd == 3) || (reqOpt.type4th == 3)) {
						if ((pPoly = GetPolygonDataByPoint(lng, lat, TYPE_ENT_COMPLEX)) != nullptr) {
							// 폴리곤 정보 복사
							pPolygon = pPoly->getAttributeVertex();
							pEnt = pPoly->getAttributeEntrance();
							pLinkKey = pPoly->getAttributeLink();

							cntEnt = pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT);
							cntLinkKey = pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_LINK);

							pOptInfo->id = pPoly->poly_id.llid;
							pOptInfo->vtPolygon.assign(&pPolygon[0], &pPolygon[pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX)]);
						}
					}
				}
			}
			
			if (pPoly && (pPoly->poly_id.poly.type == TYPE_POLYGON_COMPLEX) && (cntRet <= 0)) { // 단지 
				int32_t nMaxDist = 100000; // 최대 100km, 산바운더리는 꽤 넓음

				if (pPoly->poly_id.poly.type == TYPE_POLYGON_COMPLEX) {
					nMaxDist = 5000; // 최대 5km, 골프장이 꽤 넓음.
				}
				else if (pPoly->poly_id.poly.type == TYPE_POLYGON_MOUNTAIN) {
					nMaxDist = 100000; // 최대 100km, 산입구점 최장거리 확인 필요 
				}
				else { // 산바운더리 없는 입구점은 N미터까지 허용
					nMaxDist = 100000; // 최대 5km, 골프장이 꽤 넓음.
				}

				if (reqOpt.typeAll == 0) // 전체 타입
				{
					cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, 0, 0, vtEntInfo);
				}
				else if (pPoly->poly_id.poly.type == 0 && reqOpt.typeAll != 0)
				{
					; // not thing
				}
				else
				{
					// 최대 4번의 스텝까지 허용
					// nEntType, 차량 입구점 타입 // 0:알아서, 1: 차량 입구점, 2: 택시 승하차 지점(건물), 3: 택시 승하차 지점(건물군), 4: 택배 차량 하차 거점, 5: 보행자 입구점, 	6: 배달 하차점(차량, 오토바이), 7: 배달 하차점(자전거, 도보)
					// nEntType, 숲길 입구점 타입 // 0:알아서, 1: 숲길 입구점

					// 1st
					if (reqOpt.type1st != 0) {
						cntRet += checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqOpt.type1st, 1, vtEntInfo);
					}					
					// 2nd
					if (cntRet <= 0 && reqOpt.type2nd != 0) {
						cntRet += checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqOpt.type2nd, 1, vtEntInfo);
					}
					// 3rd
					if (cntRet <= 0 && reqOpt.type3rd != 0) {
						cntRet += checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqOpt.type3rd, 1, vtEntInfo);
					}
					// 4th
					if (cntRet <= 0 && reqOpt.type4th != 0) {
						cntRet += checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqOpt.type4th, 1, vtEntInfo);
					}
				}

				if (cntRet > 0) {
					if (pPoly->poly_id.poly.type == TYPE_POLYGON_MOUNTAIN) {
						pOptInfo->nType = TYPE_ENT_MOUNTAIN;
					} else {
						pOptInfo->nType = TYPE_ENT_COMPLEX;
					}
				}


				// 가장 가까운 단지내 도로 매칭
				stEntryPointInfo entInfo = { 0, };
				entInfo.dwDist = INT_MAX;

				stLinkInfo* pLink;
				double nMinDist = m_dataCost.optimal.cost_cpx;
				double retLng, retLat, retDist, retIr, minIr;

				//for (vector<KeyID>::const_iterator it = pPoly->vtLink.begin(); it != pPoly->vtLink.end(); it++) {
				for (int ii = 0; ii < cntLinkKey; ii++) {
					pLink = GetVLinkDataById(pLinkKey[ii]);
					if (pLink != nullptr) {

						int idxVtx = 0;
						if ((idxVtx = linkProjection(pLink, lng, lat, nMinDist, retLng, retLat, retDist, retIr)) < 0) {
							continue;
						}

						if ((retDist < nMinDist) ||
							((static_cast<int32_t>(retDist) == nMinDist) && (minIr <= 0 || IR_PRECISION <= minIr)) ||
							((static_cast<int32_t>(retDist) == nMinDist) && abs(retIr) < abs(minIr)))
						{
							nMinDist = static_cast<int32_t>(retDist);
							minIr = retIr;

							entInfo.nAttribute = 0;
							entInfo.x = retLng;
							entInfo.y = retLat;
							entInfo.dwDist = retDist;

							int nDir = 0;
							bool isLeft = isLeftSide(pLink->getVertex(idxVtx), pLink->getVertex(idxVtx + 1), &reqCoord);
							if (isLeft) { // 양방향 왼쪽이면 역
								nDir = 2;
							} else { // 양방향 오른쪽이면 정
								nDir = 1;
							}
							entInfo.nAngle = getAngle(pLink, nDir, idxVtx);
						}
					}
				} // for

				// 단지내 도로 매칭 되었으면 추가
				if (entInfo.dwDist < INT_MAX) {
					if (pOptInfo->nType == TYPE_ENT_NONE) {
						// 결과 타입을 단지내 도로로 적용
						pOptInfo->nType = TYPE_ENT_CPX_ROAD;
					}
					vtExpInfo.emplace_back(entInfo);
				}
			}
		}
		
		// 지하철입구점, 
		if (reqOpt.typeAll != TYPE_OPTIMAL_ENTRANCE_SUBWAY) {
			stEntryPointInfo entInfo = { 0, };
			stLinkInfo * pNearRoad = nullptr;
			SPoint retPos = { 0.f, };
			double retDist = 0.f;

			// 도로 사이드에 위치시킬 최적 지점 이격 거리
			for (int ii = 0; ii < 5; ii++) {
				if ((pNearRoad = GetLinkDataByPointAround(lng, lat, m_dataCost.optimal.cost_lv[ii], retPos.x, retPos.y, retDist, nMatchType, nLinkDataType)) != nullptr) {

					if (pNearRoad->ped.facility_type == TYPE_OBJECT_SUBWAY) {

					}
					break;
				}
			}

			// 입구점 정보에 각도 정보 추가
			for (auto& ent : vtEntInfo) {
				// 최적지점에 각도가 없으면 검색 후 서치
				if (ent.nAngle == 0x1FF) {
					pNearRoad = nullptr;

					

					if (pNearRoad != nullptr && entInfo.dwDist < INT_MAX) {
						ent.nAngle = entInfo.nAngle;
					}

					LOG_TRACE(LOG_INFO, "additional search as no angle, reqX:%.6f, reqY:%.6f, polyId:%d, entY:%.6f, entY:%.6f", pOptInfo->x, pOptInfo->y, pOptInfo->id, ent.x, ent.y);
				}
			}


			// 입구점 정보가 없으면 주변 가까운 도로 검출
			if (vtEntInfo.empty() || nSubOption == 1) {
				pNearRoad = nullptr;

				// 도로 사이드에 위치시킬 최적 지점 이격 거리
				for (int ii = 0; ii < 5; ii++) {
#if defined(USE_FOREST_DATA)
					if ((pNearRoad = GetNearRoadByPoint(reqCoord.x, reqCoord.y, m_dataCost.optimal.cost_lv[ii], TYPE_LINK_MATCH_NONE, nLinkDataType, -1, entInfo)) != nullptr)
#else
					if ((pNearRoad = GetNearRoadByPoint(reqCoord.x, reqCoord.y, m_dataCost.optimal.cost_lv[ii], TYPE_LINK_MATCH_CARSTOP, nLinkDataType, -1, entInfo)) != nullptr)
#endif
					{
						break;
					}
				}

				if (pNearRoad != nullptr && entInfo.dwDist < INT_MAX) {
					if (pOptInfo->nType == TYPE_ENT_NONE) {
						// 결과 타입을 주변 도로로 적용
						pOptInfo->nType = TYPE_ENT_NEAR_ROAD;
						pOptInfo->id = pNearRoad->link_id.llid;
					}
					vtExpInfo.emplace_back(entInfo);
				}
			}
		} else if (reqOpt.typeAll != TYPE_OPTIMAL_ENTRANCE_MOUNTAIN) { // 숲길입구점이 아니면 요청시에는 도로를 찾자
			stEntryPointInfo entInfo = { 0, };
			stLinkInfo * pNearRoad = nullptr;

			// 입구점 정보에 각도 정보 추가
			for (auto& ent : vtEntInfo) {
				// 최적지점에 각도가 없으면 검색 후 서치
				if (ent.nAngle == 0x1FF) {
					pNearRoad = nullptr;

					// 도로 사이드에 위치시킬 최적 지점 이격 거리
					for (int ii = 0; ii < 5; ii++) {
						if ((pNearRoad = GetNearRoadByPoint(ent.x, ent.y, m_dataCost.optimal.cost_lv[ii], TYPE_LINK_MATCH_CARSTOP_EX, nLinkDataType, -1, entInfo)) != nullptr) {
							break;
						}
					}

					if (pNearRoad != nullptr && entInfo.dwDist < INT_MAX) {
						ent.nAngle = entInfo.nAngle;
					}

					LOG_TRACE(LOG_INFO, "additional search as no angle, reqX:%.6f, reqY:%.6f, polyId:%d, entY:%.6f, entY:%.6f", pOptInfo->x, pOptInfo->y, pOptInfo->id, ent.x, ent.y);
				}
			}


			// 입구점 정보가 없으면 주변 가까운 도로 검출
			if (vtEntInfo.empty() || nSubOption == 1) {
				pNearRoad = nullptr;

				// 도로 사이드에 위치시킬 최적 지점 이격 거리
				for (int ii = 0; ii < 5; ii++) {
#if defined(USE_FOREST_DATA)
					if ((pNearRoad = GetNearRoadByPoint(reqCoord.x, reqCoord.y, m_dataCost.optimal.cost_lv[ii], TYPE_LINK_MATCH_NONE, nLinkDataType, -1, entInfo)) != nullptr)
#else
					if ((pNearRoad = GetNearRoadByPoint(reqCoord.x, reqCoord.y, m_dataCost.optimal.cost_lv[ii], TYPE_LINK_MATCH_CARSTOP, nLinkDataType, -1, entInfo)) != nullptr)
#endif
					{
						break;
					}
				}

				if (pNearRoad != nullptr && entInfo.dwDist < INT_MAX) {
					if (pOptInfo->nType == TYPE_ENT_NONE) {
						// 결과 타입을 주변 도로로 적용
						pOptInfo->nType = TYPE_ENT_NEAR_ROAD;
						pOptInfo->id = pNearRoad->link_id.llid;
					}
					vtExpInfo.emplace_back(entInfo);
				}
			}
		}

		cntRet = vtEntInfo.size();
		if (cntRet <= 0 && vtExpInfo.empty()) {
			//sprintf((char*)m_strMsg.c_str(), "Can't find optimal points, polyId:%d, polyType:%d", pOptInfo->id, pOptInfo->nType);
		}
		else {
			if (cntRet > 1 && reqOpt.typeAll == 0) {
				sort(vtEntInfo.begin(), vtEntInfo.end(), cmpEntPoint);
			}

			// 요청된 갯수만 적용
			if (nReqCount <= 0 || cntRet <= nReqCount) {
				pOptInfo->vtEntryPoint.assign(vtEntInfo.begin(), vtEntInfo.end());
			} else {
				pOptInfo->vtEntryPoint.assign(vtEntInfo.begin(), vtEntInfo.begin() + nReqCount);
				cntRet = nReqCount;
			}

			// 추가 속성 적용
			if (!vtExpInfo.empty()) {
				for (int ii = 0; ii < vtExpInfo.size(); ii++) {
					pOptInfo->vtEntryPoint.emplace_back(vtExpInfo[ii]);
					cntRet++;
				}
				//LOG_TRACE(LOG_DEBUG, "Polygon EntryPoint  Info, id:%d, type:%d, cnt:%d, ext:%d", pOptInfo->id, pOptInfo->nType, cntRet, vtExpInfo.size());
			} else {
				//LOG_TRACE(LOG_DEBUG, "Polygon EntryPoint Info, id:%d, type:%d, cnt:%d", pOptInfo->id, pOptInfo->nType, cntRet);
			}
		}
	}

	return cntRet;
}


int32_t CDataManager::GetRequestMultiOptimalPoints(IN const char* szRequest, OUT vector<SPoint>& vtOrigins, OUT stReqOptimal& reqOpt)
{
	int32_t ret = ROUTE_RESULT_SUCCESS;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	int userId = 0;
	if (root != NULL) {
		cJSON* pValue = cJSON_GetObjectItem(root, "id");
		if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
			userId = cJSON_GetNumberValue(pValue);
		} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
			userId = atoi(cJSON_GetStringValue(pValue));
		}

		if (userId == 0 || userId == NULL_VALUE) {
			userId = 13578642;
		}

		cJSON* pOption = cJSON_GetObjectItem(root, "option");
		if (pOption != NULL) {
			pValue = cJSON_GetObjectItem(pOption, "type");
			if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
				reqOpt.typeAll = cJSON_GetNumberValue(pValue);
			} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
				reqOpt.typeAll = atoi(cJSON_GetStringValue(pValue));
			}

			pValue = cJSON_GetObjectItem(pOption, "count");
			if ((pValue != NULL) && cJSON_IsNumber(pValue)) {
				reqOpt.reqCount = cJSON_GetNumberValue(pValue);
			} else if ((pValue != NULL) && cJSON_IsString(pValue)) {
				reqOpt.reqCount = atoi(cJSON_GetStringValue(pValue));
			} else {
				reqOpt.reqCount = 0;
			}
		}

		cJSON* pOrigins = cJSON_GetObjectItem(root, "origins");
		if (pOrigins != NULL) {
			int cntPoints = cJSON_GetArraySize(pOrigins);

			if (cntPoints > 0) {
				vtOrigins.reserve(cntPoints);

				SPoint tmpCoord;
				cJSON* pCoords;
				for (int ii = 0; ii < cntPoints; ii++) {
					pCoords = cJSON_GetArrayItem(pOrigins, ii);
					if ((pCoords != NULL) && (cJSON_GetArraySize(pCoords) == 2)) {
						pValue = cJSON_GetArrayItem(pCoords, 0);
						tmpCoord.x = cJSON_GetNumberValue(pValue);
						pValue = cJSON_GetArrayItem(pCoords, 1);
						tmpCoord.y = cJSON_GetNumberValue(pValue);

						vtOrigins.emplace_back(tmpCoord);
					}
				} // for
			}
		}

		cJSON_Delete(root);
	} // cJSON
#endif // #if defined(USE_CJSON)

	return ret;
}


int32_t CDataManager::GetMultiOptimalPointDataByPoints(IN const vector<SPoint>& vtOrigins, OUT vector<stOptimalPointInfo>& vtOptInfos, IN const int32_t nEntType, IN const int32_t nReqCount, IN const int32_t nSubOption, IN const int32_t nMatchType, IN const int32_t nLinkDataType)
{
	int32_t ret = 0;

	int cntPoints = vtOrigins.size();

	time_t startTime = LOG_TRACE(LOG_DEBUG, "start, multi optimal position search");

	if (cntPoints > 0) {
		vtOptInfos.resize(cntPoints);

#if defined(USE_MULTIPROCESS)
#pragma omp parallel for
#endif
		for (int ii = 0; ii < cntPoints; ii++) {
			if (GetOptimalPointDataByPoint(vtOrigins[ii].x, vtOrigins[ii].y, &vtOptInfos[ii], nEntType, nReqCount, nMatchType, nLinkDataType, nSubOption) >= 1) {
				ret++;
			}
		}
	}

#if defined(USE_MULTIPROCESS)
	LOG_TRACE(LOG_DEBUG, startTime, "ent, multi optimal position search(omp), cnt:%d", ret);
#else 
	LOG_TRACE(LOG_DEBUG, startTime, "ent, multi optimal position search, cnt:%d", ret);
#endif

	return ret;
}


stLinkInfo * CDataManager::GetNearLinkDataByCourseId(IN const int32_t nCourseId, IN const double lng, IN const double lat, IN const int32_t nMaxDist, OUT double& retLng, OUT double& retLat, OUT double& retDist)
{
	stLinkInfo* retLink = nullptr;

	KeyID key;
	int nMinDist = nMaxDist;
	double retIr, minIr;

	set<uint64_t>* psetCourse = GetLinkByCourse(nCourseId);
	if (psetCourse != nullptr) {
		for (const auto& linkid : *psetCourse) {
			key.llid = linkid;
			stLinkInfo* pLink = GetFLinkDataById(key);
			if (pLink != nullptr) {
				int idxVtx = 0;
				if ((idxVtx = linkProjection(pLink, lng, lat, nMinDist, retLng, retLat, retDist, retIr)) < 0) {
					continue;
				}

				if ((retDist < nMinDist) ||
					((static_cast<int32_t>(retDist) == nMinDist) && (minIr <= 0 || IR_PRECISION <= minIr)) ||
					((static_cast<int32_t>(retDist) == nMinDist) && abs(retIr) < abs(minIr))) {
					nMinDist = static_cast<int32_t>(retDist);
					minIr = retIr;

					retLink = pLink;
				}
			}
		}
	}

	return retLink;
}


int32_t CDataManager::GetSubwayDataByPoint(IN const double lng, IN const double lat, IN const int32_t maxDist, OUT stEntryPointInfo& entInfo)
{
	int32_t retCnt = 0;


	stMeshInfo* pMesh = nullptr;
	stLinkInfo* retLink = nullptr;


#if defined(USE_PEDESTRIAN_DATA)
	stLinkInfo* minLinkPedestrian = nullptr;
	double minDistPedestrian = INT_MAX;
	int32_t minMatchVtxIndexPedestrian = -1;
	SPoint minCoordPedestrian = { 0, };
#endif

	static const uint32_t s_maxMesh = 9;

	stMeshInfo* s_ppMesh[s_maxMesh];
	uint32_t nMaxMesh = 0;


#if defined(USE_DATA_CACHE)
	if (!m_mapFileIndex.empty() && (pMesh = GetMeshDataByPoint(lng, lat)) != nullptr) {
		s_ppMesh[nMaxMesh++] = pMesh;
	}
#else
	if (m_mapFileIndex.empty() && GetMeshCount() > 0) {
		// 파일 캐쉬가 아닌 데이터 버퍼 로딩 상태일 경우
		nMaxMesh = GetMeshDataByPoint(lng, lat, s_maxMesh, s_ppMesh);
	}

	// 주변 메쉬를 추가
	if (nMaxMesh == 1 && !s_ppMesh[0]->neighbors.empty()) {
		for (unordered_set<uint32_t>::const_iterator it = s_ppMesh[0]->neighbors.begin(); it != s_ppMesh[0]->neighbors.end(); it++) {
			// 주변 메쉬의 중심 거리가 최대 요청 거리 이하일 경우만 추가
			if ((pMesh = GetMeshDataById(*it)) != nullptr && isInBox(lng, lat, pMesh->data_box, maxDist)) {
				s_ppMesh[nMaxMesh++] = pMesh;
			}
		} // for
	}
#endif

#if defined(USE_MULTIPROCESS)
	volatile bool flag = false;
#endif

	//#pragma omp parallel for
	for (int32_t ii = 0; ii < nMaxMesh; ii++) {
#if defined(USE_MULTIPROCESS)
		if (flag) continue;
#endif

		stMeshInfo* pMesh = s_ppMesh[ii];
		if (pMesh == nullptr) {
			continue;
		}

		stLinkInfo* pLink = nullptr;
		double minLng = 0.f;
		double minLat = 0.f;
		double minDist = maxDist;
		double minIr = 0.f;
		int minVtxIdx = -1;

#if defined(USE_PEDESTRIAN_DATA)
#ifdef TEST_SPATIALINDEX
		if (pMesh->wlinkTree != nullptr) {
			vector<KeyID> vtLinkId;
			m_pMapMesh->GetNearLinksfromSpatialindex(pMesh, lng, lat, maxDist, vtLinkId);
			for (int ii = 0; ii < vtLinkId.size(); ii++) {
				if (!(pLink = GetWLinkDataById(vtLinkId[ii]))) {
#else // #ifdef TEST_SPATIALINDEX
		if (!pMesh->wlinks.empty()) {
#if defined(USE_MULTIPROCESS)
#pragma omp parallel firstprivate(pLink, minLng, minLat, minDist, minVtxIdx, minIr)
#pragma omp for
#endif
			for (int ii = 0; ii < pMesh->wlinks.size(); ii++) {
				if (!(pLink = GetWLinkDataById(pMesh->wlinks[ii]))) {
#endif // #ifdef TEST_SPATIALINDEX

					if (pLink->ped.facility_type == TYPE_OBJECT_SUBWAY) {
						continue;
					}
				}
#if defined(USE_MULTIPROCESS)
#pragma omp critical
#endif
				if (minDist < minDistPedestrian) {
					minLinkPedestrian = pLink;
					minCoordPedestrian.x = minLng;
					minCoordPedestrian.y = minLat;
					minDistPedestrian = minDist;
					minMatchVtxIndexPedestrian = minVtxIdx;
				}
			} // for
		}
#endif
	} // for mesh

	return retCnt;
}

//const char* CDataManager::GetErrorMessage(void)
//{
//	return m_strMsg.c_str();
//}


bool CDataManager::IsAvoidTruckLink(IN const TruckOption* pTruckOption, IN const stLinkInfo* pLink)
{
	bool ret = false;

	if (pTruckOption && pTruckOption->isEnableTruckOption() && pLink) {
		if (pLink->veh.height) {
			double height = GetExtendDataById(pLink->link_id, TYPE_LINKEX_HEIGHT, TYPE_KEY_LINK);
			if ((height > 0.f) && ((height * 100) < pTruckOption->height)) {
				ret = true;
			}
		}
		if (!ret && pLink->veh.weight) {
			double weight = GetExtendDataById(pLink->link_id, TYPE_LINKEX_WEIGHT, TYPE_KEY_LINK);
			if ((weight > 0.f) && ((weight * 1000) < pTruckOption->weight)) {
				ret = true;
			}
		}
	}

	return ret;
}


bool CDataManager::GetNewData(IN const uint32_t idTile)
{
	// id 매칭, 로딩되야 할 메쉬
	unordered_map<uint32_t, FileIndex> mapNeedMatch;
	unordered_map<uint32_t, FileIndex>::const_iterator it;
	if ((it = m_mapFileIndex.find(idTile)) != m_mapFileIndex.end()) {
		mapNeedMatch.emplace(idTile, it->second);
	}

	return CalculateCache(mapNeedMatch);
}


// 영역에 만족할때까지 주변 메쉬들을 계속 확장하며 확인
int32_t FindNearMesh(IN const SBox& rtWorld, IN const unordered_map<uint32_t, FileIndex>& mapIdx, IN const uint32_t idMesh, OUT unordered_map<uint32_t, FileIndex>& retMeshs)
{
	int32_t cntFirst = retMeshs.size();
	int32_t cntLast = 0;
	unordered_map<uint32_t, FileIndex>::const_iterator it;

	// 주변 메쉬 id
	if ((it = mapIdx.find(idMesh)) != mapIdx.end()) {
		// 바디 데이터가 존재하고, 영역에 들어오는 메쉬면 추가
		if ((it->second.szBody >= 0) && isInPitBox(it->second.rtData, rtWorld)) {
			retMeshs.emplace(it->second.idTile, it->second);

			// 주변 메쉬 포함
			for (int ii = 0; ii < 8; ii++) {
				if ((it->second.idNeighborTile[ii] > 0) && (retMeshs.find(it->second.idNeighborTile[ii]) == retMeshs.end())) {
					FindNearMesh(rtWorld, mapIdx, it->second.idNeighborTile[ii], retMeshs);
				}
			} // for

			return true;
		}
	}

	return false;
}


bool CDataManager::GetNewData(IN const SBox& rtWorld)
{
	uint32_t cntMatch = 0;
	uint32_t idMesh = NULL_VALUE;
	unordered_map<uint32_t, FileIndex> mapNeedMatch;

	double lng = (rtWorld.Xmin + rtWorld.Xmax) / 2;
	double lat = (rtWorld.Ymin + rtWorld.Ymax) / 2;

	// 요청 영역의 중심에 매칭되는 하나의 메쉬를 찿고, 찾은 매쉬의 주변 메쉬를 돌아가며 확인
	for (const auto& idx : m_mapFileIndex) {
		if ((idx.second.szBody >= 0) && isInBox(lng, lat, idx.second.rtData)) {
			idMesh = idx.second.idTile;
			if (idx.second.idTile == GLOBAL_MESH_ID) {
				continue;
			}
			break;
		}
	} // for

	// 중심 좌표에 매칭되는 메쉬가 없으면 영역에 포함되는 메쉬 검색
	if (idMesh == NULL_VALUE) {
		for (const auto& idx : m_mapFileIndex) {
			if ((idx.second.szBody >= 0) && isInPitBox(idx.second.rtData, rtWorld)) {
				idMesh = idx.first;
				break;
			}
		}
	}
	
	if ((idMesh != NULL_VALUE) && (FindNearMesh(rtWorld, m_mapFileIndex, idMesh, mapNeedMatch) == true)) {
		return CalculateCache(mapNeedMatch);
	}

	return false;
}


bool CDataManager::GetNewData(IN const double& lng, IN const double& lat)
{
	unordered_map<uint32_t, FileIndex> mapNeedMatch;
	unordered_map<uint32_t, FileIndex>::const_iterator it;

	for (const auto& idx : m_mapFileIndex) {
		if (isInBox(lng, lat, idx.second.rtData)) {
			mapNeedMatch.emplace(idx.second.idTile, idx.second);

			if (idx.second.idTile == GLOBAL_MESH_ID) {
				continue;
			} else {
				// id 매칭, 로딩되야 할 메쉬
				stMeshInfo* pMesh = m_pMapMesh->GetMeshById(idx.second.idTile);
				if (pMesh != nullptr) {
					for (const auto& mesh : pMesh->neighbors) {
						if ((it = m_mapFileIndex.find(mesh)) != m_mapFileIndex.end()) {
							mapNeedMatch.emplace(it->second.idTile, it->second);
						}
					} // for
				}
				break;
			}
		}
	} // for

	return CalculateCache(mapNeedMatch);
}


bool CDataManager::LoadStaticData(IN const char* pszFilePath)
{
	bool ret = false;

	LOG_TRACE(LOG_DEBUG, "Read data, iNavi static traffic data , path : %s", pszFilePath);

#if defined(USE_INAVI_STATIC_DATA)
	ret = m_pStaticMgr->LoadData(pszFilePath);
#else
	ret = true;
#endif

	return ret;
}


void CDataManager::SetFileMgr(IN CFileManager* pFileMgr)
{
	m_pFileMgr = pFileMgr;
}


void CDataManager::SetDataPath(IN const char* pszDataPath)
{
	strcpy(m_szDataPath, pszDataPath);

	// check dir
	char szCheckPath[MAX_PATH] = {0, };
	sprintf(szCheckPath, "%s/usr", pszDataPath);
	checkDirectory(szCheckPath);
}


const char* CDataManager::CDataManager::GetDataPath(void) const
{
	return m_szDataPath;
}


int getRequestCost(IN const char* szRequest, IN const char* szType, OUT DataCost* pDataCost)
{
	if (szRequest == nullptr || szType == nullptr || pDataCost == nullptr) {
		return 0;
	}

	// json
	int cntValue = 0;

#if defined(USE_CJSON)
	cJSON* root = cJSON_Parse(szRequest);
	if (root != NULL) {
		string strType;
		cJSON* pData = cJSON_GetObjectItem(root, "type");
		if (pData) {
			strType = cJSON_GetStringValue(pData);
		}

		if ((strcmp(szType, strType.c_str()) == 0) && (strcmp(szType, "pedestrian") == 0)) {
			cJSON* pData = cJSON_GetObjectItem(root, "cost");
			cJSON* pArray = nullptr;
			cJSON* pItem = nullptr;
			cJSON* pCost = nullptr;
			const int nMaxSize = ROUTE_OPT_COUNT;

			// forest cost
			if (pData) {
				pItem = cJSON_GetObjectItem(pData, "forest");
				if (pItem != NULL) {
					// 기본
					pArray = cJSON_GetObjectItem(pItem, "base");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_forest_base[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// 인기도	
					pArray = cJSON_GetObjectItem(pItem, "popular");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_forest_popular[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// 코스	
					pArray = cJSON_GetObjectItem(pItem, "course");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_forest_course[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// 경사도
					pArray = cJSON_GetObjectItem(pItem, "slop");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_forest_slop[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}
				}

				// lane cost
				pItem = cJSON_GetObjectItem(pData, "lane");
				if (pItem != NULL) {
					// 걷기	
					pArray = cJSON_GetObjectItem(pItem, "walk");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_lane_walk[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// 자전거			
					pArray = cJSON_GetObjectItem(pItem, "bike");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_lane_bike[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}
				}


				// angle cost
				pItem = cJSON_GetObjectItem(pData, "angle");
				if (pItem != NULL) {
					// 걷기					
					pArray = cJSON_GetObjectItem(pItem, "walk");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_angle_walk[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// 자전거	
					pArray = cJSON_GetObjectItem(pItem, "bike");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_angle_bike[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}
				}

				// walk cost
				pItem = cJSON_GetObjectItem(pData, "walking");
				if (pItem != NULL) {
					// TWIN ROAD // 복선
					pArray = cJSON_GetObjectItem(pItem, "side");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_walk_side[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// WALK WITH CAR 차량겸용
					pArray = cJSON_GetObjectItem(pItem, "with");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_walk_with[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// BICYCLE ONLY 자전거전용					
					pArray = cJSON_GetObjectItem(pItem, "bike");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_walk_bike[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// PEDESTRIAN 보행전용					
					pArray = cJSON_GetObjectItem(pItem, "only");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_walk_only[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// VIRTUAL 가상보행					
					pArray = cJSON_GetObjectItem(pItem, "line");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_walk_line[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}
				}

				// bicycle cost
				pItem = cJSON_GetObjectItem(pData, "bicycle");
				if (pItem != NULL) {
					// BICYCLE ONLY					
					pArray = cJSON_GetObjectItem(pItem, "bike");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_bike_only[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// BICYCLE WITH CAR					
					pArray = cJSON_GetObjectItem(pItem, "with");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_bike_with[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// PEDESTRIAN
					pArray = cJSON_GetObjectItem(pItem, "walk");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->pedestrian.cost_bike_walk[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}
				}


				// facility type cost
				pItem = cJSON_GetObjectItem(pData, "facility");
				if (pItem != NULL) {
					// facility
					static const char facility_walk_key[TYPE_OBJECT_COUNT][128] = {
						"facility_none", "walk_cave", "walk_underpass", "walk_footbridge", "walk_overpass", 
						"walk_bridge", "walk_subway", "walk_railorad", "walk_busstation", "walk_undergroundmall",
						"walk_throughbuilding", "walk_complexpark", "walk_complexapt", "walk_complextour", "walk_complexetc",
					};

					static const char facility_bike_key[TYPE_OBJECT_COUNT][128] = {
						"facility_none", "bike_cave", "bike_underpass", "bike_footbridge", "bike_overpass", 
						"bike_bridge", "bike_subway", "bike_railorad", "bike_busstation", "bike_undergroundmall", 
						"bike_throughbuilding", "bike_complexpark", "bike_complexapt", "bike_complextour", "bike_complexetc",	
					};

					// walk
					for (int ii = 0; ii < TYPE_OBJECT_COUNT; ii++) {
						pArray = cJSON_GetObjectItem(pItem, facility_walk_key[ii]);
						if (pArray && cJSON_IsArray(pArray)) { // new
							int nSize = cJSON_GetArraySize(pArray);
							for (int jj = 0; jj < min(nSize, nMaxSize); jj++) {
								pCost = cJSON_GetArrayItem(pArray, jj);
								if (pCost != nullptr) {
									pDataCost->pedestrian.cost_facility_walk[ii][jj] = cJSON_GetNumberValue(pCost);
									cntValue++;
								}
							} // for jj
						}
					} // for ii

					// bike
					for (int ii = 0; ii < TYPE_OBJECT_COUNT; ii++) {
						pArray = cJSON_GetObjectItem(pItem, facility_bike_key[ii]);
						if (pArray && cJSON_IsArray(pArray)) { // new
							int nSize = cJSON_GetArraySize(pArray);
							for (int jj = 0; jj < min(nSize, nMaxSize); jj++) {
								pCost = cJSON_GetArrayItem(pArray, jj);
								if (pCost != nullptr) {
									pDataCost->pedestrian.cost_facility_bike[ii][jj] = cJSON_GetNumberValue(pCost);
									cntValue++;
								}
							} // for jj
						}
					} // for ii
				}


				// gate type cost
				pItem = cJSON_GetObjectItem(pData, "gate");
				if (pItem != NULL) {
					// gate
					static const char gate_walk_key[TYPE_GATE_COUNT][128] = {
						"gate_none", "walk_slop", "walk_stairs", "walk_elcalator", "walk_stairescalator", 
						"walk_elevator", "walk_connection", "walk_crosswalk", "walk_movingwalk", "walk_steppingstones", 
						"walk_line",
					};

					static const char gate_bike_key[TYPE_GATE_COUNT][128] = {
						"gate_none", "bike_slop", "bike_stairs", "bike_elcalator", "bike_stairescalator",
						"bike_elevator", "bike_connection", "bike_crosswalk", "bike_movingwalk", "bike_steppingstones"
						, "bike_line",
					};

					// walk
					for (int ii = 0; ii < TYPE_GATE_COUNT; ii++) {
						pArray = cJSON_GetObjectItem(pItem, gate_walk_key[ii]);
						if (pArray && cJSON_IsArray(pArray)) { // new
							int nSize = cJSON_GetArraySize(pArray);
							for (int jj = 0; jj < min(nSize, nMaxSize); jj++) {
								pCost = cJSON_GetArrayItem(pArray, jj);
								if (pCost != nullptr) {
									pDataCost->pedestrian.cost_gate_walk[ii][jj] = cJSON_GetNumberValue(pCost);
									cntValue++;
								}
							} // for jj
						}
					} // for ii

					// bike
					for (int ii = 0; ii < TYPE_GATE_COUNT; ii++) {
						pArray = cJSON_GetObjectItem(pItem, gate_bike_key[ii]);
						if (pArray && cJSON_IsArray(pArray)) { // new
							int nSize = cJSON_GetArraySize(pArray);
							for (int jj = 0; jj < min(nSize, nMaxSize); jj++) {
								pCost = cJSON_GetArrayItem(pArray, jj);
								if (pCost != nullptr) {
									pDataCost->pedestrian.cost_gate_bike[ii][jj] = cJSON_GetNumberValue(pCost);
									cntValue++;
								}
							} // for jj
						}
					} // for ii
				}

			}
		} // pedestrian
		else if ((strcmp(szType, strType.c_str()) == 0)) {
			//&& ((strcmp(szType, "vehicle") == 0) || (strcmp(szType, "tms") == 0))) {
			cJSON* pData = cJSON_GetObjectItem(root, "cost");
			cJSON* pArray = nullptr;
			cJSON* pItem = nullptr;
			cJSON* pCost = nullptr;
			const int nMaxSize = 10;

			if (pData) {
				// level cost
				pItem = cJSON_GetObjectItem(pData, "speed");
				if (pItem != NULL) {
					// level0
					pArray = cJSON_GetObjectItem(pItem, "lv0");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_speed_lv0[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// level1
					pArray = cJSON_GetObjectItem(pItem, "lv1");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_speed_lv1[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// level2
					pArray = cJSON_GetObjectItem(pItem, "lv2");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_speed_lv2[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// level3
					pArray = cJSON_GetObjectItem(pItem, "lv3");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_speed_lv3[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// level4
					pArray = cJSON_GetObjectItem(pItem, "lv4");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_speed_lv4[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// level5
					pArray = cJSON_GetObjectItem(pItem, "lv5");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_speed_lv5[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// level6
					pArray = cJSON_GetObjectItem(pItem, "lv6");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_speed_lv6[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// level7
					pArray = cJSON_GetObjectItem(pItem, "lv7");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_speed_lv7[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// level8
					pArray = cJSON_GetObjectItem(pItem, "lv8");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_speed_lv8[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// level9
					pArray = cJSON_GetObjectItem(pItem, "lv9");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_speed_lv9[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}
				}

				// angle cost
				pItem = cJSON_GetObjectItem(pData, "time");
				if (pItem != NULL) {
					// angle 0
					pArray = cJSON_GetObjectItem(pItem, "ang0");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_time_ang0[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					// recomended
					pArray = cJSON_GetObjectItem(pItem, "ang45");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_time_ang45[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					pArray = cJSON_GetObjectItem(pItem, "ang90");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_time_ang90[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					pArray = cJSON_GetObjectItem(pItem, "ang135");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_time_ang135[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					pArray = cJSON_GetObjectItem(pItem, "ang180");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_time_ang180[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					pArray = cJSON_GetObjectItem(pItem, "ang225");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_time_ang225[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					pArray = cJSON_GetObjectItem(pItem, "ang270");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_time_ang270[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}

					pArray = cJSON_GetObjectItem(pItem, "ang315");
					if (pArray && cJSON_IsArray(pArray)) { // new
						int nSize = cJSON_GetArraySize(pArray);
						for (int ii = 0; ii < min(nSize, nMaxSize); ii++) {
							pCost = cJSON_GetArrayItem(pArray, ii);
							if (pCost != nullptr) {
								pDataCost->vehicle.cost_time_ang315[ii] = cJSON_GetNumberValue(pCost);
								cntValue++;
							}
						}
					}
				}
			}
		} // vehicle

		cJSON_Delete(root);
	} // root
#endif //#if defined(USE_CJSON)

	return cntValue;
}


int CDataManager::GetDataCost(IN const uint32_t type, OUT DataCost& dataCost)
{
	int retCntCost = 0;

	// cost data file
	char szCostFile[FILENAME_MAX + 1];
	char szTarget[FILENAME_MAX + 1] = { 0, };
	if (type == TYPE_DATA_PEDESTRIAN) {
		sprintf(szTarget, "pedestrian");
	} else if (type == TYPE_DATA_TREKKING) {
		sprintf(szTarget, "forest");
	} else {
#if defined(USE_TMS_API)
#	if defined(DEMO_FOR_HANJIN)
		sprintf(szTarget, "hanjin");
#	else
		sprintf(szTarget, "tms");
#	endif
#else
		sprintf(szTarget, "vehicle");
#endif
	}

#if defined(_WIN32) && defined(_WINDOWS)
	sprintf_s(szCostFile, FILENAME_MAX, "%s/usr/%s_cost.json", m_szDataPath, szTarget);
#else
	snprintf(szCostFile, FILENAME_MAX, "%s/usr/%s_cost.json", m_szDataPath, szTarget);
#endif
	FILE* fp = fopen(szCostFile, "rb");
	if (fp) {
		static const int MAX_DATACOST_SIZE = 1024 * 128; //128 Kib
		fseek(fp, 0L, SEEK_END);
		size_t nFileSize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		if (!nFileSize || (nFileSize > MAX_DATACOST_SIZE)) {
			LOG_TRACE(LOG_WARNING, "data cost has wrong file size, file:%s, size:%s", szCostFile, nFileSize);
		} else {
			string strJson;
			const size_t nBuff = 1024;
			char szBuff[nBuff + 1] = { 0, };
#if defined(_WIN32)
			while (fread_s(szBuff, nBuff, nBuff, 1, fp) != 0) {
#else
			while (fread(szBuff, nBuff, 1, fp) != 0) {
#endif
				strJson.append(szBuff);
				memset(szBuff, 0x00, nBuff);
			}
			strJson.append(szBuff);

			retCntCost = getRequestCost(strJson.c_str(), szTarget, &dataCost);
			//if (retCntCost > 0) {
			//	m_pRouteMgr.SetRouteCost(type, &dataCost, cntCost);
			//}
		}
		fclose(fp);
	}

	return retCntCost;
}


void CDataManager::SetDataCost(IN const uint32_t type, IN const DataCost* pCost)
{
	if (pCost != nullptr) {
		int32_t listCount = 18;

		LOG_TRACE(LOG_DEBUG, "set optimal type:%d", type);

		switch (type) {
		case TYPE_DATA_BUILDING:
		case TYPE_DATA_COMPLEX:
		case TYPE_DATA_ENTRANCE:
		{
			// old 
			LOG_TRACE(LOG_DEBUG_LINE, "set optimal cost(old), ");
			LOG_TRACE(LOG_DEBUG_CONTINUE, "dist step : %d, %d, %d, %d, %d, ", m_dataCost.optimal.cost_lv[0],
				m_dataCost.optimal.cost_lv[1], m_dataCost.optimal.cost_lv[2], m_dataCost.optimal.cost_lv[3], m_dataCost.optimal.cost_lv[4]);
			LOG_TRACE(LOG_DEBUG_CONTINUE, " max bulding & complex : ");
			LOG_TRACE(LOG_DEBUG_CONTINUE, "%d, %d", m_dataCost.optimal.cost_bld, m_dataCost.optimal.cost_cpx);
			LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");

			memcpy(&m_dataCost, pCost, sizeof(m_dataCost));			

			// new
			LOG_TRACE(LOG_DEBUG_LINE, "set optimal arr cost(new), ");
			LOG_TRACE(LOG_DEBUG_CONTINUE, "dist step : %d, %d, %d, %d, %d, ", m_dataCost.optimal.cost_lv[0],
				m_dataCost.optimal.cost_lv[1], m_dataCost.optimal.cost_lv[2], m_dataCost.optimal.cost_lv[3], m_dataCost.optimal.cost_lv[4]);
			LOG_TRACE(LOG_DEBUG_CONTINUE, " max bulding & complex : ");
			LOG_TRACE(LOG_DEBUG_CONTINUE, "%d, %d", m_dataCost.optimal.cost_bld, m_dataCost.optimal.cost_cpx);
			LOG_TRACE(LOG_DEBUG_CONTINUE, "\n");
		}
		break;

		default:
		{
			LOG_TRACE(LOG_DEBUG, "default data type not defined");
		}
		break;
		} //switch
	}
}


bool CDataManager::AddIndexData(IN const FileIndex& pData)
{
	m_mapFileIndex.emplace(pData.idTile, pData);

	return true;
}


// cache
uint32_t CDataManager::SetCacheCount(IN const uint32_t cntCache)
{
	if (0 <= cntCache && cntCache <= MAX_MESH_COUNT) {
		m_cntMaxCache = cntCache;
	}

	return m_cntMaxCache;
}


int CDataManager::RemoveCacheItem(IN const int cntItem)
{
	if (m_mapCache.empty()) {
		return 0;
	}

	int nMaxPoint = INT_MIN;
	//int nCacheKey = -1;
	int cntShouldRemovItems = cntItem;

	if (cntItem < 0) {
		// 전체 삭제
		cntShouldRemovItems = m_mapCache.size();
	}
	else if (cntItem <= 0) {
		cntShouldRemovItems = 1;
	}

	priority_queue<FileCacheData*> pqCandidate;
	map<uint32_t, FileCacheData>::iterator itCache;

	// 매칭된 캐쉬가 부족하면 캐쉬에서 삭제
	for (itCache = m_mapCache.begin(); itCache != m_mapCache.end(); itCache++)
	{
		// 매칭안된 녀석중 제일 오래된 녀석 검색
		if (itCache->second.cntMatchingPoint >= nMaxPoint)
		{
			if (cntItem >= 0) {
				// 부분 삭제
				nMaxPoint = itCache->second.cntMatchingPoint;
			}
			//nCacheKey = itCache->first;

			pqCandidate.push(&itCache->second);
		}
	}

	for (int ii = 0; ii < cntShouldRemovItems || !pqCandidate.empty(); ii++) {
		FileCacheData* pItem = pqCandidate.top();
		pqCandidate.pop();

		itCache = m_mapCache.find(pItem->nTileId);
		if (itCache != m_mapCache.end())
		{
			if (!DeleteData(itCache->second.nTileId))
			{
				LOG_TRACE(LOG_ERROR, "Failed, can't delete mesh id, mesh_id:%d", itCache->second.nTileId);
			}

			try {
				m_mapCache.erase(itCache++);
			}
			catch (exception e) {
				LOG_TRACE(LOG_ERROR, "Error, delete cache, %s", e.what());
			}
		}
	}

	return m_mapCache.size();
}


int CDataManager::CalculateCache(unordered_map<uint32_t, FileIndex>& mapNeedMatch)
{
	if (m_mapFileIndex.empty()) {
		return 0;
	}
	else if (mapNeedMatch.empty()) {
		return 0;
	}

	// 매회 캐슁 키 갱신
	static int matchKey = 0;
	matchKey++;
	if (matchKey > INT_MAX) {
		matchKey = 0;
	}

	map<uint32_t, FileCacheData>::iterator itCache;
	unordered_map<uint32_t, FileIndex>::iterator itNeed;
	int32_t cntNeedMatch = mapNeedMatch.size();
	int32_t cntCached = m_mapCache.size();
	int32_t cntRemaindCache = m_cntMaxCache - cntCached;
	int32_t cntMatched = 0;

	// 현재 캐쉬 데이터의 매칭 검사
	priority_queue<FileCacheData*> pqRemoveCandidate;
	for (itCache = m_mapCache.begin(); itCache != m_mapCache.end(); itCache++)
	{
		// 이미 존재하는 아이템은 무시
		if ((itNeed = mapNeedMatch.find(itCache->second.nTileId)) != mapNeedMatch.end())
		{
			itCache->second.nMatchKey = matchKey;
			itCache->second.cntMatchingPoint--;

			mapNeedMatch.erase(itNeed);
			cntNeedMatch--;
			cntMatched++;
		}
		else {
			// 삭제 가능 목록 생성
			pqRemoveCandidate.emplace(&itCache->second);
		}
	}


	// 매칭되는 아이템이 최대 캐쉬를 넘었으면 그냥 돌아가자
	if (cntNeedMatch <= 0 || m_cntMaxCache <= cntMatched)
	{
		return m_cntMaxCache;
	}


	// 남은 캐쉬가 필요 캐쉬보다 적으면 포인트 작은 캐쉬 데이터 삭제
	//for (; !pqRemoveCandidate.empty(); )
	for (; cntRemaindCache < cntNeedMatch && !pqRemoveCandidate.empty(); )
	{
		// 이미 매칭된 캐쉬와 남은 캐쉬가 최대 캐쉬를 넘지는 못하도록
		if (m_cntMaxCache <= cntRemaindCache + cntMatched) {
			break;
		}

		FileCacheData* pItem = pqRemoveCandidate.top(); pqRemoveCandidate.pop();
		if (pItem != nullptr) {
			itCache = m_mapCache.find(pItem->nTileId);
			if (itCache != m_mapCache.end())
			{
				if (!DeleteData(itCache->second.nTileId))
				{
					LOG_TRACE(LOG_ERROR, "Failed, can't delete mesh, id:%d", itCache->second.nTileId);
				}

				try {
					m_mapCache.erase(itCache++);
					cntRemaindCache++;
					cntCached--;
				}
				catch (exception e) {
					LOG_TRACE(LOG_ERROR, "Error, delete cache, %s", e.what());
				}
			}
		}
	} // for

	
	// 캐쉬 추가
	for (itNeed = mapNeedMatch.begin(); itNeed != mapNeedMatch.end(); itNeed++)
	{
		if (cntRemaindCache <= 0 || m_cntMaxCache <= cntCached) {
			break;
		}

#if 1//defined(_DEBUG)
		size_t sizeMem = checkMemorySize();
		time_t tickMesh = LOG_TRACE(LOG_TEST, "");// , "Before, Cache memory usages : %u", sizeMem);
#endif

		if (!checkTestMesh(itNeed->first)) {
			continue;
		}

		if (!m_pFileMgr->GetData(itNeed->second.idxTile)) {
			LOG_TRACE(LOG_ERROR, "Failed, can't load mesh, id:%d", itNeed);
			continue;
		}

		FileCacheData newCache;
		newCache.nTileId = itNeed->second.idTile;
		newCache.nMatchKey = matchKey;
		newCache.cntMatchingPoint = INT_MAX;

		m_mapCache.emplace(itNeed->second.idTile, newCache);
		cntCached++;
		cntRemaindCache--;

#if 1//defined(_DEBUG)
		double sizeGap = checkMemorySize() - sizeMem;
		if (sizeGap != 0) {
			LOG_TRACE(LOG_DEBUG, tickMesh, "Cache memory usages : %s", getSizeString(sizeGap));
		}
#endif
	} // for

	return cntCached;
}


// for spatial index
#ifdef TEST_SPATIALINDEX
int32_t CDataManager::CreateSpatialindex(IN const int32_t nDataType)
{
	int ret = 0;

	vector<SpatialIndex::Region> regions;
	vector<id_type> ids;
	int cnt = 0;

	int cntMesh = GetMeshCount();
	for (int ii = 0; ii < cntMesh; ii++) {
		stMeshInfo* pMesh = GetMeshData(ii);
		if (pMesh) {
			CreateSpatialindex(pMesh, nDataType);
		}
	}

	return ret;
}


int32_t CDataManager::CreateSpatialindex(IN stMeshInfo* pMesh, IN const int32_t nDataType)
{
	int ret = 0;

#if defined(USE_SPATIALINDEX_BULK)
	vector<SpatialIndex::Region> regions;
	vector<id_type> ids;
	int cnt = 0;
	stLinkInfo* pLink = nullptr;

	if (nDataType == TYPE_DATA_TREKKING) { 	// for forest
#if defined(USE_FOREST_DATA)
		cnt = pMesh->setFLinkDuplicateCheck.size();
		if (cnt > 0) {
			regions.reserve(cnt); ids.reserve(cnt);
			for (const auto& key : pMesh->setFLinkDuplicateCheck) {
				pLink = GetFLinkDataById(key);
				if (pLink) {
					SBox boxArea;
					boxArea.Xmin = boxArea.Ymin = DBL_MAX;
					boxArea.Xmax = boxArea.Ymax = DBL_MIN;
					extendDataBox(boxArea, pLink->getVertex(), pLink->getVertexCount());

					double low[2] = { boxArea.Xmin, boxArea.Ymin };
					double high[2] = { boxArea.Xmax, boxArea.Ymax };
					Region region(low, high, 2);

					regions.emplace_back(region);
					ids.emplace_back(pLink->link_id.llid);
				}
			}

			if (regions.size() < 50) {
				m_pMapMesh->CreateSpatialindex(TYPE_DATA_TREKKING, pMesh, regions, ids);
			} else {
				m_pMapMesh->CreateSpatialindexByStream(TYPE_DATA_TREKKING, pMesh, regions, ids);
			}
		}
#endif
	} else if (nDataType == TYPE_DATA_PEDESTRIAN) { // for pedestrian
#if defined(USE_PEDESTRIAN_DATA)
		cnt = pMesh->setWLinkDuplicateCheck.size();
		if (cnt > 0) {
			regions.reserve(cnt); ids.reserve(cnt);
			for (const auto& key : pMesh->setWLinkDuplicateCheck) {
				pLink = GetWLinkDataById(key);
				if (pLink) {
					SBox boxArea;
					boxArea.Xmin = boxArea.Ymin = DBL_MAX;
					boxArea.Xmax = boxArea.Ymax = DBL_MIN;
					extendDataBox(boxArea, pLink->getVertex(), pLink->getVertexCount());

					double low[2] = { boxArea.Xmin, boxArea.Ymin };
					double high[2] = { boxArea.Xmax, boxArea.Ymax };
					Region region(low, high, 2);

					regions.emplace_back(region);
					ids.emplace_back(pLink->link_id.llid);
				}
			}

			if (_regions.size() < 50) {
				m_pMapMesh->CreateSpatialindex(TYPE_DATA_PEDESTRIAN, pMesh, regions, ids);
			} else {
				m_pMapMesh->CreateSpatialindexByStream(TYPE_DATA_PEDESTRIAN, pMesh, regions, ids);
			}
		}
#endif
	} else if (nDataType == TYPE_DATA_VEHICLE) {  // for vlink
#if defined(USE_VEHICLE_DATA)
		cnt = pMesh->setVLinkDuplicateCheck.size();
		if (cnt > 0) {
			regions.reserve(cnt); ids.reserve(cnt);
			for (const auto& key : pMesh->setVLinkDuplicateCheck) {
				pLink = GetVLinkDataById(key);
				if (pLink) {
					SBox boxArea;
					boxArea.Xmin = boxArea.Ymin = DBL_MAX;
					boxArea.Xmax = boxArea.Ymax = DBL_MIN;
					extendDataBox(boxArea, pLink->getVertex(), pLink->getVertexCount());

					double low[2] = { boxArea.Xmin, boxArea.Ymin };
					double high[2] = { boxArea.Xmax, boxArea.Ymax };
					Region region(low, high, 2);

					regions.emplace_back(region);
					ids.emplace_back(pLink->link_id.llid);
				}
			}

			if (regions.size() < 50) {
				m_pMapMesh->CreateSpatialindex(TYPE_DATA_VEHICLE, pMesh, regions, ids);
			} else {
				m_pMapMesh->CreateSpatialindexByStream(TYPE_DATA_VEHICLE, pMesh, regions, ids);
			}
		}
#endif
	}
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA) 	
	else if (nDataType == TYPE_DATA_COMPLEX) { // for cpx
		cnt = pMesh->setCpxDuplicateCheck.size();
		if (cnt > 0) {
			stPolygonInfo* pPoly;
			regions.clear(); ids.clear();
			regions.reserve(cnt); ids.reserve(cnt);
			for (const auto& key : pMesh->setCpxDuplicateCheck) {
				pPoly = GetComplexDataById(key);
				if (pPoly) {
					double low[2] = { pPoly->data_box.Xmin, pPoly->data_box.Ymin };
					double high[2] = { pPoly->data_box.Xmax, pPoly->data_box.Ymax };
					Region region(low, high, 2);

					regions.emplace_back(region);
					ids.emplace_back(pPoly->poly_id.llid);
				}
			}
			m_pMapMesh->CreateSpatialindexByStream(TYPE_DATA_COMPLEX, pMesh, regions, ids);
		}
	} else if (nDataType == TYPE_DATA_BUILDING) { // for bld
		cnt = pMesh->setBldDuplicateCheck.size();
		if (cnt > 0) {
			stPolygonInfo* pPoly;
			regions.clear(); ids.clear();
			regions.reserve(cnt); ids.reserve(cnt);
			for (const auto& key : pMesh->setBldDuplicateCheck) {
				pPoly = GetBuildingDataById(key);
				if (pPoly) {
					double low[2] = { pPoly->data_box.Xmin, pPoly->data_box.Ymin };
					double high[2] = { pPoly->data_box.Xmax, pPoly->data_box.Ymax };
					Region region(low, high, 2);

					regions.emplace_back(region);
					ids.emplace_back(pPoly->poly_id.llid);
				}
			}
			m_pMapMesh->CreateSpatialindexByStream(TYPE_DATA_BUILDING, pMesh, regions, ids);
		}
	}
#endif
#	else // USE_SPATIALINDEX_BULK
	CreateSpatialindex(pMesh);
#	endif // USE_SPATIALINDEX_BULK

	return ret;
}


int32_t CDataManager::InsertSpatialindex(IN stMeshInfo* pMesh, IN const int32_t nDataType)
{
	int32_t ret = 0;

	return ret;
}
#endif // #ifdef TEST_SPATIALINDEX