#pragma once

#include "../route/MapDef.h"

#include "FileName.h"
#include "FileMesh.h"
#include "FileTrekking.h"
#include "FilePedestrian.h"
#include "FileVehicle.h"
#include "FileComplex.h"
#include "FileBuilding.h"
#include "FileEntrance.h"

#if defined(__USE_TEMPLETE)
#include "MapBase.h"
#endif


class CFileManager
{
public:
	CFileManager();
	virtual ~CFileManager();

protected:
	string m_strErrMsg;

	// 전체 지도 영역
	SBox m_rtBox;
	//unordered_map<uint32_t, FileIndex> m_mapIndex;

private:
	CFileName m_fileName;
	CFileMesh m_fileMesh;
	CFileTrekking m_fileTrekking;
	CFilePedestrian m_filePedestrian;
	CFileVehicle m_fileVehicle;
	CFileComplex m_fileComplex;
	CFileBuilding m_fileBuilding;
	CFileEntrance m_fileEntrance;


protected:
	char m_szDataPath[MAX_PATH];
	

#if defined(__USE_TEMPLETE)
	MapBase<stComplexInfo> m_MapBase;
#endif
	const char* GetErrorMsg();


public:
	bool Initialize(void);
	void Release(void);

	void SetDataMgr(CDataManager* pDataMgr);
	const SBox* GetMeshRegion(void) const;

	///////////////////////////////////////////////////////////////////////////
	bool OpenFile(IN const char* pszFilePath, IN const uint32_t nFileType);
	bool SaveData(IN const char* pszFilePath);

	bool LoadData(IN const char* pszFilePath);
	bool LoadDataByIdx(IN const uint32_t idx);

	bool GetData(IN const uint32_t idTile);
};

