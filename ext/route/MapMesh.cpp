
#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "MapMesh.h"
#include "../utils/UserLog.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


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
	if (!mapData.empty())
	{
		for (map<uint64_t, stMeshInfo*>::iterator it = mapData.begin(); it != mapData.end(); it++)
		{
			delete it->second;
			it->second = nullptr;
		}

		mapData.clear();
		map<uint64_t, stMeshInfo*>().swap(mapData);
	}

	MapBase::Release();
}


void MapMesh::SetBox(IN const SBox* pBox)
{
	memcpy(&m_rtBox, pBox, sizeof(m_rtBox));
}


int MapMesh::AddData(IN const stMeshInfo * pData)
{
	static const int nMaxNeighbor = 8;
	int nCurNeighbor = 0;

	if (pData != nullptr) {
		mapData.emplace(pData->mesh_id.tile_id, const_cast<stMeshInfo*>(pData));
	}
	
	return mapData.size();
}


bool MapMesh::DeleteData(IN const uint64_t id)
{
	map<uint64_t, stMeshInfo*>::const_iterator it = mapData.find(id);
	if (it != mapData.end()) {
		try {
			delete it->second;
			mapData.erase(it);
		}
		catch(exception e) {
			LOG_TRACE(LOG_ERROR, "Error, delete mesh, %s", e.what());
		}

		return true;
	}

	return false;
}


int MapMesh::InsertNode(IN const KeyID keyId)
{
	stMeshInfo* pMesh = GetMeshById(keyId.tile_id);
	if (pMesh) {
#if 1//defined(_DEBUG)
		auto it = pMesh->setNodeDuplicateCheck.insert(keyId);
		if (!it.second)
		{
			LOG_TRACE(LOG_ERROR, "Error, --------------- node id duplicated in MapMesh, %d", keyId.nid);
		}
#endif
		pMesh->nodes.emplace_back(keyId);
		return pMesh->nodes.size();
	}

	return 0;
}


int MapMesh::InsertLink(IN const stLinkInfo * pData)
{
	if (pData != nullptr)
	{
		stMeshInfo* pMesh = GetMeshById(pData->link_id.tile_id);
		if (pMesh) {
#if 1// defined(_DEBUG)
			auto it = pMesh->setLinkDuplicateCheck.insert(pData->link_id);
			if (!it.second)
			{
				LOG_TRACE(LOG_ERROR, "Error, --------------- link id duplicated in MapMesh, %d", pData->link_id.nid);
			}
#endif
			pMesh->links.emplace_back(pData->link_id);

#pragma omp parallel for
			for (int ii = 0; ii < pData->getVertexCount(); ii++)
			{
				ExtendDataBox(pMesh, pData->getVertexX(ii), pData->getVertexY(ii));
			}

			return pMesh->links.size();
		}
	}

	return 0;
}


#if defined(USE_PEDESTRIAN_DATA)
int MapMesh::InsertWNode(IN const KeyID keyId)
{
	stMeshInfo* pMesh = GetMeshById(keyId.tile_id);
	if (pMesh) {
#if 1//defined(_DEBUG)
		auto it = pMesh->setWNodeDuplicateCheck.insert(keyId);
		if (!it.second)
		{
			LOG_TRACE(LOG_ERROR, "Error, --------------- node id duplicated in MapMesh, %d", keyId.nid);
		}
#endif
		pMesh->wnodes.emplace_back(keyId);
		return pMesh->wnodes.size();
	}

	return 0;
}

int MapMesh::InsertWLink(IN const stLinkInfo * pData)
{
	if (pData != nullptr)
	{
		stMeshInfo* pMesh = GetMeshById(pData->link_id.tile_id);
		if (pMesh) {
#if 1// defined(_DEBUG)
			auto it = pMesh->setWLinkDuplicateCheck.insert(pData->link_id);
			if (!it.second)
			{
				LOG_TRACE(LOG_ERROR, "Error, --------------- link id duplicated in MapMesh, %d", pData->link_id.nid);
			}
#endif
			pMesh->wlinks.emplace_back(pData->link_id);

#pragma omp parallel for
			for (int ii = 0; ii < pData->getVertexCount(); ii++)
			{
				ExtendDataBox(pMesh, pData->getVertexX(ii), pData->getVertexY(ii));
			}

			return pMesh->wlinks.size();
		}
	}

	return 0;
}
#endif


