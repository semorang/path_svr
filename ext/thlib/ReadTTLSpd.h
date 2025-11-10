#pragma once
#include <iostream>

#include "sti_ctrl_s.h"
#include "sti_index_ctrl.h"		// get spd
#include "ttl_mapreader_r.h"	// get ttl id

#include <vector>
#include <unordered_map>

class ReadTTLSpd
{
private:
	// spd
	sTrfCtrl*		m_pTrfCtrl;

	// ttl id
	std::map<unsigned long long, uint32_t,
		std::less<unsigned long long>,
		thallocator<std::pair<unsigned long long, uint32_t> > >  m_mapTTL;

	std::vector<TTLSPD> m_vtStatic;	// testtt s get spd all ttl

	char m_szPath[FILENAME_MAX];
	size_t m_size;
	bool m_isInitialized;

private:
	// "[#TTL] load_ttl success" -> Ž�� ���� �⵿�� �Ʒ� �Լ� �����ϸ� ������ �α�
	int load_ttl();

public:
	ReadTTLSpd();
	~ReadTTLSpd();

	bool Initialize();
	void Release();

	bool LoadData(const char* szPath);

	uint8_t GetSpd(const unsigned long long ttlId, const time_t time, const float linkLength);
	int GetStaticSpeedBlock(const time_t time, std::unordered_map<uint64_t, uint8_t>& umapStatic);
};

