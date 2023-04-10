
// TreckingView.h : CTreckingView Ŭ������ �������̽�
//

#pragma once


class CTreckingView : public CView
{
protected: // serialization������ ��������ϴ�.
	CTreckingView();
	DECLARE_DYNCREATE(CTreckingView)

// Ư���Դϴ�.
public:
	CTreckingDoc* GetDocument() const;

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual void OnDraw(CDC* pDC);  // �� �並 �׸��� ���� �����ǵǾ����ϴ�.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// �����Դϴ�.
public:
	virtual ~CTreckingView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnMove(int x, int y);
};

#ifndef _DEBUG  // TreckingView.cpp�� ����� ����
inline CTreckingDoc* CTreckingView::GetDocument() const
   { return reinterpret_cast<CTreckingDoc*>(m_pDocument); }
#endif

