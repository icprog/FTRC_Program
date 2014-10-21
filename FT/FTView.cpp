// FTView.cpp : implementation of the CFTView class
//

#include "stdafx.h"
#include "FT.h"

#include "FTDoc.h"
#include "FTView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFTView

IMPLEMENT_DYNCREATE(CFTView, CView)

BEGIN_MESSAGE_MAP(CFTView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
//	ON_WM_CLOSE()
END_MESSAGE_MAP()

// CFTView construction/destruction

CFTView::CFTView()
{
	// TODO: add construction code here

}

CFTView::~CFTView()
{
}

BOOL CFTView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

CRunInfo* CFTView::RetriveDlgPoint()
{
	return &m_RunInfo;
}
// CFTView drawing

void CFTView::OnDraw(CDC* /*pDC*/)
{
	CFTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CFTView printing

BOOL CFTView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFTView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFTView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CFTView diagnostics

#ifdef _DEBUG
void CFTView::AssertValid() const
{
	CView::AssertValid();
}

void CFTView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFTDoc* CFTView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFTDoc)));
	return (CFTDoc*)m_pDocument;
}
#endif //_DEBUG


// CFTView message handlers

int CFTView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	m_RunInfo.Create(IDD_RUNINFO,this);
	return 0;
}

void CFTView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	m_RunInfo.SetWindowPos(NULL,0,0,880,600,SWP_SHOWWINDOW);
	// TODO: Add your specialized code here and/or call the base class
}


