// MainView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Trecking.h"
#include "MainView.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// MainView
#include "utils/UserLog.h"


static int g_drawLockZoomLevel = 2;
static bool g_isOnPaint = false;

#define MAX_ZOOM 19
#define MIN_ZOOM 0

IMPLEMENT_DYNCREATE(MainView, CFormView)

//#define malloc DEBUG_MALLOC
//
//#include <crtdbg.h>


MainView::MainView()
	: CFormView(IDD_MAINVIEW)
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}

MainView::~MainView()
{
	//_CrtDumpMemoryLeaks();
}

void MainView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(MainView, CFormView)
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(ID_DATA_PARSE, &MainView::OnDataParse)
	ON_COMMAND(ID_DATA_SAVE, &MainView::OnDataSave)
	ON_COMMAND(ID_DATA_LOAD, &MainView::OnDataLoad)
	ON_COMMAND(ID_DO_ROUTE, &MainView::OnDoRoute)
	ON_BN_CLICKED(IDC_BTN_ZOOMIN, &MainView::OnBnClickedBtnZoomin)
	ON_BN_CLICKED(IDC_BTN_ZOOMOUT, &MainView::OnBnClickedBtnZoomout)
	ON_WM_ACTIVATE()
	ON_WM_ACTIVATEAPP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_BN_CLICKED(IDC_BTN_GOTO_START, &MainView::OnBnClickedBtnGotoStart)
	ON_BN_CLICKED(IDC_BTN_GOTO_END, &MainView::OnBnClickedBtnGotoEnd)
	ON_COMMAND(ID_MAP_SET_START, &MainView::OnMapSetStart)
	ON_COMMAND(ID_MAP_SET_WAYPOINT, &MainView::OnMapSetWaypoint)
	ON_COMMAND(ID_MAP_SET_END, &MainView::OnMapSetEnd)
	ON_COMMAND(ID_MAP_SET_ROUTE, &MainView::OnMapSetRoute)
END_MESSAGE_MAP()


// MainView 진단입니다.

#ifdef _DEBUG
void MainView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void MainView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// MainView 메시지 처리기입니다.


int MainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_isMouseDown = false;
	m_isMouseMove = false;
	m_nCurrentZoom = 5;
	m_cntRouteLine = 0;
	m_pRouteLineWorld = nullptr;
	m_pRouteLineScreen = nullptr;
	m_cntCache = 100;

	return 0;
}

static int g_cntMaxMesh = 0;
static int g_cntPitInMesh = 0;
static stMeshInfo** g_pPitInMesh = nullptr;

void MainView::OnDestroy()
{
	CFormView::OnDestroy();

	CGnOSMCoord::Free();

	if (g_pPitInMesh)
	{
		free(g_pPitInMesh);
	}

	if (m_pRouteLineWorld)
	{
		free(m_pRouteLineWorld);
		m_pRouteLineWorld = nullptr;
	}
	if (m_pRouteLineScreen)
	{
		free(m_pRouteLineScreen);
		m_pRouteLineScreen = nullptr;
	}
}


void MainView::OnPaint()
{
	g_isOnPaint = true;

	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CFormView::OnPaint()을(를) 호출하지 마십시오.

	CRect rct;
	GetClientRect(&rct);
	static BITMAPINFO m_Info; { m_Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); m_Info.bmiHeader.biWidth = rct.Width(); m_Info.bmiHeader.biHeight = -rct.Height(); m_Info.bmiHeader.biPlanes = 1; m_Info.bmiHeader.biBitCount = 32; m_Info.bmiHeader.biCompression = BI_RGB; m_Info.bmiHeader.biSizeImage = m_Info.bmiHeader.biWidth * (-m_Info.bmiHeader.biHeight) * 4; }
	//CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트
	SetDIBitsToDevice(dc.GetSafeHdc(), 0, 0, rct.Width(), rct.Height(), 0, 0, 0, rct.Height(), g_UniDib.m_pBits, &m_Info, DIB_RGB_COLORS);

	g_isOnPaint = false;
}


void MainView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	if (cx > 0 && cy > 0)
	{
		g_UniDib.Create(cx, cy);
		CGnOSMCoord::Instance()->InitScreen(0, 0, cx, cy);

		m_centerCross.x = cx >> 1;
		m_centerCross.y = cy >> 1;

		m_rtViewport.right = m_rtViewport.left + cx;
		m_rtViewport.bottom = m_rtViewport.top + cy;
	}

	if (m_isInitialized) {
		RedrawMap();
	}
}


void MainView::OnMove(int x, int y)
{
	CFormView::OnMove(x, y);

	m_rtViewport.left = x;
	m_rtViewport.top = y;
}


void MainView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_isMouseDown = true;
	m_isMouseMove = false;

	CFormView::OnLButtonDown(nFlags, point);
}


void MainView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!m_isMouseDown) {
		return;
	}

	m_isMouseDown = false;

	if (!m_isMouseMove)
	{
		m_isMouseMove = false;
		PointF ptWorld;

		CGnOSMCoord::Instance()->ScreenToWorld(point.x, point.y, ptWorld.x, ptWorld.y);


		if (m_nCurrentZoom < 5) {
			// link click
			stLinkInfo * pLink = nullptr;
			int nDist = m_nCurrentZoom * 0.2 + 5;
			if (pLink = m_pFileMgr.GetLinkDataByPointAround(ptWorld.x, ptWorld.y, nDist))
			{
				m_keyClickedLinkId = pLink->link_id;
				m_vtClickedLink.clear();
				//m_vtClickedLink.assign(pLink->vtPts.begin(), pLink->vtPts.end());
				//copy(pLink->vtPts.begin(), pLink->vtPts.end(), m_vtClickedLink.begin());
				for (int ii = 0; ii < pLink->vtPts.size(); ii++)
				{
					m_vtClickedLink.push_back(pLink->vtPts[ii]);
				}
			}
			else
			{
				CGnOSMCoord::Instance()->CenterFocus(ptWorld.x, ptWorld.y);
			}
		}
		else
		{
			CGnOSMCoord::Instance()->CenterFocus(ptWorld.x, ptWorld.y);
		}

		RedrawMap();
	}

	CFormView::OnLButtonUp(nFlags, point);
}


void MainView::OnMouseMove(UINT nFlags, CPoint point)
{
	int ptMouseGapX = point.x - m_ptLastLMouse.x;
	int ptMouseGapY = point.y - m_ptLastLMouse.y;

	m_ptLastLMouse.x = point.x;
	m_ptLastLMouse.y = point.y;

	if (m_isMouseDown && (ptMouseGapX != 0 || ptMouseGapY != 0))
	{
		m_isMouseMove = true;

#if 1
		PointN ptCoordN = { m_centerCross.x - ptMouseGapX, m_centerCross.y - ptMouseGapY };
		PointF ptCoordF = { 0, };
		CGnOSMCoord::Instance()->ScreenToWorld(ptCoordN, ptCoordF);
		CGnOSMCoord::Instance()->CenterFocus(ptCoordF.x, ptCoordF.y);
			
		RedrawMap();
#else
		CGnOSMCoord::Instance()->PanMap(m_centerCross.x, m_centerCross.y, m_centerCross.x + ptMouseGapX, m_centerCross.y + ptMouseGapY);
		Invalidate();
#endif
	}
	else
	{
		// darw center coordnate
		CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();

		if (pFrm->GetStatusBar()) {
			PointF ptCursorF = { 0, };
			PointN ptCursorN = { point.x, point.y };
			if (CGnOSMCoord::Instance()->ScreenToWorld(ptCursorN, ptCursorF))
			{
				char szInfo[64] = { 0, };
				CString strCursor;
				strCursor.Format(_T("Cursor: %.6f, %.6f"), ptCursorF.x, ptCursorF.y);
				pFrm->GetStatusBar()->SetWindowTextW(strCursor);
			}
		}
	}

	CFormView::OnMouseMove(nFlags, point);
}


