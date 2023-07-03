#if defined(_WIN32)
#include "../stdafx.h"
#else

#endif

#include "FileManager.h"

#include "../shp/shpio.h"
#include "../utils/UserLog.h"
#include "../utils/GeoTools.h"
#include "../route/MMPoint.hpp"

#include <queue>


#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFileManager::CFileManager()
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

//#if defined (USE_PEDESTRIAN_DATA)
//	m_trackShpMgr.SetFileManager(this);
//#endif
}

CFileManager::~CFileManager()
{
	//_CrtDumpMemoryLeaks();
}


bool CFileManager::Initialize(void)
{

	return true;
}

void CFileManager::Release(void)
{

}


void CFileManager::SetDataMgr(CDataManager* pDataMgr)
{
	if (pDataMgr)
	{
		m_fileName.SetDataManager(pDataMgr);
		m_fileMesh.SetDataManager(pDataMgr);

#if defined(USE_TREKKING_DATA)
		m_fileTrekking.SetDataManager(pDataMgr);
		m_fileTrekking.SetNameManager(&m_fileName);
#endif
#if defined(USE_PEDESTRIAN_DATA)
		m_filePedestrian.SetDataManager(pDataMgr);
		m_filePedestrian.SetNameManager(&m_fileName);
#endif
#if defined(USE_VEHICLE_DATA)
		m_fileVehicle.SetDataManager(pDataMgr);
		m_fileVehicle.SetNameManager(&m_fileName);

#if defined(USE_TREKKING_POINT_API)
		m_fileTraffic.SetDataManager(pDataMgr);
#endif
#endif
#if defined(USE_COMPLEX_DATA)
		m_fileComplex.SetDataManager(pDataMgr);
		m_fileComplex.SetNameManager(&m_fileName);
#endif
#if defined(USE_BUILDING_DATA)
		m_fileBuilding.SetDataManager(pDataMgr);
		m_fileBuilding.SetNameManager(&m_fileName);
		m_fileBuilding.SetFileComplex(&m_fileComplex);
#endif
#if defined(USE_COMPLEX_DATA) | defined(USE_BUILDING_DATA)
		m_fileEntrance.SetDataManager(pDataMgr);
		m_fileEntrance.SetFileComplex(&m_fileComplex);
		m_fileEntrance.SetFileBuilding(&m_fileBuilding);
#endif
	}
}


const SBox* CFileManager::GetMeshRegion(void) const
{
	// total area
	return &m_rtBox;
}


bool CFileManager::OpenFile(IN const char* pszFilePath, IN const uint32_t nFileType)
{
	if (!pszFilePath || TYPE_DATA_COUNT <= nFileType) {
		LOG_TRACE(LOG_ERROR, "Failed, can't open file, file:%s, type:%d", pszFilePath, nFileType);
		return false;
	}

	switch (nFileType)
	{
	case TYPE_DATA_NAME:
		m_fileName.OpenFile(pszFilePath);
		break;

	case TYPE_DATA_MESH:
		m_fileMesh.OpenFile(pszFilePath);
		break;

#if defined(USE_TREKKING_DATA)
	case TYPE_DATA_TREKKING:
		m_fileTrekking.OpenFile(pszFilePath);
		break;
#endif

#if defined(USE_PEDESTRIAN_DATA)
	case TYPE_DATA_PEDESTRIAN:
		m_filePedestrian.OpenFile(pszFilePath);
		break;
#endif

#if defined(USE_VEHICLE_DATA)
	case TYPE_DATA_VEHICLE:
		m_fileVehicle.OpenFile(pszFilePath);
		break;

#if defined(USE_TREKKING_POINT_API)
	case TYPE_DATA_TRAFFIC:
		m_fileTraffic.OpenFile(pszFilePath);
		break;
#endif
#endif

#if defined(USE_COMPLEX_DATA)
	case TYPE_DATA_COMPLEX:
		m_fileComplex.OpenFile(pszFilePath);
		break;
#endif

#if defined(USE_BUILDING_DATA)
	case TYPE_DATA_BUILDING:
		m_fileBuilding.OpenFile(pszFilePath);
		break;
#endif

#if defined(USE_COMPLEX_DATA) | defined(USE_BUILDING_DATA)
	case TYPE_DATA_ENTRANCE:
		m_fileEntrance.OpenFile(pszFilePath);
		break;
#endif

	default:
		break;
	}

	return true;
}