#if defined(USE_VEHICLE_DATA)
int MapMesh::InsertVNode(IN const KeyID keyId)
{
	stMeshInfo* pMesh = GetMeshById(keyId.tile_id);
	if (pMesh) {
#if 1//defined(_DEBUG)
		auto it = pMesh->setVNodeDuplicateCheck.insert(keyId);
		if (!it.second)
		{
			LOG_TRACE(LOG_ERROR, "Error, --------------- node id duplicated in MapMesh, %d", keyId.nid);
		}
#endif
		pMesh->vnodes.emplace_back(keyId);
		return pMesh->vnodes.size();
	}

	return 0;
}

int MapMesh::InsertVLink(IN const stLinkInfo * pData)
{
	if (pData != nullptr)
	{
		stMeshInfo* pMesh = GetMeshById(pData->link_id.tile_id);
		if (pMesh) {
#if 1// defined(_DEBUG)
			auto it = pMesh->setVLinkDuplicateCheck.insert(pData->link_id);
			if (!it.second)
			{
				LOG_TRACE(LOG_ERROR, "Error, --------------- link id duplicated in MapMesh, %d", pData->link_id.nid);
			}
#endif
			pMesh->vlinks.emplace_back(pData->link_id);

#pragma omp parallel for
			for (int ii = 0; ii < pData->getVertexCount(); ii++)
			{
				ExtendDataBox(pMesh, pData->getVertexX(ii), pData->getVertexY(ii));
			}

			return pMesh->vlinks.size();
		}
	}

	return 0;
}
#endif

# if defined(USE_OPTIMAL_POINT_API)
bool MapMesh::InsertComplex(IN const stPolygonInfo * pData)
{
	stMeshInfo* pMesh = GetMeshById(pData->poly_id.tile_id);
	if (pMesh) {
#if 1//defined(_DEBUG)
		auto it = pMesh->setCpxDuplicateCheck.insert(pData->poly_id);
		if (!it.second)
		{
			LOG_TRACE(LOG_ERROR, "Error, --------------- complex id duplicated in MapMesh, %d", pData->poly_id.nid);
		}
#endif
		pMesh->complexs.emplace_back(pData->poly_id);

		const SPoint* pPolygon = pData->getAttributeVertex();
#pragma omp parallel for
		//for (int ii = 0; ii < pData->vtVtx.size(); ii++)
		//{
		//	ExtendDataBox(pMesh, pData->vtVtx[ii].x, pData->vtVtx[ii].y);
		//}
		for (int ii = pData->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX) - 1; ii >= 0 ; --ii)
		{
			ExtendDataBox(pMesh, pPolygon[ii]);
		}

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
		pMesh->buildings.emplace_back(pData->poly_id);

		const SPoint* pPolygon = pData->getAttributeVertex();
#pragma omp parallel for
		//for (int ii = 0; ii < pData->vtVtx.size(); ii++)
		//{
		//	ExtendDataBox(pMesh, pData->vtVtx[ii].x, pData->vtVtx[ii].y);
		//}
		for (int ii = pData->getAttributeCount(TYPE_POLYGON_DATA_ATTR_VTX) - 1; ii >= 0; --ii)
		{
			ExtendDataBox(pMesh, pPolygon[ii]);
		}

		return true;
	}

	return false;
}
#endif

stMeshInfo * MapMesh::GetMeshById(IN int32_t meshId)
{
	map<uint64_t, stMeshInfo*>::const_iterator it = mapData.find(meshId);
	if (it != mapData.end()) {
		return &*mapData.find(meshId)->second;
	}

	return nullptr;	
}

stMeshInfo* MapMesh::GetMeshByPoint(IN const double lng, IN const double lat)
{
	stMeshInfo* pMesh = nullptr;

	if (lng != 0.f && lat != 0.f) 
	{
		for (map<uint64_t, stMeshInfo*>::const_iterator it = mapData.begin(); it != mapData.end(); it++)
		{
			if (lng < it->second->mesh_box.Xmin || it->second->mesh_box.Xmax < lng
				|| lat < it->second->mesh_box.Ymin || it->second->mesh_box.Ymax < lat)
			{
				continue;
			}

			pMesh = it->second;
			
			break;
		}
	}

	return pMesh;
}

int MapMesh::GetPitInMesh(IN const double lng, IN const double lat, IN const int nMaxBuff, OUT stMeshInfo** pData)
{
	int cntMesh = 0;

	if (nMaxBuff > 0 && pData != nullptr)
	{
		for (map<uint64_t, stMeshInfo*>::const_iterator it = mapData.begin(); it != mapData.end(); it++)
		{
			if (lng < it->second->mesh_box.Xmin || it->second->mesh_box.Xmax < lng
				|| lat < it->second->mesh_box.Ymin || it->second->mesh_box.Ymax < lat)
			{
				continue;
			}

			pData[cntMesh++] = it->second;

			if (cntMesh >= nMaxBuff)
				break;
		}
	}

	return cntMesh;
}

