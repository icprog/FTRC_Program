// FTDoc.h : interface of the CFTDoc class
//


#pragma once


class CFTDoc : public CDocument
{
protected: // create from serialization only
	CFTDoc();
	DECLARE_DYNCREATE(CFTDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CFTDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