BOOL MainView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (PtInRect(&m_rtViewport, pt))
	{
		CPoint ptView = { pt.x - (m_ptParent.x + m_rtViewport.left), pt.y - (m_ptParent.y + m_rtViewport.top) };
		PointN ptPrevScreen = { m_centerCross.x - (ptView.x - m_centerCross.x), m_centerCross.y - (ptView.y - m_centerCross.y) };
		PointF ptPrevWorld;
		PointF ptNextWorld;

		CGnOSMCoord::Instance()->ScreenToWorld(m_centerCross.x, m_centerCross.y, ptPrevWorld.x, ptPrevWorld.y);
		CGnOSMCoord::Instance()->ScreenToWorld(ptView.x, ptView.y, ptNextWorld.x, ptNextWorld.y);
		CGnOSMCoord::Instance()->CenterFocus(ptNextWorld.x, ptNextWorld.y);

		if (zDelta > 0)
		{
			// Zoom In
			if (MIN_ZOOM < m_nCurrentZoom)
				m_nCurrentZoom--;
		}
		else if (zDelta < 0)
		{
			// Zoom Out
			if (m_nCurrentZoom < MAX_ZOOM)
				m_nCurrentZoom++;
		}
		CGnOSMCoord::Instance()->SetZoomLevel(m_nCurrentZoom);

		CGnOSMCoord::Instance()->ScreenToWorld(ptPrevScreen, ptPrevWorld);
		CGnOSMCoord::Instance()->CenterFocus(ptPrevWorld.x, ptPrevWorld.y);

		RedrawMap();
	}


	return CFormView::OnMouseWheel(nFlags, zDelta, pt);
}


void MainView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CFormView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void MainView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CFormView::OnKeyUp(nChar, nRepCnt, nFlags);
}



void MainView::OnBnClickedBtnZoomin()
{
	if (MIN_ZOOM < m_nCurrentZoom)
		m_nCurrentZoom--;

	CGnOSMCoord::Instance()->SetZoomLevel(m_nCurrentZoom);

	RedrawMap();
}


void MainView::OnBnClickedBtnZoomout()
{
	if (m_nCurrentZoom < MAX_ZOOM)
		m_nCurrentZoom++;

	CGnOSMCoord::Instance()->SetZoomLevel(m_nCurrentZoom);

	RedrawMap();
}



void MainView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_ptLastRMouse.x = point.x;
	m_ptLastRMouse.y = point.y;

	CFormView::OnRButtonDown(nFlags, point);
}


void MainView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_ptLastRMouse.x == point.x && m_ptLastRMouse.y == point.y)
	{
		CMenu menuMap, *pContextMenu;
		menuMap.LoadMenu(IDR_POPUP_MAP);

		// waypoint
		menuMap.EnableMenuItem(ID_MAP_SET_WAYPOINT, MF_DISABLED/*MF_GRAYED*/);

		// do route
		if (m_pFileMgr.GetMeshCount() <= 0) {
			menuMap.EnableMenuItem(ID_MAP_SET_ROUTE, MF_DISABLED/*MF_GRAYED*/);
		}


		pContextMenu = menuMap.GetSubMenu(0);
		pContextMenu->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, point.x, point.y, this);


	}

	CFormView::OnRButtonUp(nFlags, point);
}


void MainView::OnBnClickedBtnGotoStart()
{
	if (m_pFileMgr.GetMeshCount() <= 0) {
		CString strMsg;
		strMsg.Format(_T("Please Load Shp Data!!!!!!!!!!!!!"));
		AfxMessageBox(strMsg);
		return;
	}

	CString strCoord;
	GetDlgItem(IDC_EDT_START_COORD)->GetWindowTextW(strCoord);
	SPoint ptCoord = getCoordFromString(strCoord);

	CGnOSMCoord::Instance()->CenterFocus(ptCoord.x, ptCoord.y);

	m_pRouteMgr.SetDeparture(ptCoord.x, ptCoord.y);

	RedrawMap();
}


void MainView::OnBnClickedBtnGotoEnd()
{
	if (m_pFileMgr.GetMeshCount() <= 0) {
		CString strMsg;
		strMsg.Format(_T("Please Load Shp Data!!!!!!!!!!!!!"));
		AfxMessageBox(strMsg);
		return;
	}

	CString strCoord;
	GetDlgItem(IDC_EDT_END_COORD)->GetWindowTextW(strCoord);
	SPoint ptCoord = getCoordFromString(strCoord);

	CGnOSMCoord::Instance()->CenterFocus(ptCoord.x, ptCoord.y);

	m_pRouteMgr.SetDestination(ptCoord.x, ptCoord.y);

	RedrawMap();
}


SPoint getCoordFromString(CString &strCoord) {
	CString str_sub;
	SPoint ptLocation;
	int column = 0;
	int pos = 0;

	while (AfxExtractSubString(str_sub, strCoord, pos++, ',') != false) {
		switch (column) {
		case 0:
			ptLocation.x = _wtof(str_sub);
			column++;
			break;
		case 1:
			ptLocation.y = _wtof(str_sub);
			column++;
			break;
		default:
			break;
		}
	}

	return ptLocation;
}