int MapMesh::GetPitInData(IN const double lng, IN const double lat, IN const int nMaxBuff, OUT stMeshInfo** pData)
{
	int cntMesh = 0;

	if (nMaxBuff > 0 && pData != nullptr)
	{
		for (map<uint64_t, stMeshInfo*>::const_iterator it = mapData.begin(); it != mapData.end(); it++)
		{
			if (lng < it->second->data_box.Xmin || it->second->data_box.Xmax < lng
				|| lat < it->second->data_box.Ymin || it->second->data_box.Ymax < lat)
			{
				continue;
			}

			pData[cntMesh++] = it->second;

			if (cntMesh >= nMaxBuff)
				break;
		}
	}

	return cntMesh;
}


const uint32_t MapMesh::GetCount(void) const
{
	return mapData.size();
}

stMeshInfo * MapMesh::GetMeshData(IN const uint32_t idx)
{
	if (0 <= idx && idx < mapData.size())
	{
		map<uint64_t, stMeshInfo*>::const_iterator it = mapData.begin();
		advance(it, idx);

		//if (it->second->mesh_type != NOT_USE)
			return it->second;
	}

	return nullptr;
}

void MapMesh::ExtendDataBox(IN stMeshInfo * pData, IN const double lng, IN const double lat) const
{
	if (pData != nullptr)
	{
		if (lng < pData->data_box.Xmin || pData->data_box.Xmin == 0) { pData->data_box.Xmin = lng; }
		if (pData->data_box.Xmax < lng || pData->data_box.Xmax == 0) { pData->data_box.Xmax = lng; }
		if (lat < pData->data_box.Ymin || pData->data_box.Ymin == 0) { pData->data_box.Ymin = lat; }
		if (pData->data_box.Ymax < lat || pData->data_box.Ymax == 0) { pData->data_box.Ymax = lat; }
	}
}

void MapMesh::ExtendDataBox(IN stMeshInfo * pData, IN const SPoint& coord) const
{
	if (pData != nullptr) {
		return ExtendDataBox(pData, coord.x, coord.y);
	}
}

