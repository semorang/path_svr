#pragma once

#include "../include/MapDef.h"
#include "../include/types.h"
#include "DataManager.h"


typedef struct _tagRouteProbablePath {
	KeyID LinkId; // ��ũ ID
	KeyID NodeId; // ��� ID
	vector<stLinkInfo*> JctLinks; // ���� ��ũ

	_tagRouteProbablePath() {
		LinkId.llid = 0;
		NodeId.llid = 0;
	}

	~_tagRouteProbablePath() {
		if (!JctLinks.empty()) {
			for (const auto& junction : JctLinks) {
				delete ((stLinkInfo*)junction);
			}
			JctLinks.clear();
			vector<stLinkInfo*>().swap(JctLinks);
		}
	}
}RouteProbablePath;



template<typename ... Args>
std::string string_format(const std::string& format, Args ... args);

class CRoutePackage
{
public:
	CRoutePackage();
	virtual ~CRoutePackage();

	bool Initialize(void);
	void Release(void);

	void SetDataMgr(IN CDataManager* pDataMgr);

	void GetErrorResult(IN const int32_t err_code, IN const char* err_msg,  OUT string& strJson);
	void GetMultiRouteResult(IN const RouteResultInfo* pResult, IN const int target, OUT string& strJson);
	void GetMultiRouteResultForiNavi(IN const RouteResultInfo* pResult, OUT string& strResult);
	void GetClusteringResult(IN const vector<stDistrict>& vtClusters, OUT string& strJson);
	void GetBoundaryResult(IN const vector<SPoint>& vtBoundary, OUT string& strJson);
		// �б��� ����� ��μ��� ������ ������ ��ũ ��� // like a mpp
	// length = ������� �ִ� ����, 0: ��ü, ...
	// expansion = �� ��� �� Ȯ���� ����, 0: ���� ��θ�, 1: ���� ��ο��� 1�ܰ踸 ����ġ�� ...
	// branchLength = ����ġ���� ����� �ִ� ����
	const size_t GetRouteProbablePath(IN const RouteResultInfo* pResult, OUT vector<RouteProbablePath*>& vtJctInfo, IN const double length = 0, IN const int32_t expansion = 1, IN const double branchLength = 50);

private:
	CDataManager* m_pDataMgr;
};

