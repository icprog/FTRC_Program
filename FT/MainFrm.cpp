// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "FT.h"
#include "MainFrm.h"
#include "FTView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	m_wndToolBar.ShowWindow(SW_HIDE);
	m_wndStatusBar.ShowWindow(SW_HIDE);
	SetWindowLong(m_hWnd,GWL_STYLE,GetWindowLong(m_hWnd,GWL_STYLE)&~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX|WS_SIZEBOX));
	SetWindowPos(&wndTop,0,0,880,600,SWP_NOMOVE|SWP_SHOWWINDOW);
		
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style&=~FWS_ADDTOTITLE;
	char sztemp[10240];
	memset(sztemp , 0 ,sizeof(sztemp));
	sprintf_s(sztemp,"Station:%s version:%s Date:%s         %s_%s",
		PRODUCT_NAME,
		DIALOGUE_VERSION,
		PROGRAM_TIME , 
		_T(__DATE__) ,
		_T(__TIME__));
	cs.lpszName = _T(sztemp);
	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

#include "Winsock2.h"


class CRunInfo;

void CMainFrame::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	WSACleanup();
	
	CFTView *pView = ((CFTView*)GetActiveView());
	CRunInfo *pRunInfo = pView->RetriveDlgPoint();

	HKEY   hk;
	DWORD  dwDisp;
	char RegBuf[256] = "SYSTEM\\CurrentControlSet\\Services\\NETGEAR";
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,
					RegBuf,
					0,
					NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_WRITE | KEY_QUERY_VALUE,
					NULL,
					&hk,
					&dwDisp))
	{
		AfxMessageBox("Open registry fail...");
	}
	else
	{
		char szSSID[256] = "";
		char szChannel[256] = "";
		char szGoldenIPWireless[256] = "";
		char szGoldenIPEthernet[256] = "";

		char szChannel_2G_1[256] = "";
		char szChannel_2G_2[256] = "";
		char szChannel_5G_1[256] = "";
		char szChannel_5G_2[256] = "";

		int iSSID = ((CComboBox*)(pRunInfo->GetDlgItem(IDC_EDIT_SSID)))->GetCurSel();		
		
		sprintf_s(szSSID,256,"%d",iSSID);		
	
		pRunInfo->GetDlgItemText(IDC_IPADDRESS_GOLDENIP  , szGoldenIPWireless,256);
		pRunInfo->GetDlgItemText(IDC_IPADDRESS_GOLDENIP2 , szGoldenIPEthernet,256);
		
		pRunInfo->GetDlgItemText(IDC_EDIT_2G_CHANNEL1 , szChannel_2G_1,256);
		pRunInfo->GetDlgItemText(IDC_EDIT_2G_CHANNEL2 , szChannel_2G_2,256);
		pRunInfo->GetDlgItemText(IDC_EDIT_5G_CHANNEL1 , szChannel_5G_1,256);
		pRunInfo->GetDlgItemText(IDC_EDIT_5G_CHANNEL2 , szChannel_5G_2,256);

		if(RegSetValueEx(hk,"SSID",0,REG_SZ,(LPBYTE)szSSID,(DWORD)strlen(szSSID)+1)		
		|| RegSetValueEx(hk,"GoldenIP(W)",0,REG_SZ,(LPBYTE)szGoldenIPWireless,(DWORD)strlen(szGoldenIPWireless)+1)
		|| RegSetValueEx(hk,"CHANNEL_2G_1",0,REG_SZ,(LPBYTE)szChannel_2G_1,(DWORD)strlen(szChannel_2G_1)+1)
		|| RegSetValueEx(hk,"CHANNEL_2G_2",0,REG_SZ,(LPBYTE)szChannel_2G_2,(DWORD)strlen(szChannel_2G_2)+1)
		|| RegSetValueEx(hk,"CHANNEL_5G_1",0,REG_SZ,(LPBYTE)szChannel_5G_1,(DWORD)strlen(szChannel_5G_1)+1)
		|| RegSetValueEx(hk,"CHANNEL_5G_2",0,REG_SZ,(LPBYTE)szChannel_5G_2,(DWORD)strlen(szChannel_5G_2)+1)
		|| RegSetValueEx(hk,"GoldenIP(E)",0,REG_SZ,(LPBYTE)szGoldenIPEthernet,(DWORD)strlen(szGoldenIPEthernet)+1))
		{
			RegCloseKey(hk); 
		}
		
	}

	RegCloseKey(hk);
	
	shutdown(pRunInfo->m_Socket , 0);
	if(pRunInfo->m_Socket!=INVALID_SOCKET)
	{
		closesocket(pRunInfo->m_Socket);
		pRunInfo->m_Socket = INVALID_SOCKET;
	}
	shutdown(pRunInfo->m_GoldenSocket , 0);
	if(pRunInfo->m_GoldenSocket!=INVALID_SOCKET)
	{
		closesocket(pRunInfo->m_GoldenSocket);
		pRunInfo->m_GoldenSocket = INVALID_SOCKET;
	}

	CRunInfo *pRun = ((CFTView*)GetActiveView())->RetriveDlgPoint();
	CFrameWnd::OnClose();

}