void MainView::Initialize()
{
	m_isInitialized = true;

	
	// 데이터 경로 설정
	bool isOk = false;

	CString strExecutePath;
	CString strCurDirectory;
	if (GetModuleFileName(NULL, strExecutePath.GetBuffer(MAX_PATH + 1), MAX_PATH)) {
		strExecutePath.ReleaseBuffer();
#if 1//defined(_DEBUG)
		strCurDirectory = strExecutePath.Left(strExecutePath.ReverseFind('\\'));
		strCurDirectory = strCurDirectory.Left(strCurDirectory.ReverseFind('\\'));
		strCurDirectory = strCurDirectory.Left(strCurDirectory.ReverseFind('\\') + 1);
#else
		strCurDirectory = strExecutePath.Left(strExecutePath.ReverseFind('\\') + 1);
#endif

#if defined(_DEBUG)
		strCurDirectory.Append(_T("config.ini"));
#elif defined(USE_TECKING_DATA)
		strCurDirectory.Append(_T("config_tre.ini"));
#else // USE_PEDESTRIAN_DATA
		strCurDirectory.Append(_T("config_ped.ini"));
#endif
		CFileFind finder;

		if (finder.FindFile(strCurDirectory)) {
			FILE* fp = _tfopen(strCurDirectory.GetString(), _T("rt"));
			if (fp) {
				char szBuff[MAX_PATH + 1] = { 0, };
				TCHAR wszBuff[MAX_PATH + 1] = { 0, };
				TCHAR * pBuff;
				for (; fgets(szBuff, MAX_PATH, fp);)
				{
					memset(wszBuff, 0x00, sizeof(wszBuff));
					if (szBuff[strlen(szBuff) - 1] == '\n') {
						szBuff[strlen(szBuff) - 1] = '\0';
					}
#ifdef _WINDOWS
					int nLen = MultiByteToWideChar(CP_ACP, 0, szBuff, strlen(szBuff), NULL, NULL);
					MultiByteToWideChar(CP_ACP, 0, szBuff, strlen(szBuff), wszBuff, nLen);
#else
					mbstowcs(strUnicode, strMultibyte, strlen(strMultibyte));
#endif        

					// comment
					if (wszBuff[0] == '#' || wszBuff[0] == '/' || wszBuff[0] == '!') {
						continue;
					}

					// 원도 데이터 경로
					pBuff = _tcsstr(wszBuff, _T("WORK_RESOURCE"));
					if (pBuff) {
						pBuff = _tcsstr(pBuff, _T("="));
						if (pBuff) {
							m_strWorkResourcePath.Append(pBuff + 1);
							continue;
						}
					}

					// 컴파일 이후 데이터 저장 경로
					pBuff = _tcsstr(wszBuff, _T("WORK_RESULT"));
					if (pBuff) {
						pBuff = _tcsstr(pBuff, _T("="));
						if (pBuff) {
							m_strWorkResultPath.Append(pBuff + 1);
							continue;
						}
					}

					// 엔진 사용 데이터 경로
					pBuff = _tcsstr(wszBuff, _T("DATA_RESOURCE"));
					if (pBuff) {
						pBuff = _tcsstr(pBuff, _T("="));
						if (pBuff) {
							m_strDataResourcePath.Append(pBuff + 1);
							continue;
						}
					}

					// 줌레벨					
					pBuff = _tcsstr(wszBuff, _T("ZOOM_LEVEL"));
					if (pBuff) {
						pBuff = _tcsstr(pBuff, _T("="));
						if (pBuff) {
							CString strCoord;
							strCoord.Append(pBuff + 1);
							m_nCurrentZoom = _ttoi(strCoord);
							continue;
						}
					}
					// 중심점
					pBuff = _tcsstr(wszBuff, _T("CENTER_LOCATION"));
					if (pBuff) {
						pBuff = _tcsstr(pBuff, _T("="));
						if (pBuff) {
							CString strCoord; 
							strCoord.Append(pBuff + 1);
							m_ptCenterLocation = getCoordFromString(strCoord);
							continue;
						}
					}
					

					// 시작점
					pBuff = _tcsstr(wszBuff, _T("START_LOCATION"));
					if (pBuff) {
						pBuff = _tcsstr(pBuff, _T("="));
						if (pBuff) {
							m_strStartLocation.Append(pBuff + 1);
							continue;
						}
					}

					// 종료점
					pBuff = _tcsstr(wszBuff, _T("END_LOCATION"));
					if (pBuff) {
						pBuff = _tcsstr(pBuff, _T("="));
						if (pBuff) {
							m_strEndLocation.Append(pBuff + 1);
							continue;
						}
					}

					// 캐쉬 카운트
					pBuff = _tcsstr(wszBuff, _T("CACHE_COUNT"));
					if (pBuff) {
						pBuff = _tcsstr(pBuff, _T("="));
						if (pBuff) {
							m_cntCache = _ttoi(pBuff + 1);
							continue;
						}
					}
				}

				fclose(fp);
			}
		}
	}
	else
	{
		m_strWorkResourcePath = _T("C:/__data/보행자네트워크 WGS84LONLAT_제주도/");
		m_strWorkResultPath = _T("C:/__data/Trecking/");
		m_strDataResourcePath = _T("C:/__data/Trecking/");
	}


	
	if (m_ptCenterLocation.x <= 0 || m_ptCenterLocation.y <= 0) {
		m_ptCenterLocation.x = 126.92644;
		m_ptCenterLocation.y = 37.5799;
	}

	CGnOSMCoord::Instance()->CenterFocus(m_ptCenterLocation.x, m_ptCenterLocation.y);
	CGnOSMCoord::Instance()->SetZoomLevel(m_nCurrentZoom);


	CString strCoordStart;
	CString strCoordEnd;



	//strCoordStart.Format(_T("126.49301,33.39342")); // 한라산 

	// 제주 서남부 - 남부
	//strCoordStart.Format(_T("126.43086, 33.23661"));
	//strCoordEnd.Format(_T("126.24542,33.27456"));


	// 짧은 숲길
	//strCoordStart.Format(_T("126.638948, 33.367398"));	//제주 시작점
	//strCoordEnd.Format(_T("126.66331, 33.32565"));	//제주 종료점

	// 짧은 도보 길
	//strCoordStart.Format(_T("126.57466, 33.25805"));
	//strCoordEnd.Format(_T("126.55789,33.24914"));


	// 서울 자전거길
	if (m_strStartLocation.IsEmpty()) {
		m_strStartLocation.Format(_T("126.92644, 37.57990"));
	}
	if (m_strEndLocation.IsEmpty()) {
		m_strEndLocation.Format(_T("126.94755, 37.51116"));
	}


	m_pFileMgr.SetCacheCount(m_cntCache);


	SetDlgItemText(IDC_EDT_START_COORD, m_strStartLocation);
	SetDlgItemText(IDC_EDT_END_COORD, m_strEndLocation);



	RedrawMap();
}


void MainView::SetMove(IN const int x, IN const int y)
{
	m_ptParent.x = x;
	m_ptParent.y = y;

	
}

void MainView::OnDataParse()
{
	bool isOk = false;

	if (m_strWorkResourcePath.GetLength() <= 0) {
		CFileDialog dlg(OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | BIF_BROWSEINCLUDEFILES, NULL);

		if (dlg.DoModal() == IDOK)
		{
			CFileFind finder;
			////CString CurDirectory = L"C:\\__Data\\등산길데이터샘플\\숲길이동지도_샘플_220316\\데이터\\LINK_JEJU.shp"; // dlg.GetPathName();
			CString CurDirectory = dlg.GetFolderPath() + _T("\\"); //dlg.GetPathName();
			//CString	FileName = L"WLINK.shp";// dlg.GetFileName();
			////CString	FileName = L"LINK_JEJU.shp";// dlg.GetFileName();
		}
	}


	CFileFind finder;
	////CString CurDirectory = L"C:\\__Data\\등산길데이터샘플\\숲길이동지도_샘플_220316\\데이터\\LINK_JEJU.shp"; // dlg.GetPathName();
	//CString CurDirectory = L"C:\\__Data\\보행자네트워크 WGS84LONLAT_제주도\\02. 데이터\\WLINK.shp"; // dlg.GetPathName();
	//CString	FileName = L"WLINK.shp";// dlg.GetFileName();
	////CString	FileName = L"LINK_JEJU.shp";// dlg.GetFileName();
	vector<string> vtFilePath;
	//CurDirectory.TrimRight(FileName);

	CString str = _T("*.shp");

	CString str1 = m_strWorkResourcePath + str;

	char szFiles[10][MAX_PATH];
	int cntFile = 0;
	memset(szFiles, 0x00, MAX_PATH * 10);

	bool isFileExist = finder.FindFile(str1);

	while (isFileExist) {

		isFileExist = finder.FindNextFileW();

		CString FileName = m_strWorkResourcePath + finder.GetFileName();

		string strFile = _strdup(CT2A(FileName));
		vtFilePath.push_back(strFile);
	}

	if (!vtFilePath.empty()) {
		isOk = m_pFileMgr.OpenFile(&vtFilePath);
	}



	if (isOk)
	{
		CGnOSMCoord::Instance()->CenterFocus((m_pFileMgr.GetMeshRegion()->Xmax + m_pFileMgr.GetMeshRegion()->Xmin) / 2,
			(m_pFileMgr.GetMeshRegion()->Ymax + m_pFileMgr.GetMeshRegion()->Ymin) / 2);
		CGnOSMCoord::Instance()->SetZoomLevel(m_nCurrentZoom);

		RedrawMap();
	}


}


void MainView::OnDataSave()
{
	char szFile[MAX_PATH] = { 0, };

	CString strFile = m_strWorkResultPath;
#if defined(USE_PEDESTRIAN_DATA)
	strFile += _T("pedestrian.dat");
#elif defined(USE_TECKING_DATA)
	strFile +=+ _T("trecking.dat");
#endif

	if (m_pFileMgr.GetMeshCount() <= 0) {
		CString strMsg;
		strMsg.Format(_T("Please Load Shp Data!!!!!!!!!!!!!"));
		AfxMessageBox(strMsg);
		return;
	}

	int len = WideCharToMultiByte(CP_ACP, 0, strFile, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strFile, -1, szFile, len, NULL, NULL);

	if (!m_pFileMgr.SaveData(szFile))
	{
		CString strMsg;
		strMsg.Format(_T("Can't save network file: %s "), strFile);
		AfxMessageBox(strMsg);
		return;
	}

	m_keyClickedLinkId.llid = 0;
	m_vtClickedLink.clear();

	RedrawMap();
}


