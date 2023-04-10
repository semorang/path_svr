#if defined(_WIN32)
#include "../stdafx.h"
#endif


#include "DataManager.h"

#include "../shp/shpio.h"
#include "../utils/UserLog.h"
#include "../utils/GeoTools.h"
#include "MMPoint.hpp"


#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDataManager::CDataManager()
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	m_pMapName = nullptr;
	m_pMapMesh = nullptr;
	m_pMapNode = nullptr;
	m_pMapLink = nullptr;
	m_pMapWNode = nullptr;
	m_pMapWLink = nullptr;
	m_pMapVNode = nullptr;
	m_pMapVLink = nullptr;
	m_pMapComplex = nullptr;
	m_pMapBuilding = nullptr;

	m_pMapName = new MapName();
	m_pMapMesh = new MapMesh();
#if defined(USE_TREKKING_DATA)
	m_pMapNode = new MapNode();
	m_pMapLink = new MapLink();
#endif
#if defined(USE_PEDESTRIAN_DATA)
	m_pMapWNode = new MapNode();
	m_pMapWLink = new MapLink();
#endif
#if defined(USE_VEHICLE_DATA)
	m_pMapVNode = new MapNode();
	m_pMapVLink = new MapLink();
#endif
#if defined(USE_COMPLEX_DATA)
	m_pMapComplex = new MapPolygon();
#endif
#if defined(USE_BUILDING_DATA)
	m_pMapBuilding = new MapPolygon();
#endif

//#if defined (USE_PEDESTRIAN_DATA)
//	m_trackShpMgr.SetFileManager(this);
//#endif
}

