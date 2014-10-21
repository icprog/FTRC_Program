#pragma once


// CIsStartSfis dialog

class CIsStartSfis : public CDialog
{
	DECLARE_DYNAMIC(CIsStartSfis)

public:
	CIsStartSfis(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIsStartSfis();

// Dialog Data
	enum { IDD = IDD_ISOPENSFIS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_ProductName;
};
