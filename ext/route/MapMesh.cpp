
#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "MapMesh.h"
#include "../utils/UserLog.h"
#include "../utils/GeoTools.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

#ifdef TEST_SPATIALINDEX
class LinkVisitor : public SpatialIndex::IVisitor
{
public:
	std::vector<id_type> results;

	// 노드 방문
	void visitNode(const SpatialIndex::INode& n) override
	{
		// 필요 시 구현. 비워둬도 OK
	}

	// 데이터 방문 (단일 객체)
	void visitData(const SpatialIndex::IData& d) override
	{
		results.push_back(d.getIdentifier());
	}

	// 데이터 방문 (여러 객체)
	void visitData(std::vector<const SpatialIndex::IData*>& v) override
	{
		for (const auto& data : v) {
			results.push_back(data->getIdentifier());
		}
	}
};


class DistanceVisitor : public SpatialIndex::IVisitor
{
public:
	SpatialIndex::Point query;
	double maxDist;
	std::vector<id_type> results;

	DistanceVisitor(const SpatialIndex::Point& q, double d) : query(q), maxDist(d) {}

	void visitNode(const SpatialIndex::INode&) override {}

	void visitData(const SpatialIndex::IData& d) override
	{
		SpatialIndex::IShape* shape = nullptr;
		d.getShape(&shape);
		double dist = query.getMinimumDistance(*shape);
		if (dist <= maxDist) {
			results.push_back(d.getIdentifier());
		}

		delete shape;
	}

	void visitData(std::vector<const SpatialIndex::IData*>& v) override
	{
		for (const auto& d : v) {
			SpatialIndex::IShape* shape = nullptr;
			d->getShape(&shape);
			double dist = query.getMinimumDistance(*shape);
			if (dist <= maxDist) {
				results.push_back(d->getIdentifier());
			}

			delete shape;
		}
	}
};


class PointContainmentVisitor : public SpatialIndex::IVisitor
{
public:
	SpatialIndex::Point query;
	std::vector<id_type> results;

	PointContainmentVisitor(const SpatialIndex::Point& p) : query(p) {}

	void visitNode(const SpatialIndex::INode&) override {}

	void visitData(const SpatialIndex::IData& d) override
	{
		SpatialIndex::IShape* shape = nullptr;
		d.getShape(&shape);

		const Region* region = dynamic_cast<const Region*>(shape);
		if (region && region->containsPoint(query)) {
			results.push_back(d.getIdentifier());
		}

		delete shape;
	}

	void visitData(std::vector<const SpatialIndex::IData*>& v) override
	{
		for (const auto& d : v) {
			SpatialIndex::IShape* shape = nullptr;
			d->getShape(&shape);

			const Region* region = dynamic_cast<const Region*>(shape);
			if (region && region->containsPoint(query)) {
				results.push_back(d->getIdentifier());
			}

			delete shape;
		}
	}
};


class BulkDataStream : public SpatialIndex::IDataStream
{
public:
	std::vector<SpatialIndex::Region>& regions;
	std::vector<id_type>& ids;
	size_t index = 0;

	BulkDataStream(std::vector<SpatialIndex::Region>& r, std::vector<id_type>& i)
		: regions(r), ids(i) {}

	bool hasNext() override {
		return index < regions.size();
	}

	SpatialIndex::IData* getNext() override {
		if (!hasNext()) {
			throw Tools::EndOfStreamException("BulkDataStream: getNext past end");
		}

		//if (index >= regions.size()) return nullptr;  // 안전하게 처리

		SpatialIndex::Region& rect = regions[index];
		id_type id = ids[index];
		index++;

		return new SpatialIndex::RTree::Data(0, nullptr, rect, id);
	}

	void rewind() override {
		index = 0;
	}

	uint32_t size() override {
		return regions.size();
	}
};
#endif // #ifdef TEST_SPATIALINDEX


MapMesh::MapMesh()
{
}


MapMesh::~MapMesh()
{
	Release();
}


bool MapMesh::Initialize(void)
{
	return MapBase::Initialize();
}


void MapMesh::Release(void)
{
	if (!m_mapData.empty())
	{
		for (map<uint64_t, stMeshInfo*>::iterator it = m_mapData.begin(); it != m_mapData.end(); it++)
		{
			delete it->second;
			it->second = nullptr;
		}

		m_mapData.clear();
		map<uint64_t, stMeshInfo*>().swap(m_mapData);
	}

	if (!m_vtKeyData.empty()) {
		m_vtKeyData.clear();
		vector<uint64_t>().swap(m_vtKeyData);
	}

	MapBase::Release();
}


void MapMesh::SetBox(IN const SBox* pBox)
{
	memcpy(&m_rtBox, pBox, sizeof(m_rtBox));
}


int MapMesh::AddData(IN const stMeshInfo * pData)
{
	if (pData != nullptr) {
		auto result = m_mapData.emplace(pData->mesh_id.tile_id, const_cast<stMeshInfo*>(pData));
		if (result.second) {
			uint64_t key = result.first->first;
			m_vtKeyData.emplace_back(key); // 순차 접근용
		}
#ifdef TEST_SPATIALINDEX
#if !defined(USE_SPATIALINDEX_BULK)
		CreateSpatialindex(const_cast<stMeshInfo*>(pData));
#endif
#endif
	}
	
	return m_mapData.size();
}


bool MapMesh::DeleteData(IN const uint64_t id)
{
	map<uint64_t, stMeshInfo*>::const_iterator it = m_mapData.find(id);
	if (it != m_mapData.end()) {
		try {
			// vector 먼저 삭제
			m_vtKeyData.erase(std::remove(m_vtKeyData.begin(), m_vtKeyData.end(), it->first), m_vtKeyData.end());

			delete it->second;
			m_mapData.erase(it);
		}
		catch(exception e) {
			LOG_TRACE(LOG_ERROR, "Error, delete mesh, %s", e.what());
		}

		return true;
	}

	return false;
}


