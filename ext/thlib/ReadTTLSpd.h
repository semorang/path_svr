#pragma once
#include <iostream>

#include "sti_ctrl_s.h"

#include "sti_index_ctrl.h"		// get spd
#include "ttl_mapreader_r.h"	// get ttl id

class ReadTTLSpd
{
private:
	// spd
	sTrfCtrl*		m_pTrfCtrl;

	// ttl id
	std::map<unsigned long long, uint32_t,
		std::less<unsigned long long>,
		thallocator<std::pair<unsigned long long, uint32_t> > >  m_mapTTL;

	char m_szPath[FILENAME_MAX];
	size_t m_size;

private:
	// "[#TTL] load_ttl success" -> Ž�� ���� �⵿�� �Ʒ� �Լ� �����ϸ� ������ �α�
	int load_ttl();

public:
	ReadTTLSpd();
	~ReadTTLSpd();

	bool Initialize();
	void Release();

	bool LoadData(const char* szPath);

	uint8_t GetSpd(unsigned long long ttlId, time_t time, float linkLength);
};