void MainView::OnDataLoad()
{
	char szFile[MAX_PATH] = { 0, };
	CString strFile = m_strDataResourcePath;

#if defined(USE_PEDESTRIAN_DATA)
	strFile += _T("pedestrian.dat");
#else
	strFile += +_T("trecking.dat");
#endif

	int len = WideCharToMultiByte(CP_ACP, 0, strFile, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strFile, -1, szFile, len, NULL, NULL);

	if (!m_pFileMgr.LoadData(szFile))
	{
		CString strMsg;
		strMsg.Format(_T("Can't fine network file: %s "), strFile);
		AfxMessageBox(strMsg);
		return;
	}

	m_pRouteMgr.SetFileMgr(&m_pFileMgr);

	CGnOSMCoord::Instance()->CenterFocus((m_pFileMgr.GetMeshRegion()->Xmax + m_pFileMgr.GetMeshRegion()->Xmin) / 2,
		(m_pFileMgr.GetMeshRegion()->Ymax + m_pFileMgr.GetMeshRegion()->Ymin) / 2);
	CGnOSMCoord::Instance()->SetZoomLevel(m_nCurrentZoom);

	RedrawMap();
}

void g_FpDrawRouteStatus(const void* pHost, const unordered_map<uint64_t, CandidateLink>* pRouting)
{
	if (pHost != nullptr && pRouting != nullptr) {
		((MainView*)pHost)->DrawRouteStatus(pRouting);
	}

}

void MainView::OnDoRoute()
{
	int ret = -1;

	if (m_pFileMgr.GetMeshCount() <= 0) {
		CString strMsg;
		strMsg.Format(_T("Please Load Shp Data!!!!!!!!!!!!!"));
		AfxMessageBox(strMsg);
		return;
	}


	CString strcord;
	GetDlgItem(IDC_EDT_START_COORD)->GetWindowTextW(strcord);
	SPoint ptStartCoord = getCoordFromString(strcord);
	GetDlgItem(IDC_EDT_END_COORD)->GetWindowTextW(strcord);
	SPoint ptEndCoord = getCoordFromString(strcord);


	stLinkIDX getSLink;
	stLinkIDX getELink;

	KeyID sID = m_pRouteMgr.SetDeparture(ptStartCoord.x, ptStartCoord.y);
	KeyID eID = m_pRouteMgr.SetDestination(ptEndCoord.x, ptEndCoord.y);

	//bool isStartPoint = Projection(wgs_Lon_s, wgs_Lat_s, &getSLink);	
	//bool isEndPoint = Projection(wgs_Lon_e, wgs_Lat_e, &getELink);


	m_cntRouteLine = 0;
	if (m_pRouteLineWorld)
	{
		free(m_pRouteLineWorld);
		m_pRouteLineWorld = nullptr;
	}
	if (m_pRouteLineScreen)
	{
		free(m_pRouteLineScreen);
		m_pRouteLineScreen = nullptr;
	}

	if (sID.llid && eID.llid) {		//출발지/도착지 프로젝션 된 경우

		// Clear route info
		m_mapRouteDraw.clear();
		
		
		// 화면 한번 정리
		RedrawMap();


		//std::vector<stLinkIDX> vector;
		//GetConnectedLinkByLink(getSLink, vector);
		//PT sFlag(wgs_Lon_s, wgs_Lat_s);
		//PT eflag(wgs_Lon_e, wgs_Lat_e);

		//double StartEndDistance = sFlag.dist(eflag);

		//if (StartEndDistance <= ROUTE_ONE_BI_DISTANCE) {		//출발지/도착지 직선거리 15Km 이하는 단방향 탐색
		//	m_isRouting = OneDirectionRoute(&getSLink, &getELink);

		//}
		//else {																						//출발지/도착지 직선거리 15Km 초과는 단방향 탐색
		//	m_isRouting = OneDirectionRoute(&getSLink, &getELink);

		//}

		// 경로 정보 상태 콜백함수 설정
#if defined(USE_SHOW_ROUTE_SATATUS)
		m_pRouteMgr.SetRouteStatusFunc(this, g_FpDrawRouteStatus);
#endif

		DrawRoute();

#if defined(USE_TECKING_DATA)
		//m_pRouteMgr.SetRouteOption(0, ROUTE_AVOID_BRIDGE);
		m_pRouteMgr.SetRouteOption(ROUTE_OPT_COMFORTABLE, ROUTE_AVOID_BRIDGE/*ROUTE_AVOID_BRIDGE*/);
#elif defined(USE_PEDESTRIAN_DATA)
		//m_pRouteMgr.SetRouteOption(ROUTE_OPT_SHORTEST, 0);
		m_pRouteMgr.SetRouteOption(ROUTE_OPT_COMFORTABLE, 0); // 편한길
		//m_pRouteMgr.SetRouteOption(ROUTE_OPT_MAINROAD, 0); // 대로
		//m_pRouteMgr.SetRouteOption(ROUTE_OPT_BIKE, 0);
#else
		//m_pRouteMgr.SetRouteOption(0, ROUTE_AVOID_BRIDGE);
		m_pRouteMgr.SetRouteOption(ROUTE_OPT_COMFORTABLE, ROUTE_AVOID_BRIDGE/*ROUTE_AVOID_BRIDGE*/);
#endif


		if ((ret = m_pRouteMgr.Route()) == 0)
		{
			m_isRouting = true;

			const RouteResultInfo* pResult = m_pRouteMgr.GetRouteResult();
			if (pResult == nullptr) {
				AfxMessageBox(_T("Failed get route result"));
				return;
			}
			m_cntRouteLine = pResult->LinkVertex.size();
			m_pRouteLineWorld = (PointF*)malloc(m_cntRouteLine * sizeof(PointF));
			m_pRouteLineScreen = (PointN*)malloc(m_cntRouteLine * sizeof(PointN));

			memcpy(m_pRouteLineWorld, &pResult->LinkVertex.front(), m_cntRouteLine * sizeof(PointF));

			SBox rtRouteBox;
			memcpy(&rtRouteBox, &pResult->RouteBox, sizeof(rtRouteBox));

			//move to point
			CGnOSMCoord::Instance()->SetZoomBounds({ rtRouteBox.Xmin, rtRouteBox.Ymin }, { rtRouteBox.Xmax, rtRouteBox.Ymax });
			m_nCurrentZoom = CGnOSMCoord::Instance()->GetZoomLevel();

			//AfxMessageBox(_T("Routing Success"));
			Invalidate(FALSE);
			OnPaint();
		}
		else
		{
			AfxMessageBox(_T("Failed routing"));
		}

		RedrawMap();
	}
	else if (sID.llid) {		//출발지만 프로젝션 된 경우

	}
	else if (eID.llid) {			//도착지만 프로젝션 된 경우

	}
}







