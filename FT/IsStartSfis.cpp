// IsStartSfis.cpp : implementation file
//

#include "stdafx.h"
#include "FT.h"
#include "IsStartSfis.h"


// CIsStartSfis dialog

IMPLEMENT_DYNAMIC(CIsStartSfis, CDialog)

CIsStartSfis::CIsStartSfis(CWnd* pParent /*=NULL*/)
	: CDialog(CIsStartSfis::IDD, pParent)
	, m_ProductName(_T(""))
{

}

CIsStartSfis::~CIsStartSfis()
{
}

void CIsStartSfis::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PCNAME, m_ProductName);
}


BEGIN_MESSAGE_MAP(CIsStartSfis, CDialog)
END_MESSAGE_MAP()


// CIsStartSfis message handlers