#if defined(USE_FOREST_DATA)
int MapMesh::InsertFNode(IN const KeyID keyId)
{
	int ret = 0;

	stMeshInfo* pMesh = GetMeshById(keyId.tile_id);
	if (pMesh) {
#if 1//defined(_DEBUG)
		auto it = pMesh->setFNodeDuplicateCheck.insert(keyId);
		if (!it.second)
		{
			LOG_TRACE(LOG_ERROR, "Error, --------------- node id duplicated in MapMesh, %d", keyId.nid);
		}
#endif

#ifdef TEST_SPATIALINDEX
		ret = pMesh->setFNodeDuplicateCheck.size();
#else
		pMesh->nodes.emplace_back(keyId);

		ret = pMesh->nodes.size();
#endif
	}

	return ret;
}

int MapMesh::InsertFLink(IN const stLinkInfo * pData)
{
	int ret = 0;

	if (pData != nullptr)
	{
		stMeshInfo* pMesh = GetMeshById(pData->link_id.tile_id);
		if (pMesh) {
#if 1// defined(_DEBUG)
			auto it = pMesh->setFLinkDuplicateCheck.insert(pData->link_id);
			if (!it.second)
			{
				//LOG_TRACE(LOG_ERROR, "Error, --------------- flink id duplicated in MapMesh, %d", pData->link_id.nid);
			}
#endif

			pMesh->data_box = pMesh->mesh_box;

#ifdef TEST_SPATIALINDEX
			ISpatialIndex * pTree = reinterpret_cast<ISpatialIndex*>(pMesh->flinkTree);
			if (pTree) {
				SBox boxArea;
				boxArea.Xmin = boxArea.Ymin = DBL_MAX;
				boxArea.Xmax = boxArea.Ymax = DBL_MIN;
				extendDataBox(boxArea, pData->getVertex(), pData->getVertexCount());
				double low[2] = { boxArea.Xmin, boxArea.Ymin };
				double high[2] = { boxArea.Xmax, boxArea.Ymax };
				Region newRect(low, high, 2);

				pTree->insertData(0, nullptr, newRect, pData->link_id.llid);

				ret = pMesh->setFLinkDuplicateCheck.size();
			}
#else
			pMesh->links.emplace_back(pData->link_id);

			ret = pMesh->links.size();
#endif		
		}

#if defined(USE_FOREST_DATA)
		// 숲길 데이터는 1판 메쉬이기에 속도를 위해 링크에 추가된 메쉬ID로 추가 적용
		if (pData->base.link_type == TYPE_LINK_DATA_TREKKING && pData->trk_ext.trk_ext_reserved != 0) {
			stMeshInfo* pMesh = GetMeshById(pData->trk_ext.trk_ext_reserved);

			if (pMesh != nullptr) {
#if 1// defined(_DEBUG)
				auto it = pMesh->setFLinkDuplicateCheck.insert(pData->link_id);
				if (!it.second) {
					//LOG_TRACE(LOG_ERROR, "Error, --------------- flink id duplicated in MapMesh, %d", pData->link_id.nid);
				}
#endif

#ifdef TEST_SPATIALINDEX
				ISpatialIndex * pTree = reinterpret_cast<ISpatialIndex*>(pMesh->flinkTree);
				if (pTree) {
					SBox boxArea;
					boxArea.Xmin = boxArea.Ymin = DBL_MAX;
					boxArea.Xmax = boxArea.Ymax = DBL_MIN;
					extendDataBox(boxArea, pData->getVertex(), pData->getVertexCount());
					double low[2] = { boxArea.Xmin, boxArea.Ymin };
					double high[2] = { boxArea.Xmax, boxArea.Ymax };
					Region newRect(low, high, 2);

					pTree->insertData(0, nullptr, newRect, pData->link_id.llid);

					ret = pMesh->setFLinkDuplicateCheck.size();
				}
#else
				pMesh->links.emplace_back(pData->link_id);

				ret = pMesh->links.size();
#endif	
			}
		}
#endif
	}

	return ret;
}
#endif // #if defined(USE_FOREST_DATA)


#if defined(USE_PEDESTRIAN_DATA)
int MapMesh::InsertWNode(IN const KeyID keyId)
{
	int ret = 0;

	stMeshInfo* pMesh = GetMeshById(keyId.tile_id);
	if (pMesh) {
#if 1//defined(_DEBUG)
		auto it = pMesh->setWNodeDuplicateCheck.insert(keyId);
		if (!it.second)
		{
			LOG_TRACE(LOG_ERROR, "Error, --------------- node id duplicated in MapMesh, %d", keyId.nid);
		}
#endif

#ifdef TEST_SPATIALINDEX
		ret = pMesh->setWNodeDuplicateCheck.size();
#else
		pMesh->wnodes.emplace_back(keyId);

		ret = pMesh->wnodes.size();
#endif
	}

	return ret;
}

int MapMesh::InsertWLink(IN const stLinkInfo * pData)
{
	int ret = 0;

	if (pData != nullptr)
	{
		stMeshInfo* pMesh = GetMeshById(pData->link_id.tile_id);
		if (pMesh) {
#if 1// defined(_DEBUG)
			auto it = pMesh->setWLinkDuplicateCheck.insert(pData->link_id);
			if (!it.second)
			{
				LOG_TRACE(LOG_ERROR, "Error, --------------- wlink id duplicated in MapMesh, %d", pData->link_id.nid);
			}
#endif

			pMesh->data_box = pMesh->mesh_box;

#ifdef TEST_SPATIALINDEX
			ISpatialIndex * pTree = reinterpret_cast<ISpatialIndex*>(pMesh->wlinkTree);
			if (pTree) {
				SBox boxArea;
				boxArea.Xmin = boxArea.Ymin = DBL_MAX;
				boxArea.Xmax = boxArea.Ymax = DBL_MIN;
				extendDataBox(boxArea, pData->getVertex(), pData->getVertexCount());
				double low[2] = { boxArea.Xmin, boxArea.Ymin };
				double high[2] = { boxArea.Xmax, boxArea.Ymax };
				Region newRect(low, high, 2);

				pTree->insertData(0, nullptr, newRect, pData->link_id.llid);

				ret = pMesh->setWLinkDuplicateCheck.size();
			}
#else
			pMesh->wlinks.emplace_back(pData->link_id);

			ret = pMesh->wlinks.size();
#endif			
		}
	}

	return ret;
}
#endif // #if defined(USE_PEDESTRIAN_DATA)


