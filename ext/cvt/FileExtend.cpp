#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileManager.h"
#include "FileExtend.h"

#include "../shp/shpio.h"
#include "../utils/UserLog.h"
#include "../utils/Strings.h"
#include "../route/MMPoint.hpp"
#include "../route/DataManager.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

CFileExtend::CFileExtend()
{
#if defined(USE_PEDESTRIAN_DATA)	
	m_nDataType = TYPE_DATA_PEDESTRIAN;
#elif defined(USE_VEHICLE_DATA)
	m_nDataType = TYPE_DATA_VEHICLE;
#else
	m_nDataType = TYPE_DATA_VEHICLE;
#endif

	m_nFileType = TYPE_EXEC_EXTEND;
}

CFileExtend::~CFileExtend()
{

}


size_t CFileExtend::ReadBody(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody = { 0, };

	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeBody = sizeof(FileBody);

	for (int32_t idx = 0; idx < m_vtIndex.size(); idx++)
	{
		if (!checkTestMesh(m_vtIndex[idx].idTile)) {
			continue;
		}

		// read body
		if (m_vtIndex[idx].szBody <= 0) {
			continue;
		}
		else if (m_vtIndex[idx].offBody <= 0)
		{
			LOG_TRACE(LOG_ERROR, "Failed, index body info invalid, off:%d, size:%d", m_vtIndex[idx].offBody, m_vtIndex[idx].szBody);
			return 0;
		}

		fseek(fp, m_vtIndex[idx].offBody, SEEK_SET);
		if ((retRead = fread(&fileBody, sizeBody, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read body, offset:%d", m_vtIndex[idx].offBody);
			return 0;
		}

		if (m_vtIndex[idx].idTile != fileBody.idTile)
		{
			LOG_TRACE(LOG_ERROR, "Failed, index tile info not match with body, index tile id:%d vs body tile id:%d", m_vtIndex[idx].idTile, fileBody.idTile);
			return 0;
		}

		if (fileBody.szData <= 0) {
			continue;
		}

		int readItems = 0;
		int32_t cntNode = 0;
		int32_t cntLink = 0;
		if ((retRead = fread(&cntNode, sizeof(cntNode), 1, fp)) != 1) {
			LOG_TRACE(LOG_ERROR, "Failed, can't read node count");
			fclose(fp);
			return false;
		}
		readItems += sizeof(cntNode);

		if ((retRead = fread(&cntLink, sizeof(cntLink), 1, fp)) != 1) {
			LOG_TRACE(LOG_ERROR, "Failed, can't read link count");
			fclose(fp);
			return false;
		}
		readItems += sizeof(cntLink);

		// read for node
		for (int ii = 0; ii < cntNode; ii++) {
			stExtendInfo* pExtInfo = new stExtendInfo();
			int readItem = pExtInfo->read(fp);
			if (readItem > 0) {
				// add to map
				m_pDataMgr->AddNodeExData(pExtInfo, m_nDataType);
				readItems += readItem;
			}
		} // for

		  // read for link
		for (int ii = 0; ii < cntLink; ii++) {
			stExtendInfo* pExtInfo = new stExtendInfo();
			int readItem = pExtInfo->read(fp);
			if (readItem > 0) {
				// add to map
				m_pDataMgr->AddLinkExData(pExtInfo, m_nDataType);
				readItems += readItem;
			}
		} // for

		if (readItems != fileBody.szData) {
			LOG_TRACE(LOG_ERROR, "Failed, body data size not matching from read file size, tile_id:%d, body:%d vs file:%d", fileBody.idTile, fileBody.szData, readItems);
			return 0;
		}

		offFile += m_vtIndex[idx].szBody;
	} // for

	return offFile;
}


bool CFileExtend::LoadDataByIdx(IN const uint32_t idx)
{
	FILE* fp = fopen(m_szDataPath, "rb");
	if (!fp)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't open file for load data, file:%s", m_szDataPath);
		return false;
	}
	else if (idx < 0 || m_vtIndex.size() <= idx) {
		LOG_TRACE(LOG_ERROR, "Failed, request load data idx range, max:%s, req idx:%d", m_vtIndex.size(), idx);
		fclose(fp);
		return false;
	}

	FileBody fileBody = { 0, };

	const size_t sizeBody = sizeof(FileBody);

	size_t offFile = 0;
	size_t offItem = 0;
	size_t retRead = 0;

	// read body
	if (m_vtIndex[idx].offBody <= 0 || m_vtIndex[idx].szBody <= 0)
	{
		//LOG_TRACE(LOG_ERROR, "Failed, index body info invalid, off:%d, size:%d", m_vtIndex[idx].offBody, m_vtIndex[idx].szBody);
		fclose(fp);
		return false;
	}

	fseek(fp, m_vtIndex[idx].offBody, SEEK_SET);
	if ((retRead = fread(&fileBody, sizeof(fileBody), 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't read body, offset:%d", m_vtIndex[idx].offBody);
		fclose(fp);
		return false;
	}

	if (m_vtIndex[idx].idTile != fileBody.idTile)
	{
		LOG_TRACE(LOG_ERROR, "Failed, index tile info not match with body, index tile id:%d vs body tile id:%d", m_vtIndex[idx].idTile, fileBody.idTile);
		fclose(fp);
		return false;
	}

	if (fileBody.szData <= 0) {
		return false;
	}

	int readItems = 0;
	int32_t cntNode = 0;
	int32_t cntLink = 0;
	if ((retRead = fread(&cntNode, sizeof(cntNode), 1, fp)) != 1) {
		LOG_TRACE(LOG_ERROR, "Failed, can't read node count");
		fclose(fp);
		return false;
	}
	readItems += sizeof(cntNode);

	if ((retRead = fread(&cntLink, sizeof(cntLink), 1, fp)) != 1) {
		LOG_TRACE(LOG_ERROR, "Failed, can't read link count");
		fclose(fp);
		return false;
	}
	readItems += sizeof(cntLink);

	// read for node
	for (int ii = 0; ii < cntNode; ii++) {
		stExtendInfo* pExtInfo = new stExtendInfo();
		int readItem = pExtInfo->read(fp);
		if (readItem > 0) {
			// add to map
			m_pDataMgr->AddNodeExData(pExtInfo, m_nDataType);
			readItems += readItem;
		}
	} // for

	// read for link
	for (int ii = 0; ii < cntLink; ii++) {
		stExtendInfo* pExtInfo = new stExtendInfo();
		int readItem = pExtInfo->read(fp);
		if (readItem > 0) {
			// add to map
			m_pDataMgr->AddLinkExData(pExtInfo, m_nDataType);
			readItems += readItem;
		}
	} // for

	if (readItems != fileBody.szData) {
		LOG_TRACE(LOG_ERROR, "Failed, body data size not matching from read file size, tile_id:%d, body:%d vs file:%d", fileBody.idTile, fileBody.szData, readItems);
		return 0;
	}

	offFile += m_vtIndex[idx].szBody;

	fclose(fp);

	return true;
}


bool CFileExtend::Initialize()
{
	return CFileBase::Initialize();
}


void CFileExtend::Release()
{	
	CFileBase::Release();
}


size_t CFileExtend::WriteBody(FILE* fp, IN const uint32_t fileOff)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	FileBody fileBody;

	size_t offFile = fileOff;
	size_t retWrite = 0;
	size_t retRead = 0;
	long offItem = 0;
	const size_t sizeFileBody = sizeof(fileBody);

	uint32_t ii;


	
	// write body
	stMeshInfo* pMesh = nullptr;
	int cntMesh = GetMeshCount();
	for (ii = 0; ii < cntMesh; ii++) {
		pMesh = GetMeshData(ii);
		if (!pMesh) {
			LOG_TRACE(LOG_ERROR, "Failed, can't access mesh, idx:%d", ii);
			return 0;
		}

		// write body
		memset(&fileBody, 0x00, sizeFileBody);

		fileBody.idTile = pMesh->mesh_id.tile_id;

		if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1) {
			LOG_TRACE(LOG_ERROR, "Failed, can't write body[%d], written:%d", ii, retWrite);
			return 0;
		}
		offItem = sizeFileBody;

		MapExtend* pMapLinkEx = m_pDataMgr->GetMapExtend();
		if (!pMapLinkEx) {
			LOG_TRACE(LOG_ERROR, "Failed, MapLinkEx pointer disabled");
			return 0;
		}

		const stExtendMeshInfo* pMapMesh = pMapLinkEx->GetMapMeshData(pMesh->mesh_id.tile_id);
		if (pMapMesh != nullptr) {
			// write data count
			int32_t cntItem = pMapMesh->mapNode.size();
			if ((retWrite = fwrite(&cntItem, sizeof(cntItem), 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't write map node size, written:%d", retWrite);
				return 0;
			}
			fileBody.szData += sizeof(cntItem);

			cntItem = pMapMesh->mapLink.size();
			if ((retWrite = fwrite(&cntItem, sizeof(cntItem), 1, fp)) != 1) {
				LOG_TRACE(LOG_ERROR, "Failed, can't write map link size, written:%d", retWrite);
				return 0;
			}
			fileBody.szData += sizeof(cntItem);

			// write data
			for (const auto& item : pMapMesh->mapNode) {
				fileBody.szData += item.second->write(fp);
				fileBody.cntData++;
			}

			for (const auto& item : pMapMesh->mapLink) {
				fileBody.szData += item.second->write(fp);
				fileBody.cntData++;
			}

			offItem += fileBody.szData;

			// re-write body size & off
			if (offItem > sizeFileBody) {
				fseek(fp, offItem * -1, SEEK_CUR);

				if ((retWrite = fwrite(&fileBody, sizeFileBody, 1, fp)) != 1) {
					LOG_TRACE(LOG_ERROR, "Failed, can't re-write body[%d], written:%d", ii, retWrite);
					return 0;
				}

				fseek(fp, offItem - sizeFileBody, SEEK_CUR);
			}
		} else {
#if defined(__DEBUG)
			LOG_TRACE(LOG_DEBUG, "mesh dosen't have any extend data, mesh_id:%d", pMesh->mesh_id.tile_id);
#endif

		}

		// re-set index size & off buff
		m_vtIndex[ii].szBody = offItem;
		m_vtIndex[ii].offBody = offFile;

		// update file offset
		offFile += offItem;
	} // for


	// re-write index for body size and offset
	fseek(fp, m_fileHeader.offIndex, SEEK_SET);
	for (uint32_t idx = 0; idx < m_fileHeader.cntIndex; idx++)
	{
		if ((retRead = fwrite(&m_vtIndex[idx], sizeof(FileIndex), 1, fp)) != 1) {
			LOG_TRACE(LOG_ERROR, "Failed, can't re-write file index data, idx:%d", idx);
			return 0;
		}
	}

	return offFile;
}