void MapMesh::CheckNeighborMesh(void)
{
	static const int nMaxNeighbor = 8;

	// find neighbor

	// 소수점 단위가 오차가 있을 수 있어, 10만 곱한 영역을 비교하자	
	for (map<uint64_t, stMeshInfo*>::iterator itMe = mapData.begin(); itMe != mapData.end(); itMe++)
	{
		RECT rtMe = { itMe->second->mesh_box.Xmin * 100000, itMe->second->mesh_box.Ymin * 100000, itMe->second->mesh_box.Xmax * 100000, itMe->second->mesh_box.Ymax * 100000 };

		if (itMe->second->links.empty()) {
			continue;
		}

		for (map<uint64_t, stMeshInfo*>::iterator itYou = mapData.begin(); itYou != mapData.end(); itYou++)
		{
			if (itMe->second->mesh_id.tile_id == itYou->second->mesh_id.tile_id ||
				itYou->second->links.empty()) {
				continue;
			}

			bool isNeighbor = false;
			RECT rtYou = { itYou->second->mesh_box.Xmin * 100000, itYou->second->mesh_box.Ymin * 100000, itYou->second->mesh_box.Xmax * 100000, itYou->second->mesh_box.Ymax * 100000 };


			// LT == old->lt, t, l 
			if ((rtMe.left == rtYou.right && rtMe.top == rtYou.top) ||
				(rtMe.left == rtYou.left && rtMe.top == rtYou.bottom) ||
				(rtMe.left == rtYou.right && rtMe.top == rtYou.bottom))
			{
				isNeighbor = true;
			}
			// RT == old->rt, t, r 
			else if ((rtMe.right == rtYou.left && rtMe.top == rtYou.bottom) ||
				(rtMe.right == rtYou.right && rtMe.top == rtYou.bottom) ||
				(rtMe.right == rtYou.left && rtMe.top == rtYou.bottom))
			{
				isNeighbor = true;
			}
			// LB == old->lb, b, l 
			else if ((rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom) ||
				(rtMe.left == rtYou.left && rtMe.bottom == rtYou.top) ||
				(rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom))
			{
				isNeighbor = true;
			}
			// RB == old->rb, b, r 
			else if ((rtMe.right == rtYou.left && rtMe.bottom == rtYou.top) ||
				(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top) ||
				(rtMe.right == rtYou.left && rtMe.bottom == rtYou.top))
			{
				isNeighbor = true;
			}
			// L
			else if ((rtMe.left == rtYou.right && rtMe.top == rtYou.top) ||
				(rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom))
			{
				isNeighbor = true;
			}
			// R
			else if ((rtMe.right == rtYou.left && rtMe.top == rtYou.top) ||
				(rtMe.right == rtYou.left && rtMe.bottom == rtYou.bottom))
			{
				isNeighbor = true;
			}
			// T
			else if ((rtMe.left == rtYou.left && rtMe.top == rtYou.bottom) ||
				(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top))
			{
				isNeighbor = true;
			}
			// B
			else if ((rtMe.left == rtYou.left && rtMe.bottom == rtYou.top) ||
				(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top))
			{
				isNeighbor = true;
			}

			// Box edge
			else {
				rtYou.left = m_rtBox.Xmin * 100000;
				rtYou.top = m_rtBox.Ymin * 100000;
				rtYou.right = m_rtBox.Xmax * 100000;
				rtYou.bottom = m_rtBox.Ymax * 100000;

				// LT == old->lt, t, l 
				if ((rtMe.left == rtYou.right && rtMe.top == rtYou.top) ||
					(rtMe.left == rtYou.left && rtMe.top == rtYou.bottom) ||
					(rtMe.left == rtYou.right && rtMe.top == rtYou.bottom))
				{
					isNeighbor = true;
				}
				// RT == old->rt, t, r 
				else if ((rtMe.right == rtYou.left && rtMe.top == rtYou.bottom) ||
					(rtMe.right == rtYou.right && rtMe.top == rtYou.bottom) ||
					(rtMe.right == rtYou.left && rtMe.top == rtYou.bottom))
				{
					isNeighbor = true;
				}
				// LB == old->lb, b, l 
				else if ((rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom) ||
					(rtMe.left == rtYou.left && rtMe.bottom == rtYou.top) ||
					(rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom))
				{
					isNeighbor = true;
				}
				// RB == old->rb, b, r 
				else if ((rtMe.right == rtYou.left && rtMe.bottom == rtYou.top) ||
					(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top) ||
					(rtMe.right == rtYou.left && rtMe.bottom == rtYou.top))
				{
					isNeighbor = true;
				}
				// L
				else if ((rtMe.left == rtYou.right && rtMe.top == rtYou.top) ||
					(rtMe.left == rtYou.right && rtMe.bottom == rtYou.bottom))
				{
					isNeighbor = true;
				}
				// R
				else if ((rtMe.right == rtYou.left && rtMe.top == rtYou.top) ||
					(rtMe.right == rtYou.left && rtMe.bottom == rtYou.bottom))
				{
					isNeighbor = true;
				}
				// T
				else if ((rtMe.left == rtYou.left && rtMe.top == rtYou.bottom) ||
					(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top))
				{
					isNeighbor = true;
				}
				// B
				else if ((rtMe.left == rtYou.left && rtMe.bottom == rtYou.top) ||
					(rtMe.right == rtYou.right && rtMe.bottom == rtYou.top))
				{
					isNeighbor = true;
				}
			}


			if (isNeighbor)
			{
				itMe->second->neighbors.emplace((uint32_t)itYou->second->mesh_id.tile_id);
				itYou->second->neighbors.emplace((uint32_t)itMe->second->mesh_id.tile_id);

				if (itYou->second->neighbors.size() > nMaxNeighbor) {
					LOG_TRACE(LOG_ERROR, "Error, --------------- Mesh your neighbor count overflow, id:%d, %d", itYou->second->mesh_id.tile_id, itYou->second->neighbors.size());
				}
				if (itMe->second->neighbors.size() > nMaxNeighbor) {
					LOG_TRACE(LOG_ERROR, "Error, --------------- Mesh my neighbor count overflow, id:%d, %d", itMe->second->mesh_id.tile_id, itMe->second->neighbors.size());
				}
			}
		} // for		
	}
}


void MapMesh::ArrangementMesh(void)
{
	for (map<uint64_t, stMeshInfo*>::const_iterator it = mapData.begin(); it != mapData.end();) {
		if (!it->second->nodes.empty() || !it->second->links.empty()
#if defined(USE_PEDESTRIAN_DATA)
			|| !it->second->wnodes.empty() || !it->second->wlinks.empty()
#endif
#if defined(USE_VEHICLE_DATA)
			|| !it->second->vnodes.empty() || !it->second->vlinks.empty()
#endif
# if defined(USE_OPTIMAL_POINT_API)
			|| !it->second->complexs.empty() || !it->second->buildings.empty()
#endif
			) {
			it++;
		}
		else {
			//LOG_TRACE(LOG_DEBUG, "LOG, remove empty mesh : %d", it->first);
			mapData.erase(it++);
		}
	}
}