#if defined(USE_VEHICLE_DATA)
int MapMesh::InsertVNode(IN const KeyID keyId)
{
	int ret = 0;

	stMeshInfo* pMesh = GetMeshById(keyId.tile_id);
	if (pMesh) {
#if 1//defined(_DEBUG)
		auto it = pMesh->setVNodeDuplicateCheck.insert(keyId);
		if (!it.second)
		{
			LOG_TRACE(LOG_ERROR, "Error, --------------- node id duplicated in MapMesh, %d", keyId.nid);
		}
#endif

#ifdef TEST_SPATIALINDEX
		ret = pMesh->setVNodeDuplicateCheck.size();
#else
		pMesh->vnodes.emplace_back(keyId);

		ret = pMesh->vnodes.size();
#endif
	}

	return ret;
}

int MapMesh::InsertVLink(IN const stLinkInfo * pData)
{
	int ret = 0;

	if (pData != nullptr)
	{
		stMeshInfo* pMesh = GetMeshById(pData->link_id.tile_id);
		if (pMesh) {
#if 1// defined(_DEBUG)
			auto it = pMesh->setVLinkDuplicateCheck.insert(pData->link_id);
			if (!it.second)
			{
				LOG_TRACE(LOG_ERROR, "Error, --------------- vlink id duplicated in MapMesh, %d", pData->link_id.nid);
			}
#endif

//#pragma omp parallel for
//			for (int ii = 0; ii < pData->getVertexCount(); ii++)
//			{
//				ExtendDataBox(pMesh, pData->getVertexX(ii), pData->getVertexY(ii));
//			}

			pMesh->data_box = pMesh->mesh_box;

#ifdef TEST_SPATIALINDEX
#if !defined(USE_SPATIALINDEX_BULK)
			SBox boxArea;
			boxArea.Xmin = boxArea.Ymin = DBL_MAX;
			boxArea.Xmax = boxArea.Ymax = DBL_MIN;
			extendDataBox(boxArea, pData->getVertex(), pData->getVertexCount());

			ISpatialIndex * pTree = reinterpret_cast<ISpatialIndex*>(pMesh->vlinkTree);
			if (pTree) {
				double low[2] = { boxArea.Xmin, boxArea.Ymin };
				double high[2] = { boxArea.Xmax, boxArea.Ymax };
				Region newRect(low, high, 2);
				pTree->insertData(0, nullptr, newRect, pData->link_id.llid);
			}
#endif
			ret = pMesh->setVLinkDuplicateCheck.size();
#else
			pMesh->vlinks.emplace_back(pData->link_id);			

			ret = pMesh->vlinks.size();
#endif
		}
	}

	return ret;
}
#endif // #if defined(USE_VEHICLE_DATA)


#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
bool MapMesh::InsertComplex(IN const stPolygonInfo * pData)
{
	stMeshInfo* pMesh = GetMeshById(pData->poly_id.tile_id);
	if (pMesh) {
#if 1//defined(_DEBUG)
		auto it = pMesh->setCpxDuplicateCheck.insert(pData->poly_id);
		if (!it.second)
		{
			LOG_TRACE(LOG_ERROR, "Error, --------------- complex id duplicated in MapMesh, tile:%d, id:%d", pData->poly_id.tile_id, pData->poly_id.nid);
		}
#endif

		pMesh->data_box = pMesh->mesh_box;
		extendDataBox(pMesh->data_box, pData->getAttributeVertex(), pData->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX));

#ifdef TEST_SPATIALINDEX
#if !defined(USE_SPATIALINDEX_BULK)
		SBox boxArea = pData->data_box;
		if (pTree) {
			double low[2] = { boxArea.Xmin, boxArea.Ymin };
			double high[2] = { boxArea.Xmax, boxArea.Ymax };
			Region newRect(low, high, 2);
			ISpatialIndex * pTree = reinterpret_cast<ISpatialIndex*>(pMesh->complexTree);
			pTree->insertData(0, nullptr, newRect, pData->poly_id.llid);
		}
#endif
#else
		pMesh->complexs.emplace_back(pData->poly_id);
#endif


#if !defined(USE_DATA_CACHE) 
		if (pData->getAttributeCount(TYPE_POLYGON_DATA_ATTR_MESH) > 1) { // 항상 자신을 포함하므로 1개 이상일때만 인접메쉬 존재
			stMeshInfo* pJoinedMesh;
			uint32_t idMesh;
			const uint32_t* pJoined = pData->getAttributeMesh();
			for (int ii = pData->getAttributeCount(TYPE_POLYGON_DATA_ATTR_MESH) - 1; ii >= 0; --ii) {
				idMesh = pJoined[ii];
				// 23-01-06 // 단독 데이터 로딩 시, 메쉬가 순차적으로 로딩되기에 나자신보다 나중에 로딩되는 메시는 메쉬 정보가 없어 누락될 수 있음
				// 메쉬 파일을 따로 관리하는게 나을라나??
				if ((pData->poly_id.tile_id != idMesh) && ((pJoinedMesh = GetMeshById(idMesh)) != nullptr)) { // 자신은 패스
#ifdef TEST_SPATIALINDEX
#if !defined(USE_SPATIALINDEX_BULK)
					ISpatialIndex * pTree = reinterpret_cast<ISpatialIndex*>(pMesh->complexTree);
					if (pTree) {
						SBox boxArea = pData->data_box;
						double low[2] = { boxArea.Xmin, boxArea.Ymin };
						double high[2] = { boxArea.Xmax, boxArea.Ymax };
						Region newRect(low, high, 2);
						pTree->insertData(0, nullptr, newRect, pData->poly_id.llid);
					}
#endif
#else
					pJoinedMesh->complexs.emplace_back(pData->poly_id);
#endif // #ifdef TEST_SPATIALINDEX
				}
			} // for
		}
#endif
		return true;
	}

	return false;
}


