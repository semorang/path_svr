#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "FileName.h"

#include "../utils/UserLog.h"
#include "../route/DataManager.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFileName::CFileName()
{
	m_nFileType = TYPE_DATA_NAME;
	m_idxDic = 0;

	// 인덱스 0의 명칭 초기화
	AddData("undefined");
}

CFileName::~CFileName()
{
	if (!m_mapDic.empty()) {
		m_mapDic.clear();
		unordered_map<string, uint32_t>().swap(m_mapDic);
	}
}


bool CFileName::Initialize()
{
	return CFileBase::Initialize();
}


void CFileName::Release()
{
	CFileBase::Release();
}


uint32_t CFileName::AddData(char* pszName)
{
	int retIdx = 0;
	
	string strName = trim(pszName);
	if (strName.length() > 0)
	{
		unordered_map<string, uint32_t>::const_iterator it = m_mapDic.find(strName);
		if (it == m_mapDic.end())
		{
			// 없으면 추가
			m_mapDic.emplace(strName, m_idxDic);
			retIdx = m_idxDic++;

			// 클래스에 추가
			if (m_pDataMgr) {
				stNameInfo* pName = new stNameInfo;
				pName->name_id = retIdx;
				pName->name = strName;
				m_pDataMgr->AddNameData(pName);
			}
		}
		else
		{
			retIdx = it->second;
		}
	}

	return retIdx;
}


bool CFileName::SaveData(IN const char* szFilePath)
{
	/////////////////////////////////////////
	// name dic
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_NAME], g_szTypeExec[TYPE_DATA_NAME]);

	return CFileBase::SaveData(szFileName);
}


size_t CFileName::WriteHeader(FILE* fp)
{
	size_t offFile = 0;
	size_t offItem = 0;
	size_t retWrite = 0;
	size_t retRead = 0;
	static const size_t s_sizeNameHeader = sizeof(FileHeader);

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	m_fileHeader.cntIndex = m_mapDic.size();
	m_fileHeader.offIndex = sizeof(FileBase) + s_sizeNameHeader;
	m_fileHeader.offBody = m_fileHeader.offIndex + sizeof(NameDicIndex) * m_fileHeader.cntIndex;

	if ((retWrite = fwrite(&m_fileHeader, s_sizeNameHeader, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't write name dic header, cnt:%d", retWrite);
		return 0;
	}
	offFile += s_sizeNameHeader;

	LOG_TRACE(LOG_DEBUG, "Save name dic header data, name cnt:%lld", m_fileHeader.cntIndex);

	return offFile;
}


size_t CFileName::WriteIndex(FILE* fp)
{
	size_t offFile = 0;
	size_t offBody = 0;
	size_t retWrite = 0;
	size_t retRead = 0;
	NameDicIndex fileIndex;
	static const size_t s_sizeNameIndex = sizeof(NameDicIndex);

	for (unordered_map<string, uint32_t>::const_iterator it = m_mapDic.begin(); it != m_mapDic.end(); it++)
	{
		fileIndex.idxName = it->second;
		fileIndex.offBody = offBody;
		fileIndex.szBody = it->first.length();

		if ((retWrite = fwrite(&fileIndex, s_sizeNameIndex, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't write name dic index:%d", it->second);
			return 0;
		}
		offBody += fileIndex.szBody;
	}

	LOG_TRACE(LOG_DEBUG, "Save data, name index, cnt:%lld", m_mapDic.size());

	return s_sizeNameIndex * m_mapDic.size();
}


size_t CFileName::WriteBody(FILE* fp, IN const uint32_t fileOff)
{
	size_t offFile = fileOff;
	size_t offItem = 0;
	size_t retWrite = 0;
	size_t retRead = 0;
	const size_t sizeBody = sizeof(NameDicIndex);

	for (unordered_map<string, uint32_t>::const_iterator it = m_mapDic.begin(); it != m_mapDic.end(); it++)
	{
		if (it->first.length() <= 0) {
			LOG_TRACE(LOG_ERROR, "Failed, can't write name dic index:%d, size:%d", it->second, it->first.length());
			return 0;
		}

		if ((retWrite = fwrite(it->first.c_str(), it->first.length(), 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't write name dic index:%d, cnt:%d", it->second, retWrite);
			return 0;
		}

		offFile += it->first.length();
	}

	return offFile;
}


bool CFileName::LoadData(IN const char* szFilePath)
{
	if (!m_pDataMgr) {
		LOG_TRACE(LOG_ERROR, "Failed, container pointer null");
		return false;
	}

	// load name dic
	size_t offFile = 0;
	size_t offItem = 0;
	size_t retRead = 0;

	/////////////////////////////////////////
	// name dic
	char szFileName[MAX_PATH] = { 0, };
	sprintf(szFileName, "%s/%s.%s", szFilePath, g_szTypeTitle[TYPE_DATA_NAME], g_szTypeExec[TYPE_DATA_NAME]);

	return CFileBase::LoadData(szFileName);
}


size_t CFileName::ReadHeader(FILE* fp)
{
	size_t offFile = 0;
	size_t retRead = 0;
	const size_t sizeHeader = sizeof(FileHeader);

	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	if ((retRead = fread(&m_fileHeader, sizeHeader, 1, fp)) != 1)
	{
		LOG_TRACE(LOG_ERROR, "Failed, can't read header, cnt:%d", retRead);
		return 0;
	}

	offFile = sizeHeader;


	LOG_TRACE(LOG_DEBUG, "Read data, header, cnt index:%d", m_fileHeader.cntIndex);

	return offFile;
}


size_t CFileName::ReadIndex(FILE* fp)
{
	return 1;
}


size_t CFileName::ReadBody(FILE* fp)
{
	if (!fp) {
		LOG_TRACE(LOG_ERROR, "Failed, input param null, fp:%p", fp);
		return 0;
	}

	size_t offFile = 0;
	size_t retRead = 0;
	NameDicIndex fileIndex;
	const size_t sizeIndex = sizeof(fileIndex);
	char szNameBuff[1024] = { 0, };

	for (uint32_t idx = 0; idx < m_fileHeader.cntIndex; idx++)
	{
		// read index
		fseek(fp, m_fileHeader.offIndex + sizeIndex * idx, SEEK_SET);
		if ((retRead = fread(&fileIndex, sizeIndex, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read index, idx:%d", idx);
			return 0;
		}

		//m_vtIndex.emplace_back(fileIndex);
		//offFile += sizeIndex;

		if (fileIndex.szBody <= 0)
		{
			LOG_TRACE(LOG_ERROR, "Failed, index body info invalid, off:%d, size:%d", fileIndex.szBody);
			return 0;
		}


		// read body

		stNameInfo* pNameInfo = new stNameInfo;
		pNameInfo->name_id = fileIndex.idxName;

		fseek(fp, m_fileHeader.offBody + fileIndex.offBody, SEEK_SET);
		if ((retRead = fread(szNameBuff, fileIndex.szBody, 1, fp)) != 1)
		{
			LOG_TRACE(LOG_ERROR, "Failed, can't read body, offset:%d", fileIndex.offBody);
			return 0;
		}
		szNameBuff[fileIndex.szBody] = '\0';
		pNameInfo->name = szNameBuff;

		// add data
		m_pDataMgr->AddNameData(pNameInfo);


		offFile += fileIndex.szBody;
	}


	LOG_TRACE(LOG_DEBUG, "Read data, body, size:%d", offFile);

	return offFile;
}
