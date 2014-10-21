// FT.h : main header file for the FT application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CFTApp:
// See FT.cpp for the implementation of this class
//

class CFTApp : public CWinApp
{
public:
	CFTApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CFTApp theApp;