void MainView::RedrawMap() {

	if (g_isOnPaint)
		return;
	g_isOnPaint = true;

	static COLORREF col = RGBA(30, 30, 30, 255);

	int szScreen = (CGnOSMCoord::Instance()->m_ScreenRect.right - CGnOSMCoord::Instance()->m_ScreenRect.left) * (CGnOSMCoord::Instance()->m_ScreenRect.bottom - CGnOSMCoord::Instance()->m_ScreenRect.top);

	// set bg color
	for (int ii = szScreen; ii--; g_UniDib.m_pBits[ii] = col);

	RectF rtCoordF = { 0, };
	if (!CGnOSMCoord::Instance()->ScreenToWorld(CGnOSMCoord::Instance()->m_ScreenRect.left, CGnOSMCoord::Instance()->m_ScreenRect.top, rtCoordF.left, rtCoordF.bottom)) {
		return;
	}
	if (!CGnOSMCoord::Instance()->ScreenToWorld(CGnOSMCoord::Instance()->m_ScreenRect.right, CGnOSMCoord::Instance()->m_ScreenRect.bottom, rtCoordF.right, rtCoordF.top)) {
		return;
	}
	
	memcpy(&m_mapBox, &rtCoordF, sizeof(m_mapBox));

	m_pFileMgr.GetData(m_mapBox);

	//uint64_t tickMesh, tickLine, tickNode, tickInfo;
	//size_t tickPrev = GetTickCount64();
	DrawMesh();
	//tickMesh = GetTickCount64() - tickPrev;

	//tickPrev = GetTickCount64();
	DrawLink();
	//tickLine = GetTickCount64() - tickPrev;

	//tickPrev = GetTickCount64();
	DrawNode();
	//tickNode = GetTickCount64() - tickPrev;

	//tickPrev = GetTickCount64();
	DrawRoute();
	//tickNode = GetTickCount64() - tickPrev;
	
	DrawPoint();

	//tickPrev = GetTickCount64();
	DrawInfo();
	//tickInfo = GetTickCount64() - tickPrev;

	//LOG_TRACE(LOG_DEBUG, "mesh:%d, line:%d, node:%d, info:%d", tickMesh, tickLine, tickNode, tickInfo);

	Invalidate(FALSE);
}




void MainView::DrawMesh()
{
	if (g_cntMaxMesh != m_pFileMgr.GetMeshCount())
	{
		g_cntMaxMesh = m_pFileMgr.GetMeshCount();
		if (g_pPitInMesh) {
			free(g_pPitInMesh);
			g_pPitInMesh = nullptr;
			g_cntPitInMesh = 0;
		}

		if (g_cntMaxMesh != 0) {
			g_pPitInMesh = (stMeshInfo**)malloc(sizeof(stMeshInfo*) * g_cntMaxMesh);
		}
	}

	if (g_cntMaxMesh <= 0 || g_pPitInMesh == nullptr) {
		return;
	}

	g_cntPitInMesh = m_pFileMgr.GetMeshDataByRegion((SBox&)m_mapBox, g_cntMaxMesh, g_pPitInMesh);

	PointN ptScreenLT = { 0, };
	PointN ptScreenRT = { 0, };
	PointN ptScreenLB = { 0, };
	PointN ptScreenRB = { 0, };
	PointF ptWorld = { 0, };
	for (int ii = 0; ii < g_cntPitInMesh; ii++)
	{
		stMeshInfo* pMesh = g_pPitInMesh[ii];
		if (pMesh)
		{
			//LT
			ptWorld.x = pMesh->mesh_box.Xmin;
			ptWorld.y = pMesh->mesh_box.Ymin;			
			CGnOSMCoord::Instance()->WorldToScreen(ptWorld, ptScreenLT);

			// RT
			ptWorld.x = pMesh->mesh_box.Xmax;
			ptWorld.y = pMesh->mesh_box.Ymin;
			CGnOSMCoord::Instance()->WorldToScreen(ptWorld, ptScreenRT);

			// LB
			ptWorld.x = pMesh->mesh_box.Xmin;
			ptWorld.y = pMesh->mesh_box.Ymax;
			CGnOSMCoord::Instance()->WorldToScreen(ptWorld, ptScreenLB);

			// RB
			ptWorld.x = pMesh->mesh_box.Xmax;
			ptWorld.y = pMesh->mesh_box.Ymax;
			CGnOSMCoord::Instance()->WorldToScreen(ptWorld, ptScreenRB);

			if (ptScreenRB.x - ptScreenLT.x > 0 && ptScreenLT.y - ptScreenRB.y> 0)
			//if (POINTINRECT(ptScreen.x, ptScreen.y, CGnOSMCoord::Instance()->m_ScreenRect.left, CGnOSMCoord::Instance()->m_ScreenRect.top, CGnOSMCoord::Instance()->m_ScreenRect.right, CGnOSMCoord::Instance()->m_ScreenRect.bottom))
			{
				// top
				g_UniDib.ThickLine(ptScreenLT.x, ptScreenLT.y, ptScreenRT.x, ptScreenRT.y, 1, 50, 50, 50, 255);

				// bottom 
				g_UniDib.ThickLine(ptScreenLB.x, ptScreenLB.y, ptScreenRB.x, ptScreenRB.y, 1, 50, 50, 50, 255);
				//g_UniDib.FillRect(ptScreenLT.x, ptScreenLT.y, ptScreenRB.x - ptScreenLT.x, ptScreenLT.y - ptScreenRB.y, 50, 50, 50, 255);

				// left
				g_UniDib.ThickLine(ptScreenLT.x, ptScreenLT.y, ptScreenLB.x, ptScreenLB.y, 1, 50, 50, 50, 255);

				// right
				g_UniDib.ThickLine(ptScreenRT.x, ptScreenRT.y, ptScreenRB.x, ptScreenRB.y, 1, 50, 50, 50, 255);
			}
		}
	}
}

void MainView::DrawNode()
{
	for (int ii = 0; ii < g_cntPitInMesh; ii++)
	{
		stMeshInfo* pMesh = g_pPitInMesh[ii];
		if (pMesh)
		{
			int cntNode = pMesh->nodes.size();

			for (int ii = 0; ii < cntNode; ii++)
			{
				stNodeInfo* pNode = m_pFileMgr.GetNodeDataById(pMesh->nodes[ii]);
				if (pNode && pNode->sub_info != NOT_USE)
				{
					BYTE r, g, b, a = 255;
					if (pNode->node_type == TYPE_NODE_SUBWAY) { // 지하철 진.출입
						r = 230;
						g = 50;
						b = 30;
					}
					else if (pNode->node_type == TYPE_NODE_UNDERPASS) // 지하도 진.출입
					{
						r = 50;
						g = 130;
						b = 240;
					}
					else if (pNode->node_type == TYPE_NODE_UNDERGROUND_MALL) // 지하상가 진.출입
					{
						r = 230;
						g = 240;
						b = 150;
					}
					else if (pNode->node_type == TYPE_NODE_BUILDING) // 빌딩 진.출입
					{
						r = 120;
						g = 250;
						b = 70;
					}
					else
					{
						continue;
					}

					PointF ptWorld = { pNode->coord.x, pNode->coord.y };
					PointN ptScreen = { 0, };
					CGnOSMCoord::Instance()->WorldToScreen(ptWorld, ptScreen);

					if (POINTINRECT(ptScreen.x, ptScreen.y, CGnOSMCoord::Instance()->m_ScreenRect.left, CGnOSMCoord::Instance()->m_ScreenRect.top, CGnOSMCoord::Instance()->m_ScreenRect.right, CGnOSMCoord::Instance()->m_ScreenRect.bottom))
					{
						g_UniDib.FillEllipse(ptScreen.x, ptScreen.y, 3, r, g, b, a);
					}


					//if (ii == 0)
					//{
					//	CGnOSMCoord::Instance()->CenterFocus(ptWorld.x, ptWorld.y);
					//	CGnOSMCoord::Instance()->SetZoomLevel(m_nCurrentZoom);
					//}
				}
			}
		}
	}
}


