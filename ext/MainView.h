#pragma once



// MainView 폼 뷰입니다.
#include "route\MapDef.h"

#if defined(USE_PEDESTRIAN_DATA)
#	include "route\FilePedestrian.h" // 보행자/자전거 데이터
#elif defined(USE_TECKING_DATA)
#	include "route\FileTrecking.h"
#else
#	include "route\FileManager.h"
#endif
#include "route\RouteManager.h"

// for drawing
#include "Drawing.h"

#define DEBUG_MALLOC(size) _malloc_dbg(size, _NORMAL_BLOCK, __FILE__ , __LINE__) 



class MainView : public CFormView
{
	DECLARE_DYNCREATE(MainView)

protected:
	MainView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~MainView();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAINVIEW };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDataParse();
	afx_msg void OnDataSave();
	afx_msg void OnDataLoad();
	afx_msg void OnDoRoute();
	afx_msg void OnBnClickedBtnZoomin();
	afx_msg void OnBnClickedBtnZoomout();

private:
	bool m_isInitialized;

	bool m_isMouseDown;
	bool m_isMouseMove;
	bool m_isRouting;
	int m_nCurrentZoom;
	int m_cntRouteLine;
	PointF* m_pRouteLineWorld;
	PointN* m_pRouteLineScreen;

	PointN m_centerCross;
	PointN m_ptLastLMouse;
	PointN m_ptLastRMouse;

#if defined(USE_PEDESTRIAN_DATA)
	CFilePedestrian m_pFileMgr;
#elif defined(USE_TECKING_DATA)
	CFileTrecking m_pFileMgr;
#else
	CFileManager m_pFileMgr;
#endif
	CRouteManager m_pRouteMgr;

	// 전체 지도 영역
	POINT m_ptParent;
	RECT m_rtViewport;
	SBox m_mapBox;

	KeyID m_keyClickedLinkId;
	vector<SPoint>m_vtClickedLink;

	unordered_map<uint64_t, bool> m_mapRouteDraw;


	CString m_strWorkResourcePath;
	CString m_strWorkResultPath;
	CString m_strDataResourcePath;

	SPoint m_ptCenterLocation;

	CString m_strStartLocation;
	CString m_strEndLocation;

	uint32_t m_cntCache;

	CDrawing m_Drawing;

public:
	void Initialize();
	void SetMove(IN const int x, IN const int y);
	void RedrawMap();

	void DrawMesh();
	void DrawNode();
	void DrawLink();
	void DrawRoute();
	void DrawPoint();
	void DrawRouteStatus(IN const unordered_map<uint64_t, CandidateLink>* pRouting);
	void DrawInfo();


	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedBtnGotoStart();
	afx_msg void OnBnClickedBtnGotoEnd();
	afx_msg void OnMapSetStart();
	afx_msg void OnMapSetWaypoint();
	afx_msg void OnMapSetEnd();
	afx_msg void OnMapSetRoute();
};


SPoint getCoordFromString(CString &strCoord);