bool MapMesh::InsertBuilding(IN const stPolygonInfo * pData)
{
	stMeshInfo* pMesh = GetMeshById(pData->poly_id.tile_id);
	if (pMesh) {
#if 1//defined(_DEBUG)
		auto it = pMesh->setBldDuplicateCheck.insert(pData->poly_id);
		if (!it.second)
		{
			LOG_TRACE(LOG_ERROR, "Error, --------------- building id duplicated in MapMesh, tile:%d, id:%d", pData->poly_id.tile_id, pData->poly_id.nid);
		}
#endif

		pMesh->data_box = pMesh->mesh_box;
		extendDataBox(pMesh->data_box, pData->getAttributeVertex(), pData->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX));

#ifdef TEST_SPATIALINDEX
		SBox boxArea = pData->data_box;
#if !defined(USE_SPATIALINDEX_BULK)
		ISpatialIndex * pTree = reinterpret_cast<ISpatialIndex*>(pMesh->complexTree);
		if (pTree) {
			double low[2] = { boxArea.Xmin, boxArea.Ymin };
			double high[2] = { boxArea.Xmax, boxArea.Ymax };
			Region newRect(low, high, 2);
			pTree->insertData(0, nullptr, newRect, pData->poly_id.llid);
		}
#endif
#else
		pMesh->buildings.emplace_back(pData->poly_id);
#endif

		return true;
	}

	return false;
}
#endif

stMeshInfo * MapMesh::GetMeshById(IN int32_t meshId)
{
	map<uint64_t, stMeshInfo*>::const_iterator it = m_mapData.find(meshId);
	if (it != m_mapData.end()) {
		return &*m_mapData.find(meshId)->second;
	}

	return nullptr;	
}

stMeshInfo* MapMesh::GetMeshByPoint(IN const double lng, IN const double lat)
{
	stMeshInfo* pMesh = nullptr;

	if (lng != 0.f && lat != 0.f)
	{
#if defined(USE_FOREST_DATA)
		// 숲길은 단일 메쉬고, 마지막 값으로 정의했기에 숲길을 우선 검색
		for (map<uint64_t, stMeshInfo*>::const_reverse_iterator it = m_mapData.rbegin(); it != m_mapData.rend(); it++)
#else
		for (map<uint64_t, stMeshInfo*>::const_iterator it = m_mapData.begin(); it != m_mapData.end(); it++)
#endif
		{
#if defined(USE_VEHICLE_DATA)
			if (it->second->mesh_id.tile_id == GLOBAL_MESH_ID) {
				continue;
			}
#endif
			if (lng < it->second->mesh_box.Xmin || it->second->mesh_box.Xmax < lng
				|| lat < it->second->mesh_box.Ymin || it->second->mesh_box.Ymax < lat) {
				continue;
			}

			pMesh = it->second;
			
			break;
		} // for
	}

	return pMesh;
}

int MapMesh::GetPitInMesh(IN const double lng, IN const double lat, IN const int nMaxBuff, OUT stMeshInfo** pData)
{
	int cntMesh = 0;

	if (nMaxBuff > 0 && pData != nullptr)
	{
#if defined(USE_FOREST_DATA)
		// 숲길은 단일 메쉬고, 마지막 값으로 정의했기에 숲길을 우선 검색
		for (map<uint64_t, stMeshInfo*>::const_reverse_iterator it = m_mapData.rbegin(); it != m_mapData.rend(); it++)
#else
		for (map<uint64_t, stMeshInfo*>::const_iterator it = m_mapData.begin(); it != m_mapData.end(); it++)
#endif
		{
			if (lng < it->second->mesh_box.Xmin || it->second->mesh_box.Xmax < lng
				|| lat < it->second->mesh_box.Ymin || it->second->mesh_box.Ymax < lat) {
				continue;
			}

			pData[cntMesh++] = it->second;

			if (cntMesh >= nMaxBuff)
				break;
		} // for
	}

	return cntMesh;
}

int MapMesh::GetPitInData(IN const double lng, IN const double lat, IN const int nMaxBuff, OUT stMeshInfo** pData)
{
	int cntMesh = 0;

	if (nMaxBuff > 0 && pData != nullptr)
	{
#if defined(USE_FOREST_DATA)
		// 숲길은 단일 메쉬고, 마지막 값으로 정의했기에 숲길을 우선 검색
		for (map<uint64_t, stMeshInfo*>::const_reverse_iterator it = m_mapData.rbegin(); it != m_mapData.rend(); it++)
#else
		for (map<uint64_t, stMeshInfo*>::const_iterator it = m_mapData.begin(); it != m_mapData.end(); it++)
#endif
		{
			if (lng < it->second->data_box.Xmin || it->second->data_box.Xmax < lng
				|| lat < it->second->data_box.Ymin || it->second->data_box.Ymax < lat) {
				continue;
			}

			pData[cntMesh++] = it->second;

			if (cntMesh >= nMaxBuff)
				break;
		} // for
	}

	return cntMesh;
}