void MainView::DrawLink()
{
	//char * pImgMesh = nullptr;
	color32_t col;

	for (int ii = 0; ii < g_cntPitInMesh; ii++)
	{
		stMeshInfo* pMesh = g_pPitInMesh[ii];
		if (pMesh)
		{
			//if (!pImgMesh) {
			//	PointN ptScreenLT;
			//	PointN ptScreenRB;
			//	bool WorldToScreen(const PointF& _WorldPoint, PointN& _ScrPoint, int nZoomLevel = -1); // level 5
			//	CGnOSMCoord::Instance()->WorldToScreen(pMesh->data_box.Xmin, pMesh->data_box.Ymin, ptScreenLT.x, ptScreenLT.y);
			//	CGnOSMCoord::Instance()->WorldToScreen(pMesh->data_box.Xmax, pMesh->data_box.Ymax, ptScreenRB.x, ptScreenRB.y);
			//	int nWidth = ptScreenRB.x - ptScreenLT.x;
			//	int nHeight = ptScreenRB.y - ptScreenLT.y;
			//}

			int cntLink = pMesh->links.size();

			for (int jj = 0; jj < cntLink; jj++)
			{
				stLinkInfo* pLink = m_pFileMgr.GetLinkDataById(pMesh->links[jj]);
				if (pLink && pLink->sub_info != NOT_USE)
				{
					int cntVtx = pLink->vtPts.size();
					PointN ptScreen;
					CGnOSMCoord::Instance()->WorldToScreen(pLink->vtPts[0].x, pLink->vtPts[0].y, ptScreen.x, ptScreen.y);

					//if (!POINTINRECT(ptScreen.x, ptScreen.y, CGnOSMCoord::Instance()->m_ScreenRect.left, CGnOSMCoord::Instance()->m_ScreenRect.top, CGnOSMCoord::Instance()->m_ScreenRect.right, CGnOSMCoord::Instance()->m_ScreenRect.bottom))
					//{
					//	continue;
					//}


					PointF* pptWord = (PointF*)malloc(sizeof(PointF) * cntVtx);
					PointN* pptScreen = (PointN*)malloc(sizeof(PointN) * cntVtx);

					for (int kk = 0; kk < cntVtx; kk++)
					{
						pptWord[kk].x = pLink->vtPts[kk].x;
						pptWord[kk].y = pLink->vtPts[kk].y;
					}

					CGnOSMCoord::Instance()->WorldToScreen(cntVtx, pptWord, pptScreen);

					static const int arrowLength = 12;
					static const int arrowWidth = 12;

					if (pLink->link_type == TYPE_LINK_TRECKING)
					{
						g_drawLockZoomLevel = 4; // 숲길은 링크가 드므니까 더 그리자

						// 코스 속성을 바탕으로 그리고
						if (pLink->tre.course_type == TYPE_TRE_HIKING) { // 등산로
							col.rgba = RGBA_TRE_HIKING;
						}
						else if (pLink->tre.course_type == TYPE_TRE_TRAIL) { // 둘레길
							col.rgba = RGBA_TRE_TRAIL;
						}
						else if (pLink->tre.course_type == TYPE_TRE_BIKE) { // 자전거길
							col.rgba = RGBA_TRE_BIKE;
						}
						else if (pLink->tre.course_type == TYPE_TRE_CROSS) { // 종주길
							col.rgba = RGBA_TRE_CROSS;
						}
						else { // 미지정
							col.rgba = RGBA_TRE_DEFAULT;
						}

						g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 2.f, col.r, col.g, col.b, col.a );
						if (m_nCurrentZoom < g_drawLockZoomLevel) {
							g_UniDib.DrawArrowOnLine(pptScreen[cntVtx - 2].x, pptScreen[cntVtx - 2].y, pptScreen[cntVtx - 1].x, pptScreen[cntVtx - 1].y, arrowLength, arrowWidth, col.r, col.g, col.b, col.a);
						}

						// 코스 위에 노면을 그리자
						if (pLink->tre.road_info > 0)  {// 노면 정보
							int nRoadType = pLink->tre.road_info;

							// 어려운 길
							if ((nRoadType & ROUTE_AVOID_ROPE) == ROUTE_AVOID_ROPE) { // 8:밧줄 = 1000 0000 = 128
								col.rgba = RGBA_TRE_ROAD_ROPE;
							}
							else if ((nRoadType & ROUTE_AVOID_LADDER) == ROUTE_AVOID_LADDER) { // 7;사다리 = 100 0000 = 64
								col.rgba = RGBA_TRE_ROAD_LADDER;
							}
							else if ((nRoadType & ROUTE_AVOID_RIDGE) == ROUTE_AVOID_RIDGE) { // 6:릿지 = 10 0000 = 32
								col.rgba = RGBA_TRE_ROAD_RIDGE;
							}
							else if ((nRoadType & ROUTE_AVOID_ROCK) == ROUTE_AVOID_ROCK) { // 5:암릉 = 1 0000 = 16
								col.rgba = RGBA_TRE_ROAD_ROCK;
							}
							else if ((nRoadType & ROUTE_AVOID_TATTERED) == ROUTE_AVOID_TATTERED) { // 9:너덜길 = 1 0000 0000 = 256
								col.rgba = RGBA_TRE_ROAD_TATTERED;
							}

							// 편한길
							else if ((nRoadType & ROUTE_AVOID_DECK) == ROUTE_AVOID_DECK) { // 11:데크로드 = 1000 0000 0000 = 1024
								col.rgba = RGBA_TRE_ROAD_DECK;
							}
							else if ((nRoadType & ROUTE_AVOID_PALM) == ROUTE_AVOID_PALM) { // 10:야자수매트 = 100 0000 0000 = 512
								col.rgba = RGBA_TRE_ROAD_PALM;
							}
							else if ((nRoadType & ROUTE_AVOID_BRIDGE) == ROUTE_AVOID_BRIDGE) { // 4:교량 = 1000 = 8
								col.rgba = RGBA_TRE_ROAD_BRIDGE;
							}
							else if ((nRoadType & ROUTE_AVOID_STAIRS) == ROUTE_AVOID_STAIRS) { // 3:계단 = 100 = 4
								col.rgba = RGBA_TRE_ROAD_STAIRS;
							}
							else if ((nRoadType & ROUTE_AVOID_PAVE) == ROUTE_AVOID_PAVE) { // 2:포장길 = 10 = 2
								col.rgba = RGBA_TRE_ROAD_PAVE;
							}

							// 일반
							else if ((nRoadType & ROUTE_AVOID_ALLEY) == ROUTE_AVOID_ALLEY) { // 1:오솔길 = 1
								col.rgba = RGBA_TRE_ROAD_ALLEY;
							}
							else { // 미지정
								col.rgba = RGBA_TRE_ROAD_DEFAULT;
							}

							g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 1.f, col.r, col.g, col.b, col.a);
							if (m_nCurrentZoom < g_drawLockZoomLevel) {
								g_UniDib.DrawArrowOnLine(pptScreen[cntVtx - 2].x, pptScreen[cntVtx - 2].y, pptScreen[cntVtx - 1].x, pptScreen[cntVtx - 1].y, arrowLength, arrowWidth, col.r, col.g, col.b, col.a);
							}
						}
					}
					else if (pLink->link_type == TYPE_LINK_PEDESTRIAN)
					{
#if defined(USING_PED_BYCICLE_TYPE)
						if (pLink->ped.bycicle_type == TYPE_BYC_ONLY) { // 자전거 전용
							g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 1.f, 240, 150, 90, 255);
						}
						else if (pLink->ped.bycicle_type == TYPE_BYC_WITH_CAR) { // 차량 겸용
							g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 1.f, 50, 120, 30, 255);
						}
						else if (pLink->ped.bycicle_type == TYPE_BYC_WITH_WALK) { // 보행자 겸용
							g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 1.f, 50, 130, 240, 255);
						}
						else
#else
						if (pLink->ped.walk_type == TYPE_WALK_ONLY) { // 보행자 전용
							g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 1.f, 50, 130, 240, 255);
							if (m_nCurrentZoom < g_drawLockZoomLevel) {
								g_UniDib.DrawArrowOnLine(pptScreen[cntVtx - 2].x, pptScreen[cntVtx - 2].y, pptScreen[cntVtx - 1].x, pptScreen[cntVtx - 1].y, arrowLength, arrowWidth, 50, 130, 240, 255);
							}
						}
						else if (pLink->ped.walk_type == TYPE_WALK_WITH_CAR || pLink->ped.walk_type == TYPE_WALK_SIDE) { // 차량 겸용
							g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 1.f, 50, 120, 30, 255);
							if (m_nCurrentZoom < g_drawLockZoomLevel) {
								g_UniDib.DrawArrowOnLine(pptScreen[cntVtx - 2].x, pptScreen[cntVtx - 2].y, pptScreen[cntVtx - 1].x, pptScreen[cntVtx - 1].y, arrowLength, arrowWidth, 50, 120, 30, 255);
							}
						}
						else if (pLink->ped.walk_type == TYPE_WALK_WITH_BYC) { // 자전거 전용
							g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 1.f, 240, 150, 90, 255);
							if (m_nCurrentZoom < g_drawLockZoomLevel) {
								g_UniDib.DrawArrowOnLine(pptScreen[cntVtx - 2].x, pptScreen[cntVtx - 2].y, pptScreen[cntVtx - 1].x, pptScreen[cntVtx - 1].y, arrowLength, arrowWidth, 240, 150, 90, 255);
							}
						}
						else if (pLink->ped.walk_type == TYPE_WALK_THROUGH) { // 가상보행
							for (int kk = 0; kk < cntVtx - 1; kk++)
							{
								g_UniDib.ThickDashLine(pptScreen[kk].x, pptScreen[kk].y, pptScreen[kk + 1].x, pptScreen[kk + 1].y, 1.f, 5, 230, 60, 240, 255);
								if (m_nCurrentZoom < g_drawLockZoomLevel) {
									g_UniDib.DrawArrowOnLine(pptScreen[cntVtx - 2].x, pptScreen[cntVtx - 2].y, pptScreen[cntVtx - 1].x, pptScreen[cntVtx - 1].y, arrowLength, arrowWidth, 230, 60, 240, 255);
								}
							}
						}
					}
					else 
					{
						g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 1.f, 240, 240, 240, 255);
						if (m_nCurrentZoom < g_drawLockZoomLevel) {
							g_UniDib.DrawArrowOnLine(pptScreen[cntVtx - 2].x, pptScreen[cntVtx - 2].y, pptScreen[cntVtx - 1].x, pptScreen[cntVtx - 1].y, arrowLength, arrowWidth, 240, 240, 240, 255);
						}
					}