CDataManager::~CDataManager()
{
	Release();

	if (m_pMapName) {
		delete m_pMapName;
	}
	if (m_pMapMesh) {
		delete m_pMapMesh;
	}
	if (m_pMapNode) {
		delete m_pMapNode;
	}
	if (m_pMapLink) {
		delete m_pMapLink;
	}
	if (m_pMapWNode) {
		delete m_pMapWNode;
	}
	if (m_pMapWLink) {
		delete m_pMapWLink;
	}
	if (m_pMapVNode) {
		delete m_pMapVNode;
	}
	if (m_pMapVLink) {
		delete m_pMapVLink;
	}
	if (m_pMapComplex) {
		delete m_pMapComplex;
	}
	if (m_pMapBuilding) {
		delete m_pMapBuilding;
	}

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
	if (m_pMapNode) {
		m_pMapNode->Initialize();
	}
	if (m_pMapLink) {
		m_pMapLink->Initialize();
	}
	if (m_pMapWNode) {
		m_pMapWNode->Initialize();
	}
	if (m_pMapWLink) {
		m_pMapWLink->Initialize();
	}
	if (m_pMapVNode) {
		m_pMapVNode->Initialize();
	}
	if (m_pMapVLink) {
		m_pMapVLink->Initialize();
	}
	if (m_pMapComplex) {
		m_pMapComplex->Initialize();
	}
	if (m_pMapBuilding) {
		m_pMapBuilding->Initialize();
	}

	m_rtBox.Xmin = INT_MAX;
	m_rtBox.Ymin = INT_MAX;
	m_rtBox.Xmax = INT_MIN;
	m_rtBox.Ymax = INT_MIN;

#if defined(USE_DATA_CACHE)
	m_cntMaxCache = MAX_CASH_COUNT;
#endif

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
	if (m_pMapNode) {
		m_pMapNode->Release();
	}
	if (m_pMapLink) {
		m_pMapLink->Release();
	}
	if (m_pMapWNode) {
		m_pMapWNode->Release();
	}
	if (m_pMapWLink) {
		m_pMapWLink->Release();
	}
	if (m_pMapVNode) {
		m_pMapVNode->Release();
	}
	if (m_pMapVLink) {
		m_pMapVLink->Release();
	}
	if (m_pMapComplex) {
		m_pMapComplex->Release();
	}
	if (m_pMapBuilding) {
		m_pMapBuilding->Release();
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


void CDataManager::SetNeighborMesh(void)
{
	m_pMapMesh->CheckNeighborMesh();
}


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


bool CDataManager::AddNodeData(IN const stNodeInfo * pData)
{
	if (m_pMapNode->AddData(pData))
	{
		return m_pMapMesh->InsertNode(pData->node_id);
	}

	return false;
}

bool CDataManager::DeleteData(IN const uint64_t id)
{
	bool ret = false;
	stMeshInfo* pMesh = nullptr;

	if (m_pMapMesh != nullptr && ((pMesh = m_pMapMesh->GetMeshById(id)) != nullptr)) {
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

#if defined(USE_PEDESTRIAN_DATA)
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
#if defined(USE_VEHICLE_DATA)
		// delete vnodes
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
# if defined(USE_OPTIMAL_POINT_API)
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

		// delete mesh
		ret = m_pMapMesh->DeleteData(id);
	}

	return ret;
}


bool CDataManager::AddLinkData(IN const stLinkInfo * pData)
{
	if (m_pMapLink->AddData(pData))
	{
		return m_pMapMesh->InsertLink(pData);
	}

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
# if defined(USE_OPTIMAL_POINT_API)
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

# if defined(USE_OPTIMAL_POINT_API)
	if (pData) {
		stPolygonInfo* pPoly = nullptr;
		// 0:빌딩, 1:단지
		if (pData->poly_type == TYPE_ENT_BUILDING) {
			pPoly = m_pMapBuilding->GetDataById(keyId);				
		}
		else if (pData->poly_type == TYPE_ENT_COMPLEX) {
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


stNodeInfo * CDataManager::GetNodeDataById(IN const KeyID keyId, IN const bool force)
{
	stNodeInfo* pData = nullptr;

	if (m_pMapNode && !(pData = m_pMapNode->GetNodeById(keyId)) && force)
	{
		if (GetNewData(keyId.tile_id)) {
			pData = m_pMapNode->GetNodeById(keyId);
		}
	}

	return pData;
}


stLinkInfo * CDataManager::GetLinkDataById(IN const KeyID keyId, IN const bool force)
{
	stLinkInfo* pData = nullptr;

	if (m_pMapLink && !(pData = m_pMapLink->GetLinkById(keyId)) && force)
	{
		if (GetNewData(keyId.tile_id)) {
			pData = m_pMapLink->GetLinkById(keyId);
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
	stLinkInfo* pLinkBefore = GetLinkDataById(beforeLinkId);
	stLinkInfo* pLinkNext = GetLinkDataById(nextLinkId);

	stNodeInfo* pNode = nullptr;

	// get junction node
	// <--- --->, <--- <---
	if (((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(0)) && 
		(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(0))) ||
		((pLinkBefore->getVertexX(0) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) && 
		(pLinkBefore->getVertexY(0) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1)))) {
		pNode = GetNodeDataById(pLinkBefore->snode_id);
	}
	// ---> <--- , ---> --->
	else if (((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(pLinkNext->getVertexCount() - 1)) && 
			 (pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(pLinkNext->getVertexCount() - 1))) ||
			 ((pLinkBefore->getVertexX(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexX(0)) && 
			 (pLinkBefore->getVertexY(pLinkBefore->getVertexCount() - 1) == pLinkNext->getVertexY(0)))) {
		pNode = GetNodeDataById(pLinkBefore->enode_id);	
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

		pLinkInfo[cntBuff++] = GetLinkDataById(pNode->connnodes[ii]);
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
	if (m_pMapLink != nullptr) {
		pBase = m_pMapLink;
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
	if (m_pMapLink != nullptr) {
		pBase = m_pMapLink;
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
		snprintf(szVersion, sizeof(szVersion), "%d.%d.%d", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH);
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


stLinkInfo * CDataManager::GetLinkDataByPointAround(IN const double lng, IN const double lat, IN const int32_t nMaxDist, OUT double& retLng, OUT double& retLat, OUT double& retDist, IN const int32_t nMatchType, OUT int32_t* pMatchVtxIdx)
{
	int idxLinkVtx = -1;
	int idxLinkLine = -1;

	double retIr = 0.f;

	stMeshInfo* pMesh = nullptr;
	stLinkInfo* retLink = nullptr;

	static const uint32_t s_maxMesh = 9;
	static stMeshInfo* s_ppMesh[s_maxMesh];
	uint32_t nMaxMesh = 0;

#if defined(USE_DATA_CACHE)
	if (!m_mapFileIndex.empty() && (pMesh = GetMeshDataByPoint(lng, lat)) != nullptr) {
		s_ppMesh[nMaxMesh++] = pMesh;
	}
#else
	if (m_mapFileIndex.empty() && GetMeshCount() > 0)
	{
		// 파일 캐쉬가 아닌 데이터 버퍼 로딩 상태일 경우
		nMaxMesh = GetMeshDataByPoint(lng, lat, s_maxMesh, s_ppMesh);
	}

	// 주변 메쉬를 추가
	if (nMaxMesh == 1 && !s_ppMesh[0]->neighbors.empty()) {
		for (unordered_set<uint32_t>::const_iterator it = s_ppMesh[0]->neighbors.begin(); it != s_ppMesh[0]->neighbors.end(); it++) {
			// 주변 메쉬의 중심 거리가 최대 요청 거리 이하일 경우만 추가
			if ((pMesh = GetMeshDataById(*it)) != nullptr && isInBox(lng, lat, pMesh->data_box)) {
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
#endif

#pragma omp parallel for
	for (int32_t ii = 0; ii < nMaxMesh; ii++)
	{
		stMeshInfo* pMesh = s_ppMesh[ii];
		if (pMesh == nullptr) {
			continue;
		}

		stLinkInfo* pLink = nullptr;
		double minLng = 0.f;
		double minLat = 0.f;
		double minDist = nMaxDist;

#if defined(USE_TREKKING_DATA)
		if (/*minLink == nullptr && */!pMesh->links.empty())
		{
			for (int32_t jj = pMesh->links.size() - 1; jj >= 0; --jj)
			{
				if (!(pLink = GetLinkDataById(pMesh->links[jj])) ||
					((idxLinkVtx = linkProjection(pLink, lng, lat, nMaxDist, minLng, minLat, minDist, retIr)) < 0)) {
					continue;
				}

				if (minDist < retDist) {
					retLink = pLink;
					retLng = minLng;
					retLat = minLat;
					retDist = minDist;

					if (pMatchVtxIdx != nullptr) {
						*pMatchVtxIdx = idxLinkVtx;
					}
				}
			}
		}
#endif

#if defined(USE_PEDESTRIAN_DATA)
		if (/*minLink == nullptr && */!pMesh->wlinks.empty())
		{
			for (int32_t jj = pMesh->wlinks.size() - 1; jj >= 0; --jj)
			{
				if (!(pLink = GetWLinkDataById(pMesh->wlinks[jj])) ||
					((idxLinkVtx = linkProjection(pLink, lng, lat, nMaxDist, minLng, minLat, minDist, retIr)) < 0)) {
					continue;
				}

				if (minDist < retDist) {
					retLink = pLink;
					retLng = minLng;
					retLat = minLat;
					retDist = minDist;

					if (pMatchVtxIdx != nullptr) {
						*pMatchVtxIdx = idxLinkVtx;
					}
				}
			}
		}
#endif

#if defined(USE_VEHICLE_DATA)
		if (retLink != nullptr && retDist < 10) {
			// 현재 메쉬에서 10m이내 탐색 된 결과면 현재 메쉬에서 종료
			break;
		}
		else if (/*retLink == nullptr && */!pMesh->vlinks.empty())
		{
			for (int32_t jj = pMesh->vlinks.size() - 1; jj >= 0; --jj)
			{
				if (!(pLink = GetVLinkDataById(pMesh->vlinks[jj])) ||
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

					(nMatchType == TYPE_LINK_MATCH_CARSTOP) &&
					(pLink->veh.link_dtype == 1 || pLink->veh.link_dtype == 3 || //( pLink->veh.level >= 9 || 단지폴리곤에 포함된 단지내 도로만 제외
					 pLink->veh.road_type == 1 || pLink->veh.road_type == 2 || pLink->veh.road_type == 4 ||
					 pLink->veh.pass_code == 2 || pLink->veh.pass_code == 3 ||
					 pLink->veh.link_type == 3 || pLink->veh.link_type == 4 || pLink->veh.link_type == 5 || pLink->veh.link_type == 6 || pLink->veh.link_type == 8 ||
					 pLink->veh.tunnel == 1 || pLink->veh.under_pass == 1)) ||

					((nMatchType == TYPE_LINK_MATCH_CARSTOP_EX) &&
					(pLink->veh.link_dtype == 1 || // 차량 승하자 + 단지내도로(건물입구점 재확인시, 단지도로에 매칭된 입구점을 확인하기 위해 포함)
					 pLink->veh.road_type == 1 || pLink->veh.road_type == 2 || pLink->veh.road_type == 4 ||
					 pLink->veh.pass_code == 2 || pLink->veh.pass_code == 3 ||
					 pLink->veh.link_type == 3 || pLink->veh.link_type == 4 || pLink->veh.link_type == 5 || pLink->veh.link_type == 6 || pLink->veh.link_type == 8 ||
					 pLink->veh.tunnel == 1 || pLink->veh.under_pass == 1)) ||

					((nMatchType == TYPE_LINK_MATCH_FOR_TABLE) &&
					(pLink->veh.link_dtype == 1 || pLink->veh.link_dtype == 3 || pLink->veh.level >= USE_ROUTE_TABLE_LEVEL || // 2차선 이하 제외
					pLink->veh.road_type == 1 || pLink->veh.road_type == 2 || pLink->veh.road_type == 4 ||
					pLink->veh.pass_code == 2 || pLink->veh.pass_code == 3 ||
					pLink->veh.link_type == 3 || pLink->veh.link_type == 4 || pLink->veh.link_type == 5 || pLink->veh.link_type == 6 || pLink->veh.link_type == 8 ||
					pLink->veh.tunnel == 1 || pLink->veh.under_pass == 1)) ||

					((nMatchType == TYPE_LINK_MATCH_FOR_HD) &&
					(pLink->veh.link_dtype == 1 || /*pLink->veh.link_dtype == 3 || pLink->veh.level >= 9 ||*/
					// 대구 경북 과학 기술원내 도로가 단지내 도로이기에, p2p는 단지내 도로는 포함
					pLink->veh.road_type == 1 || pLink->veh.road_type == 2 || pLink->veh.road_type == 4 ||
					pLink->veh.pass_code == 2 || pLink->veh.pass_code == 3 ||
#if defined(USE_P2P_DATA)
					pLink->veh.over_pass == 1 || pLink->veh.hd_flag == 0 || // HD 링크와 매칭 안되는 링크는 제외
#endif
					pLink->veh.link_type == 3 || pLink->veh.link_type == 4 || pLink->veh.link_type == 5 || pLink->veh.link_type == 6 || pLink->veh.link_type == 8 ||
					pLink->veh.tunnel == 1 || pLink->veh.under_pass == 1)) ||
					
					((idxLinkVtx = linkProjection(pLink, lng, lat, nMaxDist, minLng, minLat, minDist, retIr)) < 0)) {
					continue;
				}

				if (minDist < retDist) {
					retLink = pLink;
					retLng = minLng;
					retLat = minLat;
					retDist = minDist;

					if (pMatchVtxIdx != nullptr) {
						*pMatchVtxIdx = idxLinkVtx;
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

#if defined(USE_TREKKING_DATA)
	pLink = GetWLinkDataById(linkId);
	if (pLink == nullptr) {
		pLink = GetLinkDataById(linkId);
	}
#elif defined(USE_PEDESTRIAN_DATA)
	pLink = GetWLinkDataById(linkId);
#elif defined(USE_VEHICLE_DATA)
	pLink = GetVLinkDataById(linkId);
#else
	pLink = GetLinkDataById(linkId);
#endif

	if (pLink) {
		idxVertex = (linkProjection(pLink, lng, lat, nMaxDist, minLng, minLat, minDist, retIr));
	}

	if (idxVertex >= 0) {
		retLng = minLng;
		retLat = minLat;
	}

	return idxVertex;
}


stPolygonInfo* CDataManager::GetPolygonDataByPoint(IN OUT double& lng, IN OUT double& lat, IN const int32_t nType)
{
	stPolygonInfo* pPoly = nullptr;
	stMeshInfo* pMesh = nullptr;

	if (m_pMapMesh && (pMesh = m_pMapMesh->GetMeshByPoint(lng, lat)) != nullptr) {
#if defined(USE_BUILDING_DATA)
		// find in building
		if (nType != 3) {
			for (vector<KeyID>::const_iterator it = pMesh->buildings.begin(); it != pMesh->buildings.end(); it++) {
				if (m_pMapBuilding && m_pMapBuilding->IsPitInPolygon(*it, lng, lat))
				{
					pPoly = m_pMapBuilding->GetDataById(*it);
					break;
				}
			}
		}
#endif
#if defined(USE_COMPLEX_DATA)
		// find in complex
		if (nType == 2 || (nType == 0 && pPoly == nullptr)) {
			for (vector<KeyID>::const_iterator it = pMesh->complexs.begin(); it != pMesh->complexs.end(); it++) {
				if (m_pMapComplex && m_pMapComplex->IsPitInPolygon(*it, lng, lat))
				{
					pPoly = m_pMapComplex->GetDataById(*it);
					break;
				}
			}
		}
#endif
	}

	return pPoly;
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
		if ((nEntType != 0 && nEntType != pEnt[ii].ent_code) || nEntType < 0 || 7 < nEntType) {
			continue;
		}

		dwDist = getRealWorldDistance(dwLng, dwLat, pEnt[ii].x, pEnt[ii].y);

		if (nMaxDist == 0 || dwDist <= nMaxDist) {
			stEntryPointInfo entInfo = { 0, };

			entInfo.nAttribute = pEnt[ii].ent_code;
			entInfo.x = pEnt[ii].x;
			entInfo.y = pEnt[ii].y;
			entInfo.dwDist = dwDist;

			vtEntInfo.emplace_back(entInfo);
			retCnt++;
		}
	} // for

	 if (retCnt > 1 && nSort == 1) {
		 sort(vtEntInfo.begin() + idxInfo, vtEntInfo.end(), cmpEntPoint);
	 }

	return retCnt;
}

bool CDataManager::GetNearRoadByPoint(IN const double lng, IN const double lat, IN const int32_t maxDist, IN const int32_t matchType, OUT stEntryPointInfo& entInfo) 
{
	bool ret = false;

	double retDist = INT_MAX;
	int32_t idxLinkVtx = -1;
	const SPoint reqPos = {lng, lat};
	SPoint retPos = {0,};
	
	static const double LaneLowDist = 3.0; // 60 미만 차선 기본 거리 미터
	static const double LaneMidDist = 3.25; // 60 이상 차선 기본 거리 미터
	static const double LaneHighDist = 3.5; // 80 이상 차선 기본 거리 미터
	
	stLinkInfo* pLink = GetLinkDataByPointAround(lng, lat, maxDist, retPos.x, retPos.y, retDist, matchType, &idxLinkVtx);
	if (pLink != nullptr) {
		double dwDist = getRealWorldDistance(lng, lat, retPos.x, retPos.y);

		// 도로에서 일정 거리 떨어진 지점을 주자 - 경탐시 방향성 적용이 원할하도록 하기 위함
		double dwLaneDist = pLink->veh.lane_cnt;
		if (pLink->veh.lane_cnt >= 6 || pLink->veh.level <= 1) {
			dwLaneDist = pLink->veh.lane_cnt * LaneHighDist;
		} else if (pLink->veh.lane_cnt <= 4 || pLink->veh.level <= 4) {
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
			if ((pLink->veh.pass_code == 5)) { // 정
				isLeft = isLeftSide(pLink->getVertex(idxLinkVtx), pLink->getVertex(idxLinkVtx + 1), &reqPos);
			}
			else { // 역
				isLeft = isLeftSide(pLink->getVertex(idxLinkVtx + 1), pLink->getVertex(idxLinkVtx), &reqPos);
			}

			// 무조건 도로 우측 방향으로 위치시켜 주자
			if (isLeft) {
				// 일방에서 좌측에 위치하면 우측 도로변으로 위치시켜 주자
				dwLaneDist += dwDist;

				if (getPointByDistance(lng, lat, newRetPos.x, newRetPos.y, dwLaneDist) == true) {
					entInfo.dwDist = dwLaneDist;
				}
				else {
					entInfo.dwDist = dwDist;
				}
			}
			else {
				// 도로로부터 지정된거리(도로너비) 만큼 띄워 도로변으로 위치시켜 주자
				if (getPointByDistance(lng, lat, newRetPos.x, newRetPos.y, dwDist - dwLaneDist) == true) {
					entInfo.dwDist = abs(dwDist - dwLaneDist);
				}
				else {
					entInfo.dwDist = dwDist;
				}
			}
		} else { // if (pLink->veh.link_type == 1) { // 비분리도로일경우 {
			dwLaneDist /= 2;

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

		// 새로운 지점 주변에 원래의 도로 말고, 또다른 가까운 도로가 있다면 원래의 도로에서 도로너비만큼 띄우지 말고, 최소의 너비만 띄운 좌표로 사용하자
		// 근접 링크 좌표에서 일정거리 띄워진 좌표
		SPoint nearRetPos = newRetPos;
		stLinkInfo* pNewLink = GetLinkDataByPointAround(newRetPos.x, newRetPos.y, dwLaneDist, nearRetPos.x, nearRetPos.y, retDist, matchType, &idxLinkVtx);

		if (pNewLink != nullptr && pNewLink != pLink) {
			// 도로 중심으로부터 지정된거리(도로너비) 만큼 띄워 도로밖으로 위치시켜 주자
			const double dwMinDist = 1.f;
			if (getPointByDistance(lng, lat, retPos.x, retPos.y, dwMinDist) == true) {
				entInfo.dwDist = abs(dwDist - dwMinDist);
			} else {
				entInfo.dwDist = dwDist;
			}

			entInfo.x = retPos.x;
			entInfo.y = retPos.y;
		} 
		else {
			entInfo.x = newRetPos.x;
			entInfo.y = newRetPos.y;
		}

		ret = true;
	} // if

	return ret;
}


int32_t CDataManager::GetOptimalPointDataByPoint(IN const double lng, IN const double lat, OUT stOptimalPointInfo* pOptInfo, IN const int32_t nEntType, IN const int32_t nRetCount, IN const int32_t nMatchType)
{
	int32_t cntRet = 0;
	stReqEntryType reqType = { nEntType };

	if (pOptInfo != nullptr)
	{
		const SPoint reqCoord = { lng, lat };
		vector<stEntryPointInfo>  vtEntInfo; // 입구점
		vector<stEntryPointInfo>  vtExpInfo; // 추가 속성 (단지내도로, 최근접도로)

		stPolygonInfo* pPoly = nullptr;
		stMeshInfo* pMesh = nullptr;

		//if (m_pMapMesh && (pMesh = m_pMapMesh->GetMeshByPoint(reqCoord.x, reqCoord.y)) != nullptr) {
		if (m_pMapMesh && (pMesh = GetMeshDataByPoint(reqCoord.x, reqCoord.y)) != nullptr) {
#if defined(USE_BUILDING_DATA)
			// find in building
			for (vector<KeyID>::const_iterator it = pMesh->buildings.begin(); it != pMesh->buildings.end(); it++) {
				if (m_pMapBuilding && m_pMapBuilding->IsPitInPolygon(*it, lng, lat))
				{
					pPoly = m_pMapBuilding->GetDataById(*it);
					//if (pPoly->vtEnt.empty()) {
					if (pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT) <= 0) {
						// 입구점이 없으면
						pPoly = nullptr;
						continue;
					}
					break;
				}
			} // for
#endif
#if defined(USE_COMPLEX_DATA)
			// find in complex
			if (pPoly == nullptr) {
				for (vector<KeyID>::const_iterator it = pMesh->complexs.begin(); it != pMesh->complexs.end(); it++) {
					if (m_pMapComplex && m_pMapComplex->IsPitInPolygon(*it, lng, lat))
					{
						pPoly = m_pMapComplex->GetDataById(*it);
						//if (pPoly->vtEnt.empty() && pPoly->vtLink.empty()) {
						if (pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT) <= 0 && pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_LINK) <= 0) {
							// 입구점 & 단지도로 모두 없으면
							pPoly = nullptr;
							continue;
						}
						break;
					}
				} // for
			}
#endif
		}
		else {
			m_strMsg = "Can't find matched mesh data";
		}
		

		pOptInfo->x = lng;
		pOptInfo->y = lat;

		if (pPoly != nullptr) {	
			// 폴리곤 정보 복사
			//pOptInfo->vtPolygon.assign(pPoly->vtVtx.begin(), pPoly->vtVtx.end());
			const SPoint* pPolygon = pPoly->getAttributeVertex();
			const stEntranceInfo* pEnt = pPoly->getAttributeEntrance();
			const KeyID* pLinkKey = pPoly->getAttributeLink();

			const int cntEnt = pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_ENT);
			const int cntLinkKey = pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_LINK);

			//std::copy(pData, pData + pPoly->getAttributeSize(TYPE_POLYGON_DATA_ATTR_VTX), pOptInfo->vtPolygon);
			pOptInfo->vtPolygon.insert(pOptInfo->vtPolygon.end(), &pPolygon[0], &pPolygon[pPoly->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX)]);

			if (pPoly->poly_id.ent.type == 0) { // 빌딩
				const int32_t nMaxDist = 1000; // 최대 1km
				//if (pPoly->bld.name > 0) {
				//	pOptInfo->name = GetNameDataByIdx(pPoly->bld.name);
				//}

				// 입구점 매칭
				//for (vector<stEntranceInfo>::const_iterator it = pPoly->vtEnt.begin(); it != pPoly->vtEnt.end(); it++) {
				if (reqType.typeAll == 0) // 전체 타입
				{
					cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, 0, 0, vtEntInfo);
				}
				else
				{
					// 최대 4번의 스텝까지 허용
					// nEntType, 입구점 타입 // 0:알아서, 1: 차량 입구점, 2: 택시 승하차 지점(건물), 3: 택시 승하차 지점(건물군), 4: 택배 차량 하차 거점, 5: 보행자 입구점, 	6: 배달 하차점(차량, 오토바이), 7: 배달 하차점(자전거, 도보)
					// 1st
					if (reqType.type1st != 0) {
						cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqType.type1st, 1, vtEntInfo);
					}
					// 2nd
					if (cntRet <= 0 && reqType.type2nd != 0) {
						cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqType.type2nd, 1, vtEntInfo);
					}
					// 3rd
					if (cntRet <= 0 && reqType.type3rd != 0) {
						cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqType.type3rd, 1, vtEntInfo);
					}
					// 4th
					if (cntRet <= 0 && reqType.type4th != 0) {
						cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqType.type4th, 1, vtEntInfo);
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
				}
			}
			else { // 단지
				const int32_t nMaxDist = 50000; // 최대 5km, 골프장이 꽤 넓음.

				if (reqType.typeAll == 0) // 전체 타입
				{
					cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, 0, 0, vtEntInfo);
				}
				else
				{
					// 최대 4번의 스텝까지 허용
					// nEntType, 입구점 타입 // 0:알아서, 1: 차량 입구점, 2: 택시 승하차 지점(건물), 3: 택시 승하차 지점(건물군), 4: 택배 차량 하차 거점, 5: 보행자 입구점, 	6: 배달 하차점(차량, 오토바이), 7: 배달 하차점(자전거, 도보)
					// 1st
					if (reqType.type1st != 0) {
						cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqType.type1st, 1, vtEntInfo);
					}					
					// 2nd
					if (cntRet <= 0 && reqType.type2nd != 0) {
						cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqType.type2nd, 1, vtEntInfo);
					}
					// 3rd
					if (cntRet <= 0 && reqType.type3rd != 0) {
						cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqType.type3rd, 1, vtEntInfo);
					}
					// 4th
					if (cntRet <= 0 && reqType.type4th != 0) {
						cntRet = checkEntType(lng, lat, pEnt, cntEnt, nMaxDist, reqType.type4th, 1, vtEntInfo);
					}
				}

				if (cntRet > 0) {
					pOptInfo->nType = TYPE_ENT_COMPLEX;
				}


				// 가장 가까운 단지내 도로 매칭
				stEntryPointInfo entInfo = { 0, };
				entInfo.dwDist = INT_MAX;

				stLinkInfo* pLink;
				double nMinDist = nMaxDist;
				double retLng, retLat, retDist, retIr, minIr;

				//for (vector<KeyID>::const_iterator it = pPoly->vtLink.begin(); it != pPoly->vtLink.end(); it++) {
				for (int ii = 0; ii < cntLinkKey; ii++) {
					pLink = GetVLinkDataById(pLinkKey[ii]);
					if (pLink != nullptr) {

						if ((linkProjection(pLink, lng, lat, nMinDist, retLng, retLat, retDist, retIr) < 0)) {
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

		// 입구점 정보가 없으면 주변 가까운 도로 검출
		if (vtEntInfo.empty()) {
			static const array<int32_t, 5> arrDist = { 100, 500, 1000, 3000, 5000 }; // 최대 5km 내에서 검색

			stEntryPointInfo entInfo = { 0, };
			bool retNearRoad = false;

			// 도로 사이드에 위치시킬 최적 지점 이격 거리
			for (const auto &nDist : arrDist) {
				// for (int ii = 0; ii < maxArrDist; ii++) {
				if ((retNearRoad = GetNearRoadByPoint(reqCoord.x, reqCoord.y, nDist, nMatchType, entInfo)) == true) {
					break;
				}
			} // for

			if (retNearRoad == true && entInfo.dwDist < INT_MAX) {
				if (pOptInfo->nType == TYPE_ENT_NONE) {
					// 결과 타입을 주변 도로로 적용
					pOptInfo->nType = TYPE_ENT_NEAR_ROAD;
				}
				vtExpInfo.emplace_back(entInfo);
			}
		}

		cntRet = vtEntInfo.size();
		if (cntRet <= 0 && vtExpInfo.empty()) {
			m_strMsg = "Can't find optimal points";
		}
		else {
			LOG_TRACE(LOG_DEBUG, "EntryPoint Info, size:%d", cntRet);
			if (cntRet > 1 && reqType.typeAll == 0) {
				sort(vtEntInfo.begin(), vtEntInfo.end(), cmpEntPoint);
			}

			// 요청된 갯수만 적용
			if (nRetCount <= 0 || cntRet <= nRetCount) {
				pOptInfo->vtEntryPoint.assign(vtEntInfo.begin(), vtEntInfo.end());
			}
			else {
				pOptInfo->vtEntryPoint.assign(vtEntInfo.begin(), vtEntInfo.begin() + nRetCount);
				cntRet = nRetCount;
			}

			// 추가 속성 적용
			LOG_TRACE(LOG_DEBUG, "Expand Info, size:%d", vtExpInfo.size());
			for (int ii = 0; ii < vtExpInfo.size(); ii++) {
				pOptInfo->vtEntryPoint.emplace_back(vtExpInfo[ii]);
				cntRet++;
			}
		}
	}

	return cntRet;
}


const char* CDataManager::GetErrorMessage(void)
{
	return m_strMsg.c_str();
}


bool CDataManager::GetNewData(IN const uint32_t idTile)
{
	unordered_map<uint32_t, FileIndex> mapNeedMatch;

	// id 매칭, 로딩되야 할 메쉬
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
		// 영역에 들어오는 메쉬면 추가
		if (isInPitBox(it->second.rtData, rtWorld)) {
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
		if (isInBox(lng, lat, idx.second.rtData)) {
			idMesh = idx.first;
			break;
		}
	} // for

	// 중심 좌표에 매칭되는 메쉬가 없으면 영역에 포함되는 메쉬 검색
	if (idMesh == NULL_VALUE) {
		for (const auto& idx : m_mapFileIndex) {
			if (isInPitBox(idx.second.rtData, rtWorld)) {
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

#if !defined(USE_DATA_CACHE)
			// id 매칭, 로딩되야 할 메쉬
			stMeshInfo* pMesh = m_pMapMesh->GetMeshById(idx.second.idTile);
			if (pMesh != nullptr) {
				for (const auto& mesh : pMesh->neighbors) {
					if ((it = m_mapFileIndex.find(mesh)) != m_mapFileIndex.end()) {
						mapNeedMatch.emplace(it->second.idTile, it->second);
					}
				} // for
			}
#endif
			break;
		}
	} // for

	return CalculateCache(mapNeedMatch);
}


void CDataManager::SetFileMgr(IN CFileManager* pFileMgr)
{
	m_pFileMgr = pFileMgr;
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

		if (!m_pFileMgr->GetData(itNeed->second.idxTile))
		{
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
	} // for

	return cntCached;
}