uint32_t MapMesh::GetPitInRegion(IN const SBox& pRegion, IN const uint32_t cntMaxBuff, OUT stMeshInfo** pMeshInfo)
{
	uint32_t cntPitIn = 0;
	uint32_t cntMesh = m_mapData.size();

	if (cntMaxBuff <= 0 || pMeshInfo == nullptr) {
		LOG_TRACE_DEBUG(LOG_ERROR, "%s", "param value null");
		return 0;
	}

#if defined(USE_FOREST_DATA)
	// 숲길은 단일 메쉬고, 마지막 값으로 정의했기에 숲길을 우선 검색
	for (map<uint64_t, stMeshInfo*>::const_reverse_iterator it = m_mapData.rbegin(); it != m_mapData.rend(); it++)
	//for (map<uint64_t, stMeshInfo*>::const_iterator it = mapData.begin(); it != mapData.end(); it++)
#else
	for (map<uint64_t, stMeshInfo*>::const_iterator it = m_mapData.begin(); it != m_mapData.end(); it++)
#endif
	{
		if (isInPitBox(it->second->data_box, pRegion)) {
			pMeshInfo[cntPitIn++] = it->second;
		}

		//if (isInPitBox(it->second->mesh_box, pRegion)) {
		//	pMeshInfo[cntPitIn++] = it->second;
		//}

		if (cntPitIn >= cntMaxBuff) {
			break;
		}
	}

	return cntPitIn;
}


const uint32_t MapMesh::GetCount(void) const
{
	return m_mapData.size();
}


// 성능떨어지는 코드, 가능하면 사용하지 말자 2024-01-22
stMeshInfo * MapMesh::GetMeshData(IN const uint32_t idx)
{
	stMeshInfo * retMesh = nullptr;

	if (0 <= idx && idx < m_mapData.size())
	{
		if (idx < m_vtKeyData.size()) {
			uint64_t key = m_vtKeyData[idx];
			retMesh = GetMeshById(key);
		} else {
			LOG_TRACE(LOG_WARNING, "map & vector count not matched, should check it!");

			map<uint64_t, stMeshInfo*>::const_iterator it = m_mapData.begin();
			advance(it, idx);
			retMesh = it->second;
		}
	}

	return retMesh;
}


//void MapMesh::CheckNeighborMesh(void)
//{
//	static const int nMaxNeighbor = 8;
//
//	// find neighbor
//
//	// 소수점 단위가 오차가 있을 수 있어, 10만 곱한 영역을 비교하자	
//	for (map<uint64_t, stMeshInfo*>::iterator itMe = mapData.begin(); itMe != mapData.end(); itMe++)
//	{
//		RECT rtMe = {
//			static_cast<int32_t>(itMe->second->mesh_box.Xmin * 100000),
//			static_cast<int32_t>(itMe->second->mesh_box.Ymin * 100000),
//			static_cast<int32_t>(itMe->second->mesh_box.Xmax * 100000),
//			static_cast<int32_t>(itMe->second->mesh_box.Ymax * 100000) };
//
//		if (itMe->second->links.empty()) {
//			continue;
//		}
//
//		for (map<uint64_t, stMeshInfo*>::iterator itYou = mapData.begin(); itYou != mapData.end(); itYou++)
//		{
//			if (itMe->second->mesh_id.tile_id == itYou->second->mesh_id.tile_id ||
//				itYou->second->links.empty()) {
//				continue;
//			}
//
//			bool isNeighbor = false;
//			RECT rtYou = { 
//				static_cast<int32_t>(itYou->second->mesh_box.Xmin * 100000), 
//				static_cast<int32_t>(itYou->second->mesh_box.Ymin * 100000), 
//				static_cast<int32_t>(itYou->second->mesh_box.Xmax * 100000), 
//				static_cast<int32_t>(itYou->second->mesh_box.Ymax * 100000) };
//
//
//			// LT == old->lt, t, l 
//			if ((rtMe.left == rtYou.right && rtMe.top == rtYou.top) ||
//				(rtMe.left == rtYou.left && rtMe.top == rtYou.bottom) ||
//				(rtMe.left == rtYou.right && rtMe.top == rtYou.bottom))
//			{
//				isNeighbor = true;
//			}
//			// RT == old->rt, t, r 
//			else if ((rtMe.right == rtYou.left && rtMe.top == rtYou.bottom) ||
//				(rtMe.right == rtYou.right && rtMe.top == rtYou.bottom) ||
//				(rtMe.right == rtYou.left && rtMe.top == rtYou.bottom))
//			{
//				isNeighbor = true;
//			}
//			// LB == old->lb, b, l 
//			else if ((rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom) ||
//				(rtMe.left == rtYou.left && rtMe.bottom == rtYou.top) ||
//				(rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom))
//			{
//				isNeighbor = true;
//			}
//			// RB == old->rb, b, r 
//			else if ((rtMe.right == rtYou.left && rtMe.bottom == rtYou.top) ||
//				(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top) ||
//				(rtMe.right == rtYou.left && rtMe.bottom == rtYou.top))
//			{
//				isNeighbor = true;
//			}
//			// L
//			else if ((rtMe.left == rtYou.right && rtMe.top == rtYou.top) ||
//				(rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom))
//			{
//				isNeighbor = true;
//			}
//			// R
//			else if ((rtMe.right == rtYou.left && rtMe.top == rtYou.top) ||
//				(rtMe.right == rtYou.left && rtMe.bottom == rtYou.bottom))
//			{
//				isNeighbor = true;
//			}
//			// T
//			else if ((rtMe.left == rtYou.left && rtMe.top == rtYou.bottom) ||
//				(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top))
//			{
//				isNeighbor = true;
//			}
//			// B
//			else if ((rtMe.left == rtYou.left && rtMe.bottom == rtYou.top) ||
//				(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top))
//			{
//				isNeighbor = true;
//			}
//
//			// Box edge
//			else {
//				rtYou.left = static_cast<int32_t>(m_rtBox.Xmin * 100000);
//				rtYou.top = static_cast<int32_t>(m_rtBox.Ymin * 100000);
//				rtYou.right = static_cast<int32_t>(m_rtBox.Xmax * 100000);
//				rtYou.bottom = static_cast<int32_t>(m_rtBox.Ymax * 100000);
//
//				// LT == old->lt, t, l 
//				if ((rtMe.left == rtYou.right && rtMe.top == rtYou.top) ||
//					(rtMe.left == rtYou.left && rtMe.top == rtYou.bottom) ||
//					(rtMe.left == rtYou.right && rtMe.top == rtYou.bottom))
//				{
//					isNeighbor = true;
//				}
//				// RT == old->rt, t, r 
//				else if ((rtMe.right == rtYou.left && rtMe.top == rtYou.bottom) ||
//					(rtMe.right == rtYou.right && rtMe.top == rtYou.bottom) ||
//					(rtMe.right == rtYou.left && rtMe.top == rtYou.bottom))
//				{
//					isNeighbor = true;
//				}
//				// LB == old->lb, b, l 
//				else if ((rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom) ||
//					(rtMe.left == rtYou.left && rtMe.bottom == rtYou.top) ||
//					(rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom))
//				{
//					isNeighbor = true;
//				}
//				// RB == old->rb, b, r 
//				else if ((rtMe.right == rtYou.left && rtMe.bottom == rtYou.top) ||
//					(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top) ||
//					(rtMe.right == rtYou.left && rtMe.bottom == rtYou.top))
//				{
//					isNeighbor = true;
//				}
//				// L
//				else if ((rtMe.left == rtYou.right && rtMe.top == rtYou.top) ||
//					(rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom))
//				{
//					isNeighbor = true;
//				}
//				// R
//				else if ((rtMe.right == rtYou.left && rtMe.top == rtYou.top) ||
//					(rtMe.right == rtYou.left && rtMe.bottom == rtYou.bottom))
//				{
//					isNeighbor = true;
//				}
//				// T
//				else if ((rtMe.left == rtYou.left && rtMe.top == rtYou.bottom) ||
//					(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top))
//				{
//					isNeighbor = true;
//				}
//				// B
//				else if ((rtMe.left == rtYou.left && rtMe.bottom == rtYou.top) ||
//					(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top))
//				{
//					isNeighbor = true;
//				}
//			}
//
//
//			if (isNeighbor)
//			{
//				itMe->second->neighbors.emplace((uint32_t)itYou->second->mesh_id.tile_id);
//				itYou->second->neighbors.emplace((uint32_t)itMe->second->mesh_id.tile_id);
//
//				if (itYou->second->neighbors.size() > nMaxNeighbor) {
//					LOG_TRACE(LOG_ERROR, "Error, --------------- Mesh your neighbor count overflow, id:%d, %d", itYou->second->mesh_id.tile_id, itYou->second->neighbors.size());
//				}
//				if (itMe->second->neighbors.size() > nMaxNeighbor) {
//					LOG_TRACE(LOG_ERROR, "Error, --------------- Mesh my neighbor count overflow, id:%d, %d", itMe->second->mesh_id.tile_id, itMe->second->neighbors.size());
//				}
//			}
//		} // for		
//	}
//}