#endif
					
					// link idx
					if (m_nCurrentZoom < g_drawLockZoomLevel) {
						char szInfo[64] = { 0, };
						PointN ptCoordN;
						PointF ptCoordF;
						// 우선 현재는 링크의 중간 버텍스 좌표를 쓰자
						if (pLink->vtPts.size() <= 2) {
							ptCoordF.x = (pLink->vtPts[0].x + pLink->vtPts[1].x) / 2;
							ptCoordF.y = (pLink->vtPts[0].y + pLink->vtPts[1].y) / 2;
						}
						else {
							ptCoordF.x = pLink->vtPts[pLink->vtPts.size() / 2].x;
							ptCoordF.y = pLink->vtPts[pLink->vtPts.size() / 2].y;
						}
						CGnOSMCoord::Instance()->WorldToScreen(ptCoordF, ptCoordN);

						sprintf(szInfo, "%d", pLink->link_id.nid);
						COLORREF col = RGBA(0, 0, 0, 255);
						int retWidth = 0;
						int retHeight = 0;
						unsigned char* pLinkIdx = g_pFontMgr.GetTextImage(szInfo, 11, &retWidth, &retHeight);

						if (pLinkIdx) {
							g_UniDib.BltFromAlpha(ptCoordN.x, ptCoordN.y, pLinkIdx, retWidth, retHeight, 250, 250, 250);
						}
					}

					free(pptWord);
					free(pptScreen);
				}
			}
		}
	}

	if (m_vtClickedLink.size())
	{
		int cntVtx = m_vtClickedLink.size();
		PointN ptScreen;
		CGnOSMCoord::Instance()->WorldToScreen(m_vtClickedLink[0].x, m_vtClickedLink[0].y, ptScreen.x, ptScreen.y);

		//if (!POINTINRECT(ptScreen.x, ptScreen.y, CGnOSMCoord::Instance()->m_ScreenRect.left, CGnOSMCoord::Instance()->m_ScreenRect.top, CGnOSMCoord::Instance()->m_ScreenRect.right, CGnOSMCoord::Instance()->m_ScreenRect.bottom))
		//{
		//	continue;
		//}


		PointF* pptWord = (PointF*)malloc(sizeof(PointF) * cntVtx);
		PointN* pptScreen = (PointN*)malloc(sizeof(PointN) * cntVtx);

		for (int jj = 0; jj < cntVtx; jj++)
		{
			pptWord[jj].x = m_vtClickedLink[jj].x;
			pptWord[jj].y = m_vtClickedLink[jj].y;
		}

		CGnOSMCoord::Instance()->WorldToScreen(cntVtx, pptWord, pptScreen);

		g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 5.f, 50, 255, 50, 255);

		m_Drawing.DrawLinkInfo(m_pFileMgr.GetLinkDataById(m_keyClickedLinkId), m_rtViewport.right, m_rtViewport.bottom);

		free(pptWord);
		free(pptScreen);
	}


	m_Drawing.DrawColorInfo(0, m_rtViewport.right, m_rtViewport.bottom);
}

void MainView::DrawRoute()
{
	if (m_isRouting && m_cntRouteLine > 0)
	{
		CGnOSMCoord::Instance()->WorldToScreen(m_cntRouteLine, m_pRouteLineWorld, m_pRouteLineScreen);

		//g_UniDib.ThickPolyLine((iPOINT*)m_pRouteLineScreen, m_cntRouteLine, 5.f, 255, 100, 50, 255);
		g_UniDib.ThickPolyLine((iPOINT*)m_pRouteLineScreen, m_cntRouteLine, 3.f, 200, 50, 50, 255);

		const RouteResultInfo* pResult = m_pRouteMgr.GetRouteResult();
		if (pResult == nullptr) {
			AfxMessageBox(_T("Failed get route result"));
			return;
		}

		m_Drawing.DrawRouteInfo(pResult, m_rtViewport.right, m_rtViewport.bottom);
	}

	if (!m_mapRouteDraw.empty())
	{
		m_mapRouteDraw.clear();
	}
}

void MainView::DrawPoint()
{
	static string szStartImgFile = "C:/__Data/images/flag/maker_ss.png";
	static string szEndImgFile = "C:/__Data/images/flag/maker_se.png";
	int retWidth = 0;
	int retHeight = 0;
	PointN ptPoint;
	unsigned char* pImg = nullptr;

	// darw start point
	if (m_pRouteMgr.GetDeparture() != nullptr && CGnOSMCoord::Instance()->WorldToScreen(*(PointF*)m_pRouteMgr.GetDeparture(), ptPoint))
	{
		if (ptPoint.x >= 0 && ptPoint.y >= 0 && (pImg = g_pImageMgr.LoadImageFromFile((char*)szStartImgFile.c_str(), &retWidth, &retHeight)))
		{
			g_UniDib.BltFrom(ptPoint.x - (retWidth >> 1), (ptPoint.y - retHeight), pImg, retWidth, retHeight);
		}
	}

	// draw end point
	if (m_pRouteMgr.GetDestination() != nullptr && CGnOSMCoord::Instance()->WorldToScreen(*(PointF*)m_pRouteMgr.GetDestination(), ptPoint))
	{
		
		if (ptPoint.x >= 0 && ptPoint.y >= 0 && (pImg = g_pImageMgr.LoadImageFromFile((char*)szEndImgFile.c_str(), &retWidth, &retHeight)))
		{
			g_UniDib.BltFrom(ptPoint.x - (retWidth >> 1), ptPoint.y - retHeight, pImg, retWidth, retHeight);
		}
	}
}

