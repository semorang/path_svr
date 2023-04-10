
// TreckingView.cpp : CTreckingView 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "Trecking.h"
#endif

#include "TreckingDoc.h"
#include "TreckingView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CTreckingView

IMPLEMENT_DYNCREATE(CTreckingView, CView)

BEGIN_MESSAGE_MAP(CTreckingView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CTreckingView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
//	ON_WM_MOVE()
END_MESSAGE_MAP()

// CTreckingView 생성/소멸

CTreckingView::CTreckingView()
{
	// TODO: 여기에 생성 코드를 추가합니다.

}

CTreckingView::~CTreckingView()
{
}

BOOL CTreckingView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CTreckingView 그리기

void CTreckingView::OnDraw(CDC* /*pDC*/)
{
	CTreckingDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
}


// CTreckingView 인쇄


void CTreckingView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CTreckingView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CTreckingView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CTreckingView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}

void CTreckingView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CTreckingView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CTreckingView 진단

#ifdef _DEBUG
void CTreckingView::AssertValid() const
{
	CView::AssertValid();
}

void CTreckingView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTreckingDoc* CTreckingView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTreckingDoc)));
	return (CTreckingDoc*)m_pDocument;
}
#endif //_DEBUG


// CTreckingView 메시지 처리기


//void CTreckingView::OnMove(int x, int y)
//{
//	CView::OnMove(x, y);
//
//	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
//}
