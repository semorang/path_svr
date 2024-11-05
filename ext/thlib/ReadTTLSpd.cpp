#include "ReadTTLSpd.h"

ReadTTLSpd::ReadTTLSpd()
{
	m_size = 1024 * 1024 * 10;	// int RpInitParam::_cache_trf;	/	rp_init_param._cache_trf   = (1024 * 1024 * 10);
}


ReadTTLSpd::~ReadTTLSpd()
{
}


bool ReadTTLSpd::Initialize()
{
	return true;
}


void ReadTTLSpd::Release()
{
	thlib::MemPolicyNew<sTrfCtrl>::memfree(m_pTrfCtrl);
}


bool ReadTTLSpd::LoadData(const char* szPath)
{
	bool ret = false;

	if (szPath != nullptr && strlen(szPath) > 0) {
		strcpy(m_szPath, szPath);

		m_pTrfCtrl = thlib::MemPolicyNew<sTrfCtrl>::alloc(1);
		m_pTrfCtrl->init(m_szPath, m_size, sTrfCache::CACHE_TTL, sTrfReader::STRF_TYPE_TTL, 0);	// traffic_pattern_ttl.bin

		ret = (load_ttl() == 0) ? true : false;
	}

	return ret;
}


uint8_t ReadTTLSpd::GetSpd(unsigned long long ttlId, time_t time, float linkLength)
{
	uint8_t retSpd = 255;

	if (ttlId > 0 && (m_mapTTL.find(ttlId) != m_mapTTL.end())) {
		int bidx = m_mapTTL[ttlId] / 2048;
		int iidx = m_mapTTL[ttlId] % 2048;

		if ((bidx < 2047) && (iidx < 2047)) {
			m_pTrfCtrl->init(m_szPath, m_size, sTrfCache::CACHE_TTL, sTrfReader::STRF_TYPE_TTL, time);	// traffic_pattern_ttl.bin
			m_pTrfCtrl->speed(&retSpd, bidx, iidx, time, linkLength);
		}
	}

	return retSpd;
}


int ReadTTLSpd::load_ttl()
{
	// node-gyp 빌드하고 실행하면 아래 구문때문에 바로 종료됨..왜일까
	// std::cout << "[#TTL] load_ttl start" << std::endl;

	int err = 0;
	thlib::thFile<> f;
	thlib::thString path;
	path.printf("%s/%s", m_szPath, ttlMapFileDefs::F_TR_ID_TTL2);
	err = f.open(path, "rb");
	if (err != 0) {
		return err;
	}

	uint32_t size = f.size();
	char *buf = thlib::MemPolicyNew<char>::alloc(size);
	if (buf == NULL) {
		return -1;
	}
	f.read(buf, size);
	f.close();

	unsigned long long *ttlid = NULL;
	uint32_t idx = 0;
	uint32_t off = sizeof(int);
	while (off < size) {
		ttlid = (unsigned long long*)(buf + off);
		off += sizeof(unsigned long long);
		m_mapTTL[*ttlid] = idx;
		++idx;
	}

	thlib::MemPolicyNew<char>::memfree(buf);

	// node-gyp 빌드하고 실행하면 아래 구문때문에 바로 종료됨..왜일까
	// std::cout << "[#TTL] load_ttl success" << std::endl;
	return 0;
}