void MainView::DrawRouteStatus(IN const unordered_map<uint64_t, CandidateLink>* pRouting)
{
	stLinkInfo* pLink = nullptr;
	unordered_map<uint64_t, bool>::iterator itDraw;

	for (unordered_map<uint64_t, CandidateLink>::const_iterator itProc = pRouting->begin(); itProc != pRouting->end(); itProc++)
	{
		if ((itDraw = m_mapRouteDraw.find(itProc->second.linkId.llid)) != m_mapRouteDraw.end())
		{
			if (itDraw->second == itProc->second.visited) {
				continue;
			}

			itDraw->second = true;
		}
		else
		{
			m_mapRouteDraw.insert({ itProc->second.linkId.llid, itProc->second.visited });
		}

		if ((pLink = m_pFileMgr.GetLinkDataById(itProc->second.linkId)))
		{
			int cntVtx = pLink->vtPts.size();

			PointN ptScreen;
			CGnOSMCoord::Instance()->WorldToScreen(pLink->vtPts[0].x, pLink->vtPts[0].y, ptScreen.x, ptScreen.y);


			PointF* pptWord = (PointF*)malloc(sizeof(PointF) * cntVtx);
			PointN* pptScreen = (PointN*)malloc(sizeof(PointN) * cntVtx);

			for (int jj = 0; jj < cntVtx; jj++)
			{
				pptWord[jj].x = pLink->vtPts[jj].x;
				pptWord[jj].y = pLink->vtPts[jj].y;
			}

			CGnOSMCoord::Instance()->WorldToScreen(cntVtx, pptWord, pptScreen);

			if (itProc->second.visited) {
				g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 1.f, 110, 40, 240, 255);

				if (m_nCurrentZoom < g_drawLockZoomLevel) {
					// 링크 계산 점수
					char szInfo[64] = { 0, };
					PointN ptCoordN;
					PointF ptCoordF;
					// 우선 현재는 링크의 중간 버텍스 좌표를 쓰자
					if (pLink->vtPts.size() <= 2) {
						ptCoordF.x = (pLink->vtPts[0].x + pLink->vtPts[1].x) / 2;
						ptCoordF.y = (pLink->vtPts[0].y + pLink->vtPts[1].y) / 2;
					}
					else {
						ptCoordF.x = pLink->vtPts[pLink->vtPts.size() / 2].x;
						ptCoordF.y = pLink->vtPts[pLink->vtPts.size() / 2].y;
					}
					CGnOSMCoord::Instance()->WorldToScreen(ptCoordF, ptCoordN);

					// cost
					sprintf(szInfo, "%d (%d)", (uint32_t)itProc->second.costTreavel, (uint32_t)itProc->second.costHeuristic);
					int retWidth = 0;
					int retHeight = 0;
					unsigned char* pLinkIdx = g_pFontMgr.GetTextImage(szInfo, 12, &retWidth, &retHeight);
					if (pLinkIdx) {
						g_UniDib.BltFromAlpha(ptCoordN.x, ptCoordN.y + 13, pLinkIdx, retWidth, retHeight, 255, 150, 10);
					}
				}
			}
			else {
				//g_UniDib.ThickPolyLine((iPOINT*)pptScreen, cntVtx, 2.f, 100, 200, 100, 255);
			}


			

			free(pptWord);
			free(pptScreen);
		}
	}

	Invalidate(FALSE);
	OnPaint();
}


static int g_szCenterLine = 50;

void MainView::DrawInfo()
{
	// draw center cross line
	g_UniDib.ThickLine(m_centerCross.x - g_szCenterLine, m_centerCross.y, m_centerCross.x + g_szCenterLine, m_centerCross.y, 1, 255, 0, 0, 100);
	g_UniDib.ThickLine(m_centerCross.x, m_centerCross.y - g_szCenterLine, m_centerCross.x, m_centerCross.y + g_szCenterLine, 1, 255, 0, 0, 100);

	// darw center coordnate
	char szInfo[64] = { 0, };
	PointN ptCoordN = m_centerCross;
	PointF ptCoordF = { 0, };
	CGnOSMCoord::Instance()->ScreenToWorld(ptCoordN, ptCoordF);
	sprintf(szInfo, "Center: %.5f, %.5f", ptCoordF.x, ptCoordF.y);

	COLORREF col = RGBA(0, 0, 0, 255);
	ptCoordN.x = CGnOSMCoord::Instance()->m_ScreenRect.left;
	ptCoordN.y = CGnOSMCoord::Instance()->m_ScreenRect.top;
	int retWidth = 0;
	int retHeight = 0;
	unsigned char* pCoord = g_pFontMgr.GetTextImage(szInfo, 20, &retWidth, &retHeight);
	if (pCoord)
	{
		g_UniDib.BltFromAlpha(ptCoordN.x + 20, ptCoordN.y + 150, pCoord, retWidth, retHeight, 255, 255, 0);
	}

	// darw zoom leveldd
	sprintf(szInfo, "Zoom:%d", m_nCurrentZoom);
	pCoord = g_pFontMgr.GetTextImage(szInfo, 20, &retWidth, &retHeight);
	if (pCoord)
	{
		g_UniDib.BltFromAlpha(ptCoordN.x + 20, ptCoordN.y + 175, pCoord, retWidth, retHeight, 255, 255, 0);
	}
}



void MainView::OnMapSetStart()
{
	if (m_pFileMgr.GetMeshCount() <= 0) {
		CString strMsg;
		strMsg.Format(_T("Please Load Shp Data!!!!!!!!!!!!!"));
		AfxMessageBox(strMsg);
		return;
	}


	CString strCoord;
	PointF ptWorld;

	CGnOSMCoord::Instance()->ScreenToWorld(m_ptLastRMouse.x, m_ptLastRMouse.y, ptWorld.x, ptWorld.y);
	strCoord.Format(_T("%.6f, %.6f"), ptWorld.x, ptWorld.y);

	GetDlgItem(IDC_EDT_START_COORD)->SetWindowTextW(strCoord);

	//CGnOSMCoord::Instance()->CenterFocus(ptWorld.x, ptWorld.y);

	m_pRouteMgr.SetDeparture(ptWorld.x, ptWorld.y);

	//DrawPoint();
	//Invalidate(TRUE);
	RedrawMap();
}


void MainView::OnMapSetWaypoint()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}


void MainView::OnMapSetEnd()
{
	if (m_pFileMgr.GetMeshCount() <= 0) {
		CString strMsg;
		strMsg.Format(_T("Please Load Shp Data!!!!!!!!!!!!!"));
		AfxMessageBox(strMsg);
		return;
	}


	CString strCoord;
	PointF ptWorld;

	CGnOSMCoord::Instance()->ScreenToWorld(m_ptLastRMouse.x, m_ptLastRMouse.y, ptWorld.x, ptWorld.y);
	strCoord.Format(_T("%.6f, %.6f"), ptWorld.x, ptWorld.y);

	GetDlgItem(IDC_EDT_END_COORD)->SetWindowTextW(strCoord);

	//CGnOSMCoord::Instance()->CenterFocus(ptWorld.x, ptWorld.y);

	m_pRouteMgr.SetDestination(ptWorld.x, ptWorld.y);

	//DrawPoint();
	//Invalidate(TRUE);
	RedrawMap();
}


void MainView::OnMapSetRoute()
{
	OnDoRoute();
}
