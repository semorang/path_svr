#pragma once

#include "MapBase.h"


#define SPEED_NOT_AVALABLE	0xFF // 255

#define TILEID_BYTE_COUNT	100000000

#define TRAFFIC_DATA_ALIVE	15 * 60 // 교통 정보 유지 시간, 현재 15분

enum
{
	TYPE_TRAFFIC_NONE,
	TYPE_TRAFFIC_REAL,
	TYPE_TRAFFIC_STATIC,
	TYPE_TRAFFIC_REAL_STATIC,
};

enum
{
	TYPE_SPEED_NONE,
	TYPE_SPEED_REAL_TTL,
	TYPE_SPEED_REAL_KS,
	TYPE_SPEED_STATIC_TTL,
	TYPE_SPEED_STATIC_KS,
};

struct stKSLinkLink
{
	uint32_t ksIdPositive; // 정
	uint32_t ksIdNagative; // 역

	stKSLinkLink()
	{
		ksIdPositive = 0;
		ksIdNagative = 0;
	}
};

struct stTTLinLink
{
	uint32_t ttlIdPositive; // 정
	uint32_t ttlIdNagative; // 역

	stTTLinLink() {
		ttlIdPositive = 0;
		ttlIdNagative = 0;
	}
};

struct KSKeyID
{
	uint64_t keyID;
	union
	{
		uint64_t dir : 4;
		uint64_t ks_id : 60;
	};
};

struct stTrafficMesh {
	uint32_t meshId;

	// link로 traffic 정보 찾기
	unordered_map<uint32_t, stKSLinkLink> mapLinkToKS; // link id를 통한 ks 매칭
	unordered_map<uint32_t, stTTLinLink> mapLinkToTTL; // link id를 통한 ttl 매칭

	// id로 traffic 정보 찾기, ttl 만 (ks는 메쉬 단위로 나뉘지 않음)
	unordered_map<uint32_t, stTrafficInfoTTL*> mapTrafficTTL; // ttl id를 통한 traffic info 검색
};

class MapTraffic : public MapBase
{
public:
	MapTraffic();
	~MapTraffic();

	virtual bool Initialize(void);
	virtual void Release(void);

	virtual const uint32_t GetCount(void) const;


	// Traffic
	const unordered_map<uint32_t, stTrafficMesh*>* GetTrafficMeshData(void) const;
	const uint8_t GetSpeed(IN const KeyID link_key, IN const uint8_t dir, IN OUT uint8_t& type);
	const uint64_t GetTrafficId(IN const KeyID link, IN const uint8_t dir, IN const uint8_t type);

	// KS
	bool AddKSData(IN const uint32_t ks_id, IN const uint32_t tile_nid, IN const uint32_t link_nid, IN const uint8_t dir);
	bool UpdateKSData(IN const uint32_t ksId, IN const uint8_t speed, uint32_t timestamp);
	bool CheckAliveKSData(IN const uint32_t timestamp);
	const stTrafficInfoKS* GetTrafficInfoKS(IN const uint32_t ks_id);
	const unordered_map<uint32_t, stTrafficInfoKS*>* GetTrafficKSMapData(void) const;

	// TTL
	bool AddTTLData(IN const uint32_t ttl_nid, IN const uint8_t ttl_dir, IN const uint32_t tile_nid, IN const uint32_t link_nid, IN const uint8_t link_dir);
	bool UpdateTTLData(IN const uint64_t ttlId, IN const uint8_t speed, uint32_t timestamp, OUT uint8_t& dir, OUT KeyID& link);
	bool CheckAliveTTLData(IN const uint32_t timestamp, OUT vector<stTrafficInfoTTL>& vtCheckedResetLink);
	const stTrafficInfoTTL* GetTrafficInfoTTL(IN const uint64_t ttl_id);
	const unordered_map<uint32_t, stTrafficInfoTTL*>* GetTrafficTTLMapData(IN const uint32_t meshId);
	
private:
	unordered_map<uint32_t, stTrafficMesh*> mapTrafficMeshData;
	unordered_map<uint32_t, stTrafficInfoKS*> mapTrafficInfoKS;  // id로 traffic 정보 찾기, ks는 메쉬 단위로 나뉘지 않아 따로 관리
};