bool CFileManager::SaveData(IN const char* pszFilePath)
{
	m_fileName.SaveData(pszFilePath);

	m_fileMesh.SaveData(pszFilePath);

#if defined(USE_TREKKING_DATA)
	m_fileTrekking.SaveData(pszFilePath);
#endif
#if defined(USE_PEDESTRIAN_DATA)
	m_filePedestrian.SaveData(pszFilePath);
#endif
#if defined(USE_VEHICLE_DATA)
	m_fileVehicle.SaveData(pszFilePath);

#if defined(USE_TREKKING_POINT_API)
	m_fileTraffic.SaveData(pszFilePath);
#endif
#endif
#if defined(USE_COMPLEX_DATA)
	m_fileComplex.SaveData(pszFilePath);
#endif
#if defined(USE_BUILDING_DATA)
	m_fileBuilding.SaveData(pszFilePath);
#endif
#if defined(USE_COMPLEX_DATA) | defined(USE_BUILDING_DATA)
	m_fileEntrance.SaveData(pszFilePath);
#endif

	return true;
}


bool CFileManager::LoadData(IN const char* pszFilePath)
{
	m_fileName.LoadData(pszFilePath);

	m_fileMesh.LoadData(pszFilePath);

#if defined(USE_TREKKING_DATA)
	if (m_fileTrekking.LoadData(pszFilePath) == true) {
		memcpy(&m_rtBox, m_fileTrekking.GetMeshRegion(), sizeof(m_rtBox));
	}
#endif

#if defined(USE_PEDESTRIAN_DATA)
	if (m_filePedestrian.LoadData(pszFilePath) == true) {
		memcpy(&m_rtBox, m_filePedestrian.GetMeshRegion(), sizeof(m_rtBox));
	}
#endif

#if defined(USE_VEHICLE_DATA)
	if (m_fileVehicle.LoadData(pszFilePath) == true) {
		memcpy(&m_rtBox, m_fileVehicle.GetMeshRegion(), sizeof(m_rtBox));
	}

#if defined(USE_TREKKING_POINT_API)
	if (m_fileTraffic.LoadData(pszFilePath) == true) {
		;
	}
#endif
#endif

#if defined(USE_COMPLEX_DATA)
	m_fileComplex.LoadData(pszFilePath);
#endif
#if defined(USE_BUILDING_DATA)
	m_fileBuilding.LoadData(pszFilePath);
#endif
#if defined(USE_COMPLEX_DATA) | defined(USE_BUILDING_DATA)
	m_fileEntrance.LoadData(pszFilePath);
#endif

	return true;
}


bool CFileManager::LoadDataByIdx(IN const uint32_t idx)
{
	m_fileName.LoadDataByIdx(idx);

#if defined(USE_TREKKING_DATA)
	m_fileTrekking.LoadDataByIdx(idx);
#endif
#if defined(USE_PEDESTRIAN_DATA)
	m_filePedestrian.LoadDataByIdx(idx);
#endif
#if defined(USE_VEHICLE_DATA)
	m_fileVehicle.LoadDataByIdx(idx);

#if defined(USE_TREKKING_POINT_API)
	m_fileTraffic.LoadDataByIdx(idx);	
#endif
#endif
#if defined(USE_COMPLEX_DATA)
	m_fileComplex.LoadDataByIdx(idx);
#endif
#if defined(USE_BUILDING_DATA)
	m_fileBuilding.LoadDataByIdx(idx);
#endif
#if defined(USE_COMPLEX_DATA) | defined(USE_BUILDING_DATA)
	m_fileEntrance.LoadDataByIdx(idx);
#endif

	return true;
}


bool CFileManager::GetData(IN const uint32_t idTile)
{
	bool ret = false;

	m_fileMesh.LoadDataByIdx(idTile);

#if defined(USE_TREKKING_DATA)
	ret |= m_fileTrekking.LoadDataByIdx(idTile);
#endif

#if defined(USE_PEDESTRIAN_DATA)
	ret |= m_filePedestrian.LoadDataByIdx(idTile);
#endif

#if defined(USE_VEHICLE_DATA)
	ret |= m_fileVehicle.LoadDataByIdx(idTile);

#if defined(USE_TREKKING_POINT_API)
	ret |= m_fileTraffic.LoadDataByIdx(idTile);	
#endif
#endif

#if defined(USE_COMPLEX_DATA)
	ret |= m_fileComplex.LoadDataByIdx(idTile);
#endif

#if defined(USE_BUILDING_DATA)
	ret |= m_fileBuilding.LoadDataByIdx(idTile);
#endif

#if defined(USE_COMPLEX_DATA) | defined(USE_BUILDING_DATA)
	ret |= m_fileEntrance.LoadDataByIdx(idTile);
#endif

	return ret;
}


const char* CFileManager::GetErrorMsg()
{
	if (!m_strErrMsg.empty())
		return m_strErrMsg.c_str();

	return nullptr;
}