void MapMesh::ArrangementMesh(void)
{
	for (map<uint64_t, stMeshInfo*>::const_iterator it = m_mapData.begin(); it != m_mapData.end();) {
#ifdef TEST_SPATIALINDEX
		if (
#if defined(USE_FOREST_DATA)
			(!it->second->setFNodeDuplicateCheck.empty() || !it->second->setFLinkDuplicateCheck.empty()) ||
#if defined(USE_PEDESTRIAN_DATA)
			(!it->second->setWNodeDuplicateCheck.empty() || !it->second->setWLinkDuplicateCheck.empty()) ||
#endif
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
			(!it->second->setCpxDuplicateCheck.empty() || !it->second->setBldDuplicateCheck.empty()) ||
#endif
#endif // #if defined(USE_FOREST_DATA)
#if defined(USE_PEDESTRIAN_DATA)
			(!it->second->setWNodeDuplicateCheck.empty() || !it->second->setWLinkDuplicateCheck.empty()) ||
#endif
#if defined(USE_VEHICLE_DATA)
			(!it->second->setVNodeDuplicateCheck.empty() || !it->second->setVLinkDuplicateCheck.empty() || it->first == 0) || // 전역메쉬(0)은 ks 저장을 위해 남겨두자
#endif
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
			(!it->second->setBldDuplicateCheck.empty() || !it->second->setCpxDuplicateCheck.empty()) ||
#endif
			0) {
			it++;
		} else {
			//LOG_TRACE(LOG_DEBUG, "LOG, remove empty mesh : %d", it->first);
			//m_mapData.erase(it++);

			// vector 먼저 삭제
			m_vtKeyData.erase(std::remove(m_vtKeyData.begin(), m_vtKeyData.end(), it->first), m_vtKeyData.end());

			delete it->second;
			m_mapData.erase(it++);
		}
#else
#if defined(USE_FOREST_DATA)
		if ((!it->second->nodes.empty() || !it->second->links.empty())
#if defined(USE_PEDESTRIAN_DATA)
			|| (!it->second->wnodes.empty() || !it->second->wlinks.empty())
#endif
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
			|| (!it->second->complexs.empty() || !it->second->buildings.empty())
#endif
#endif // #if defined(USE_FOREST_DATA)
#if defined(USE_PEDESTRIAN_DATA)
		if ((!it->second->wnodes.empty() || !it->second->wlinks.empty())
#endif
#if defined(USE_VEHICLE_DATA)
		if ((!it->second->vnodes.empty() || !it->second->vlinks.empty() || it->first == 0) // 전역메쉬(0)은 ks 저장을 위해 남겨두자
#endif
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
			|| (!it->second->complexs.empty() || !it->second->buildings.empty())
#endif
			) {
			it++;
		} 
		else {
			//LOG_TRACE(LOG_DEBUG, "LOG, remove empty mesh : %d", it->first);
			m_mapData.erase(it++);
		}
#endif
	}
}


#ifdef TEST_SPATIALINDEX
int32_t	MapMesh::CreateSpatialindex(IN const int32_t nType, IN stMeshInfo* pMesh, vector<SpatialIndex::Region>& regions, vector<id_type>& ids)
{
	int32_t ret = 0;

	if (pMesh && !regions.empty() && !ids.empty()) {
#if defined(_DEBUG)
		uint32_t indexCapacity = 500;
		uint32_t leafCapacity = 500;
		double fillFactor = 0.9;
#else
		uint32_t indexCapacity = 300;
		uint32_t leafCapacity = 300;
		double fillFactor = 0.7;
#endif
		uint32_t demension = 2;

		id_type indexId;

		if (nType == TYPE_DATA_TREKKING) {
#if defined(USE_FOREST_DATA)
			if (!pMesh->flinkStorage && !pMesh->flinkTree) {
				pMesh->flinkStorage = static_cast<void*>(StorageManager::createNewMemoryStorageManager());
				pMesh->flinkTree = static_cast<void*>(RTree::createNewRTree(*static_cast<IStorageManager*>(pMesh->flinkStorage), fillFactor, indexCapacity, leafCapacity, demension, SpatialIndex::RTree::RV_RSTAR, indexId));
		}
#endif
		} else if (nType == TYPE_DATA_PEDESTRIAN) {
#if defined(USE_PEDESTRIAN_DATA)
			if (!pMesh->wlinkStorage && !pMesh->wlinkTree) {
				pMesh->wlinkStorage = static_cast<void*>(StorageManager::createNewMemoryStorageManager());
				pMesh->wlinkTree = static_cast<void*>(RTree::createNewRTree(*static_cast<IStorageManager*>(pMesh->wlinkStorage), fillFactor, indexCapacity, leafCapacity, demension, SpatialIndex::RTree::RV_RSTAR, indexId));
			}
#endif
		} else if (nType == TYPE_DATA_VEHICLE) {
#if defined(USE_VEHICLE_DATA)
			if (!pMesh->vlinkStorage && !pMesh->vlinkTree) {
				pMesh->vlinkStorage = reinterpret_cast<void*>(StorageManager::createNewMemoryStorageManager());
				pMesh->vlinkTree = reinterpret_cast<void*>(RTree::createNewRTree(*reinterpret_cast<IStorageManager*>(pMesh->vlinkStorage), fillFactor, indexCapacity, leafCapacity, demension, SpatialIndex::RTree::RV_RSTAR, indexId));
				for (int ii = 0; ii < regions.size(); ii++) {
					((ISpatialIndex*)pMesh->vlinkTree)->insertData(0, nullptr, regions[ii], ids[ii]);
				}
			}
#endif
		} else if (nType == TYPE_DATA_BUILDING) {
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
			if (!pMesh->buildingStorage && !pMesh->buildingTree) {
				pMesh->buildingStorage = reinterpret_cast<void*>(StorageManager::createNewMemoryStorageManager());
				pMesh->buildingTree = reinterpret_cast<void*>(RTree::createNewRTree(*reinterpret_cast<IStorageManager*>(pMesh->buildingStorage), fillFactor, indexCapacity, leafCapacity, demension, SpatialIndex::RTree::RV_RSTAR, indexId));
			}
#endif
		} else if (nType == TYPE_DATA_COMPLEX) {
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
			if (!pMesh->complexStorage && !pMesh->complexTree) {
				pMesh->complexStorage = reinterpret_cast<void*>(StorageManager::createNewMemoryStorageManager());
				pMesh->complexTree = reinterpret_cast<void*>(RTree::createNewRTree(*reinterpret_cast<IStorageManager*>(pMesh->complexStorage), fillFactor, indexCapacity, leafCapacity, demension, SpatialIndex::RTree::RV_RSTAR, indexId));
			}
#endif
		}
	}

	return ret;
}


int32_t MapMesh::CreateSpatialindexByStream(IN const int32_t nType, IN stMeshInfo* pMesh, vector<SpatialIndex::Region>& regions, vector<id_type>& ids)
{
	int32_t ret = 0;

	if (pMesh && !regions.empty() && !ids.empty()) {
#if defined(_DEBUG)
		uint32_t indexCapacity = 300;
		uint32_t leafCapacity = 300;
		double fillFactor = 0.7;
#else
		uint32_t indexCapacity = 100;
		uint32_t leafCapacity = 100;
		double fillFactor = 0.7;
#endif
		uint32_t demension = 2;

		id_type indexId;
		BulkDataStream stream(regions, ids);
		//stream.rewind(); // 반드시 호출

		if (nType == TYPE_DATA_TREKKING) {
#if defined(USE_FOREST_DATA)
			if (!pMesh->flinkStorage && !pMesh->flinkTree) {
				pMesh->flinkStorage = reinterpret_cast<void*>(StorageManager::createNewMemoryStorageManager());
				pMesh->flinkTree = reinterpret_cast<void*>(RTree::createAndBulkLoadNewRTree(SpatialIndex::RTree::BLM_STR, stream,
					*reinterpret_cast<IStorageManager*>(pMesh->flinkStorage), fillFactor, indexCapacity, leafCapacity, demension, SpatialIndex::RTree::RV_RSTAR, indexId));
			}
#endif
		} else if (nType == TYPE_DATA_PEDESTRIAN) {
#if defined(USE_PEDESTRIAN_DATA)
			if (!pMesh->wlinkStorage && !pMesh->wlinkTree) {
				pMesh->wlinkStorage = reinterpret_cast<void*>(StorageManager::createNewMemoryStorageManager());
				pMesh->wlinkTree = reinterpret_cast<void*>(RTree::createAndBulkLoadNewRTree(SpatialIndex::RTree::BLM_STR, stream,
					*reinterpret_cast<IStorageManager*>(pMesh->wlinkStorage), fillFactor, indexCapacity, leafCapacity, demension, SpatialIndex::RTree::RV_RSTAR, indexId));
		}
#endif
		} else if (nType == TYPE_DATA_VEHICLE) {
#if defined(USE_VEHICLE_DATA)
			if (!pMesh->vlinkStorage && !pMesh->vlinkTree) {
				pMesh->vlinkStorage = reinterpret_cast<void*>(StorageManager::createNewMemoryStorageManager());
				pMesh->vlinkTree = reinterpret_cast<void*>(RTree::createAndBulkLoadNewRTree(SpatialIndex::RTree::BLM_STR, stream,
					*reinterpret_cast<IStorageManager*>(pMesh->vlinkStorage), fillFactor, indexCapacity, leafCapacity, demension, SpatialIndex::RTree::RV_RSTAR, indexId));
			}
#endif
		} else if (nType == TYPE_DATA_BUILDING) {
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
			if (!pMesh->buildingStorage && !pMesh->buildingTree) {
				pMesh->buildingStorage = reinterpret_cast<void*>(StorageManager::createNewMemoryStorageManager());
				pMesh->buildingTree = reinterpret_cast<void*>(RTree::createAndBulkLoadNewRTree(SpatialIndex::RTree::BLM_STR, stream,
					*reinterpret_cast<IStorageManager*>(pMesh->buildingStorage), fillFactor, indexCapacity, leafCapacity, demension, SpatialIndex::RTree::RV_RSTAR, indexId));
			}
#endif
		} else if (nType == TYPE_DATA_COMPLEX) {
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
			if (!pMesh->complexStorage && !pMesh->complexTree) {
				pMesh->complexStorage = reinterpret_cast<void*>(StorageManager::createNewMemoryStorageManager());
				pMesh->complexTree = reinterpret_cast<void*>(RTree::createAndBulkLoadNewRTree(SpatialIndex::RTree::BLM_STR, stream,
					*reinterpret_cast<IStorageManager*>(pMesh->complexStorage), fillFactor, indexCapacity, leafCapacity, demension, SpatialIndex::RTree::RV_RSTAR, indexId));
			}
#endif
		}
	}

	return ret;
}


int32_t	MapMesh::DeleteSpatialindex(IN stMeshInfo* pMesh)
{
	int32_t ret = 0;

	if (pMesh) {
#if defined(USE_FOREST_DATA)
		SAFE_DELETE(pMesh->flinkStorage);
		SAFE_DELETE(pMesh->flinkTree);
#endif

#if defined(USE_PEDESTRIAN_DATA)
		SAFE_DELETE(pMesh->wlinkStorage);
		SAFE_DELETE(pMesh->wlinkTree);
#endif

#if defined(USE_VEHICLE_DATA)
		SAFE_DELETE(pMesh->vlinkStorage);
		SAFE_DELETE(pMesh->vlinkTree);
#endif

#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
		SAFE_DELETE(pMesh->buildingStorage);
		SAFE_DELETE(pMesh->buildingTree);

		SAFE_DELETE(pMesh->complexStorage);
		SAFE_DELETE(pMesh->complexTree);
#endif
	}

	return ret;
}


int32_t MapMesh::GetNearLinksfromSpatialindex(IN stMeshInfo* pMesh, IN const double lng, IN const double lat, IN const int nMaxBuff, OUT vector<KeyID>& vtLinkId)
{
	int32_t ret = 0;

	if (pMesh) {
		uint32_t nMaxCount = 100;
		double coords[2] = { lng, lat };
		Point queryPoint(coords, 2);

		double dwMaxDist = 0.f;
		if (nMaxBuff > 0) {
			dwMaxDist = nMaxBuff * 0.00001; // WGS84
		}

#if defined(USE_FOREST_DATA)		
		if (pMesh->flinkTree) {
			LinkVisitor visitor;
			ISpatialIndex * pTree = reinterpret_cast<ISpatialIndex*>(pMesh->flinkTree);
			pTree->nearestNeighborQuery(nMaxCount, queryPoint, visitor, dwMaxDist);

			for (const auto& key : visitor.results) {
				vtLinkId.emplace_back(KeyID{ static_cast<uint64_t>(key) });
			}
		}
#endif

#if defined(USE_PEDESTRIAN_DATA)
		if (pMesh->wlinkTree) {
			LinkVisitor visitor;
			ISpatialIndex * pTree = reinterpret_cast<ISpatialIndex*>(pMesh->wlinkTree);
			pTree->nearestNeighborQuery(nMaxCount, queryPoint, visitor, dwMaxDist);

			for (const auto& key : visitor.results) {
				vtLinkId.emplace_back(KeyID{ static_cast<uint64_t>(key) });
			}
		}
#endif

#if defined(USE_VEHICLE_DATA)
		if (pMesh->vlinkTree) {
			LinkVisitor visitor;
			ISpatialIndex * pTree = reinterpret_cast<ISpatialIndex*>(pMesh->vlinkTree);
			pTree->nearestNeighborQuery(nMaxCount, queryPoint, visitor, dwMaxDist);

			for (const auto& key : visitor.results) {
				vtLinkId.emplace_back(KeyID{ static_cast<uint64_t>(key) });
			}
		}
#endif

		ret = vtLinkId.size();
	}

	return ret;
}

int32_t MapMesh::GetPitinPolygonfromSpatialindex(IN stMeshInfo* pMesh, IN const double lng, IN const double lat, IN const int type, OUT vector<KeyID>& vtPolygonId)
{
	int32_t ret = 0;

	if (pMesh) {
#if defined(USE_OPTIMAL_POINT_API) || defined(USE_MOUNTAIN_DATA)
		double coords[2] = { lng, lat };
		SpatialIndex::Point queryPoint(coords, 2);

		PointContainmentVisitor visitor(queryPoint);
		ISpatialIndex * pTree = nullptr;
		if ((type == TYPE_DATA_BUILDING) && pMesh->buildingTree){
			pTree = reinterpret_cast<ISpatialIndex*>(pMesh->buildingTree);
		} else { // if ((type == TYPE_ENT_COMPLEX) && pMesh->complexTree) {
			pTree = reinterpret_cast<ISpatialIndex*>(pMesh->complexTree);
		}

		if (pTree) {
			pTree->intersectsWithQuery(queryPoint, visitor);

			for (const auto& key : visitor.results) {
				vtPolygonId.emplace_back(KeyID{ static_cast<uint64_t>(key) });
			}
		}
#endif

		ret = vtPolygonId.size();
	}

	return ret;
}
#endif // #ifdef TEST_SPATIALINDEX
