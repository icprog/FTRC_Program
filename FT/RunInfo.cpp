// RunInfo.cpp : implementation file
//

#include "stdafx.h"
#include "FT.h"
#include "RunInfo.h"
#include "math.h"
#include "PTSC.h"
#include <direct.h>
#include "shlwapi.h"
#include "Tlhelp32.h"
#include <io.h>
#include "CompareProgram.h"
#include <stdio.h>
#include "ErrorCodeList.h"


#define WM_COMET WM_USER+1
#define UM_SOCK     WM_USER+4
#define UM_SOCK_UDP  WM_USER+5

#define RST_BTN_VALUE "Upgrading firmware, don't reboot"
#define WPS_BTN_VALUE "WPS button is pressed"
#define WIFI_BTN_VALUE "Upgrading firmware, don't config wireless"


char    m_strIMEI[20] = "";
char    szIMEIFromDUT[20] = "";
char    szRssiValue[MINBUFSIZE] = "";
float   CmpRsi=0.0;
float   CmpRso=0.0;
IMPLEMENT_DYNAMIC(CRunInfo, CDialog)

static CString strRunInfo;

static INIT_INFO m_Mydas;
void *pInstMydas;

CRunInfo *pRunInfo = NULL;
HANDLE hDev=NULL;
HANDLE hThread1;
HANDLE h_golenEvt;
bool i_flag=true;
_ButtonInfo ButtonInfo;
_GoldenInfo	GoldenInfo;
CString          g_data;

void Delay(unsigned long i)
{
    //TODO: Add your source code here
    unsigned long start = ::GetTickCount();
    while((::GetTickCount() - start) < i )
    {
		Sleep(1);
    }
}

CRunInfo::CRunInfo(CWnd* pParent )
	: CDialog(CRunInfo::IDD, pParent)
	, m_strSN(_T(""))
	, m_strMAC(_T(""))
	, m_nLanMAC(0)
	, m_nWanMAC(0)
	, m_strPincode(_T(""))  
	, m_strSfisStatus(_T(""))
	, m_strTestResult(_T(""))
	, m_strProductName(_T(""))
	, m_strTestTime(_T(""))
	, m_strErroCode(_T(""))
	, m_strSSN(_T(""))
	, m_FlukeAddr(3)
	, m_Station(_T(""))
{
	m_Socket = INVALID_SOCKET;
	m_GoldenSocket = INVALID_SOCKET;

	h_golenEvt = CreateEvent(NULL,true,false,NULL);

	m_TestTimeCount= 0;
	m_blIsOpenSfis= false;  
	m_blIsSfis = false;

	m_hEvt = NULL;
	m_hEvt = CreateEvent(NULL,true,false,NULL);
	strcpy_s(m_IniFileInfo.szDefaultIP1,MINBUFSIZE,"192.168.1.1");
}

CRunInfo::~CRunInfo()
{
	if(NULL != m_hEvt)
		CloseHandle(m_hEvt);
	if(m_Socket!=INVALID_SOCKET)
		closesocket(m_Socket);
	if(m_GoldenSocket!=INVALID_SOCKET)
		closesocket(m_GoldenSocket);
	WSACleanup();
}

bool  CRunInfo::SendDutCommand(char *pCmd, char *pRtn, int iDelay)
{ 
	int iSubDelay = iDelay/10;
	ResetEvent(m_hEvt);
	g_data="";
	SendEthernetCmd(pCmd);
	
	while(iSubDelay--)
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(m_hEvt,10))
		{
			if(pRtn==NULL)
			{
				return true;
			}
			else
			{
				if(strstr(g_data,pRtn) != NULL)
				{
					return true;
				}
			}	
			ResetEvent(m_hEvt);
		}		
	}	

	return false;
}

void CRunInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SN, m_strSN);
	DDX_Text(pDX, IDC_EDIT_MAC, m_strMAC);
	DDX_Text(pDX, IDC_EDIT_PINCODE, m_strPincode);
	DDX_CBString(pDX, IDC_EDIT_COM, m_strCom);
	DDX_Text(pDX, IDC_SFIS_STATUS, m_strSfisStatus);
	DDX_Text(pDX, IDC_RESULT, m_strTestResult);
	DDX_Control(pDX, IDC_BTN_START, m_BtnStart);
	DDX_Text(pDX, IDC_PRODUCTNAME, m_strProductName);
	DDX_Text(pDX, IDC_TESTTIME, m_strTestTime);
	DDX_Text(pDX, IDC_ERRORCODE, m_strErroCode);
	DDX_Control(pDX, IDC_BTN_QUIT, m_BtnQuit);
	DDX_Text(pDX, IDC_EDIT_CN, m_strSSN);
	DDV_MinMaxInt(pDX, m_FlukeAddr, 0, 10000);
	DDX_Text(pDX, IDC_EDIT_PASS_DUT, m_strPASS_2G);
	DDX_Text(pDX, IDC_EDIT_SSID_DUT, m_strSSID_2G);

	DDX_Text(pDX, IDC_STATION, m_Station);
}


BEGIN_MESSAGE_MAP(CRunInfo, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BTN_START, &CRunInfo::OnBnClickedBtnStart)
	ON_MESSAGE(WM_COMET,CRunInfo::OnRecvComData)
	ON_MESSAGE(UM_SOCK ,CRunInfo::OnRecvEthernetData)
	ON_BN_CLICKED(IDC_BTN_QUIT, &CRunInfo::OnBnClickedBtnQuit)
	ON_CBN_SELCHANGE(IDC_COM, &CRunInfo::OnCbnSelchangeCom)
END_MESSAGE_MAP()


// CRunInfo message handlers

HBRUSH CRunInfo::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	if(IDC_PRODUCTNAME == pWnd->GetDlgCtrlID())
	{
		pDC->SelectObject(&m_Midft);
		pDC->SetTextColor(RGB(255,255,0));
		pDC->SetBkMode(TRANSPARENT);
		return m_Grancybh;
	}
	if(IDC_PASS_COUNT == pWnd->GetDlgCtrlID())
	{
	//	pDC->SelectObject(&m_Minft);
		pDC->SetTextColor(RGB(0,0,255));
	//	pDC->SetBkMode(TRANSPARENT);
	}
	if(IDC_FAIL_COUNT == pWnd->GetDlgCtrlID())
	{
	//	pDC->SelectObject(&m_Minft);
		pDC->SetTextColor(RGB(255,0,0));
	//	pDC->SetBkMode(TRANSPARENT);
	}
	if(IDC_STATION == pWnd->GetDlgCtrlID())
	{
		pDC->SelectObject(&m_Midft);
		pDC->SetTextColor(RGB(255,255,0));
		pDC->SetBkMode(TRANSPARENT);
		return m_Grancybh;
	}
	if(IDC_RESULT==pWnd->GetDlgCtrlID())
	{
		pDC->SelectObject(&m_Maxft);
		pDC->SetBkMode(TRANSPARENT);

		if(!strcmp("StandBy", m_strTestResult))
		{
			pDC->SetTextColor(RGB(255,255,0));
			return m_Grancybh;
		}
		if(!strcmp("RUN", m_strTestResult))
		{
			pDC->SetTextColor(RGB(0,255,0));
			return m_Yellowbh;
		}
		if(!strcmp("PASS", m_strTestResult))
		{
			pDC->SetTextColor(RGB(0,0,255));
			return m_Greenbh;
		}
		if(!strcmp("FAIL", m_strTestResult))
		{
			pDC->SetTextColor(RGB(255,255,255));
			return m_Redbh;
		}
	}
	if(IDC_SFIS_STATUS==pWnd->GetDlgCtrlID())
	{
		pDC->SelectObject(&m_Minft);
		pDC->SetBkMode(TRANSPARENT);
		if(!strcmp("SFIS OFF", m_strSfisStatus))
		{
			pDC->SetTextColor(RGB(255,255,0));
			return m_Redbh;
		}
		if(!strcmp("SFIS ON", m_strSfisStatus))
		{
			pDC->SetTextColor(RGB(255,255,0));
			return m_Greenbh;		
		}
	}
	if(IDC_PASS == pWnd->GetDlgCtrlID())
	{
	//	pDC->SelectObject(&m_Minft);
		pDC->SetTextColor(RGB(0,0,255));
	//	pDC->SetBkMode(TRANSPARENT);
	}
	if(IDC_FAIL == pWnd->GetDlgCtrlID())
	{
	//	pDC->SelectObject(&m_Minft);
		pDC->SetTextColor(RGB(255,0,0));
	//	pDC->SetBkMode(TRANSPARENT);
	}
	if(IDC_TESTTIME == pWnd->GetDlgCtrlID())
	{
		pDC->SelectObject(&m_Minft);
		pDC->SetTextColor(RGB(0,0,255));
		pDC->SetBkMode(TRANSPARENT);
	}
	if(IDC_ERROR == pWnd->GetDlgCtrlID())
	{
		pDC->SelectObject(&m_SupMinft);
		pDC->SetTextColor(RGB(0,0,255));
		pDC->SetBkMode(TRANSPARENT);
	}
	if(IDC_ERRORCODE == pWnd->GetDlgCtrlID())
	{
		pDC->SelectObject(&m_SupMinft);
		pDC->SetTextColor(RGB(255,0,0));
		pDC->SetBkMode(TRANSPARENT);
	}
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

BOOL CRunInfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_SupMinft.CreatePointFont(100, _T("Arial"));
	m_Minft.CreatePointFont(200, _T("Arial"));
	m_Midft.CreatePointFont(350, _T("Arial"));
	m_Maxft.CreatePointFont(1000,_T("Arial"));

	m_Redbh.CreateSolidBrush(RGB(255,0,0));
	m_Greenbh.CreateSolidBrush(RGB(0,255,0));
	m_Yellowbh.CreateSolidBrush(RGB(255,255,0));
	m_Grancybh.CreateSolidBrush(RGB(82,64,111));

	m_ComSfis.SetParentWnd(m_hWnd);
	char szProductName[30] = "";	
	gethostname(szProductName,30);
	_strupr_s(szProductName,30);
	m_strPcName = szProductName;
	if(m_strPcName.GetLength() > 12)
	{
		m_strPcName.Delete(12,m_strPcName.GetLength()-12);
	}
	else
	{
		int len = m_strPcName.GetLength();
		for(int i = len; i<12; i++)
			m_strPcName +='-';
	}
	SetDlgItemText(IDC_EDIT_PCNAME,m_strPcName);

	((CComboBox*)GetDlgItem(IDC_COM))->AddString(_T("COM1"));
	((CComboBox*)GetDlgItem(IDC_COM))->AddString(_T("COM2"));
	((CComboBox*)GetDlgItem(IDC_COM))->AddString(_T("COM3"));
	((CComboBox*)GetDlgItem(IDC_COM))->AddString(_T("COM4"));
	((CComboBox*)GetDlgItem(IDC_COM))->AddString(_T("COM5"));
	m_strCom = _T("COM1");
	((CComboBox*)GetDlgItem(IDC_COM))->SetCurSel(0);

	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR01"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR02"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR03"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR04"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR05"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR06"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR07"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR08"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR09"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR0A"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR0B"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR0C"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR0D"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR0E"));
	((CComboBox*)GetDlgItem(IDC_SSID))->AddString(_T("NETGEAR0F"));
	((CComboBox*)GetDlgItem(IDC_SSID))->SetCurSel(0);

	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->AddString(_T("3"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->AddString(_T("4"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->AddString(_T("5"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->AddString(_T("6"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->AddString(_T("7"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->AddString(_T("8"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->AddString(_T("9"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->SetCurSel(0);

	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->AddString(_T("3"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->AddString(_T("4"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->AddString(_T("5"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->AddString(_T("6"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->AddString(_T("7"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->AddString(_T("8"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->AddString(_T("9"));
	((CComboBox*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->SetCurSel(3);

	((CComboBox*)GetDlgItem(IDC_EDIT_5G_CHANNEL1))->AddString(_T("153"));
	((CComboBox*)GetDlgItem(IDC_EDIT_5G_CHANNEL2))->AddString(_T("153"));
	((CComboBox*)GetDlgItem(IDC_EDIT_5G_CHANNEL1))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_EDIT_5G_CHANNEL2))->SetCurSel(0);

	((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_GOLDENIP2))->SetAddress(10,0,0,2);
	((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_GOLDENIP))->SetAddress(192,168,1,100);
	
	m_strTestResult = _T("StandBy");

	m_strSfisStatus = _T("SFIS OFF");

	m_strSN = _T("0000001");

	m_strMAC = _T("000000000001");

	m_strPincode = _T("12345670");

	m_strSSN = _T("0000000000001"); 

	m_strSSID_2G = _T("NETGEAR");

	m_strPASS_2G = _T("NETGEAR_PASS");

	UpdateData(false);

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
		DisplayRunTimeInfo("Create registry fail");
	}
	else
	{
		if(dwDisp == REG_CREATED_NEW_KEY)
		{
			DisplayRunTimeInfo("Create new key");
			RegCloseKey(hk);
		}
		else
		{
			DisplayRunTimeInfo("Read old key");
			char szSSID[256] = "";			
			char szGoldenIPWireless[256] = "";
			char szGoldenIPEthernet[256] = "";
			int iSSID;
			char szChannel_2G_1[256] = "";
			char szChannel_2G_2[256] = "";
			char szChannel_5G_1[256] = "";
			char szChannel_5G_2[256] = "";

			
			DWORD dwBuf1 = 256,dwBuf3 = 256,dwBuf4 = 256;
			DWORD dwBuf11 = 256,dwBuf12 = 256,dwBuf13 = 256,dwBuf14 = 256;
			if(RegQueryValueEx(hk,"SSID",NULL,NULL,(LPBYTE)szSSID,&dwBuf1)		
			 ||RegQueryValueEx(hk,"GoldenIP(W)",NULL,NULL,(LPBYTE)szGoldenIPWireless,&dwBuf3)

			 ||RegQueryValueEx(hk,"CHANNEL_2G_1",NULL,NULL,(LPBYTE)szChannel_2G_1,&dwBuf11)
			 ||RegQueryValueEx(hk,"CHANNEL_2G_2",NULL,NULL,(LPBYTE)szChannel_2G_2,&dwBuf12)
			 ||RegQueryValueEx(hk,"CHANNEL_5G_1",NULL,NULL,(LPBYTE)szChannel_5G_1,&dwBuf13)
			 ||RegQueryValueEx(hk,"CHANNEL_5G_2",NULL,NULL,(LPBYTE)szChannel_5G_2,&dwBuf14)

			 ||RegQueryValueEx(hk,"GoldenIP(E)",NULL,NULL,(LPBYTE)szGoldenIPEthernet,&dwBuf4))
			{
				DisplayRunTimeInfo("Get Info from registry fail");
				RegCloseKey(hk);
			}
			else
			{
				iSSID = atoi(szSSID);

				((CComboBox*)GetDlgItem(IDC_EDIT_SSID))->SetCurSel(iSSID);					
				
				((CIPAddressCtrl*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->SetWindowText(szChannel_2G_1);

				((CIPAddressCtrl*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->SetWindowText(szChannel_2G_2);

				((CIPAddressCtrl*)GetDlgItem(IDC_EDIT_5G_CHANNEL1))->SetWindowText(szChannel_5G_1);

				((CIPAddressCtrl*)GetDlgItem(IDC_EDIT_5G_CHANNEL2))->SetWindowText(szChannel_5G_2);

				((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_GOLDENIP))->SetWindowText(szGoldenIPWireless);

				((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_GOLDENIP2))->SetWindowText(szGoldenIPEthernet);

				RegCloseKey(hk);
			}
		}
	}

	if(!NewReadIniFile())
	{
		AfxMessageBox("請確保程序文件目錄下有init.ini！Plearse make sure init.ini at program path!");
	}

	if(!ReadIniFileForGeneric())
	{
		m_IniFileInfo.nChannelNum_2G_1 = 3;
		m_IniFileInfo.nChannelNum_5G_1 = 3;
		m_IniFileInfo.nChannelNum_2G_2 = 3;
		m_IniFileInfo.nChannelNum_5G_2 = 3;
	}	


	if(IDYES==AfxMessageBox(_T("是否要加入SFIS\nSFIS ON/OFF?"),MB_YESNO|MB_ICONQUESTION))
	{
		m_blIsSfis = true;
		m_BtnStart.EnableWindow(FALSE);
		m_BtnQuit.EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_SN))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_MAC))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_PINCODE))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_CN))->EnableWindow(false);
		((CStatic*)GetDlgItem(IDC_EDIT_SSID_DUT))->EnableWindow(false);
		((CStatic*)GetDlgItem(IDC_EDIT_PASS_DUT))->EnableWindow(false);
		((CStatic*)GetDlgItem(IDC_EDIT_SSID))->EnableWindow(false);

		char szPath[MAX_PATH] = "";
		char szFullPath[MAX_PATH] = "";
		char szFullPath1[MAX_PATH] = "";
		GetCurrentDirectory(MAX_PATH, szPath);
		sprintf_s(szFullPath, MAX_PATH, "%s\\%s", szPath , m_IniFileInfo.szTestInfoFileName);
		sprintf_s(szFullPath1, MAX_PATH, "%s\\%s", m_IniFileInfo.szServerFilePath,m_IniFileInfo.szTestInfoFileName);

		////Add compare program file last wirte time and checksum at local and server
		//char szProgramState[4096];	
		//char szLocalPath[256];
		//memset(szLocalPath , 0 , 256);
		//memset(szProgramState , 0 , 4096);
		//GetCurrentDirectory(256, szLocalPath);
		//if(AutoDownLoadProgram(szLocalPath	, m_IniFileInfo.szServerFilePath , szProgramState))
		//{
		//	DisplayRunTimeInfo("程序更新完成!Program upgrade finish!");
		//}
		//DisplayRunTimeInfo(szProgramState);
		////End

		if(!CopyFile(szFullPath1,szFullPath , false))
		{
			AfxMessageBox("請确定server是否映射到本地F:盤,請關閉程式!\n");
			SendSfisResultAndShowFail(SY05);
			return true;
		}
		
		if(!m_ComSfis.Open("com1", 9600))
		{
			AfxMessageBox("默認連接SFIS的COM1口不存在或者被其他應用程序占用\n請從新選擇COM與SFIS連接\nThe default COM1 don't exit or is used by other application!\nPls choose COM port again");
			return TRUE;
		}

		m_ComSfis.SendData(("Hello\r\n"), (int)strlen("Hello\r\n"));

		char pBuf[MINBUFSIZE] = "";
		m_ComSfis.ReadData(pBuf, MINBUFSIZE, 1500);

		if(strstr(pBuf, "ERRO") == NULL)  
		{
			AfxMessageBox(_T("請從新選擇COM與SFIS連接\n"));
			m_ComSfis.Close();
			return TRUE;
		}
		else
		{
			m_strSfisStatus = _T("SFIS ON");
			UpdateData(false);
			m_blIsOpenSfis = true;
			AfxBeginThread(ReadComForSfis, &m_ComSfis);
		}
	}
	else
	{
		if(IDOK == m_IsStartSfis.DoModal())
		{
			CString  strName = m_IsStartSfis.m_ProductName;
			if(strName.IsEmpty())
			{
				AfxMessageBox(_T("請輸入机种名稱,不能為空"));
				::PostMessage(GetParent()->GetParent()->GetSafeHwnd(),WM_CLOSE,0,0);
			}
			else
			{
				m_strProductName = m_IsStartSfis.m_ProductName;
				_strupr_s(m_strProductName.GetBuffer(m_strProductName.GetLength()), m_strProductName.GetLength()+1);
				UpdateData(false);
				if(!ReadIniFile())
				{
					AfxMessageBox("請重新配置INI檔,確保有匹配的機種名稱,程式自動關閉!\n");
					::PostMessage(GetParent()->GetParent()->GetSafeHwnd(),WM_CLOSE,0,0);
					return TRUE;
				}
			}
		}
		else
		{
			::PostMessage(GetParent()->GetParent()->GetSafeHwnd(),WM_CLOSE,0,0);
		}
	}

	return TRUE;  
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRunInfo::SendSfisResultAndShowFail(char* ErroCode)
{  
	char szErrorCode[MINBUFSIZE] = "";
	char szSendSfisData[MINBUFSIZE] = "";
	strncpy_s(szErrorCode, MINBUFSIZE, ErroCode, 4);
	DisplayRunTimeInfo(ErroCode);
	if(m_blIsOpenSfis)
	{	
		 ShowMsg("*****************Send test Result(Fail) to SFIS*****************************");
		if(strstr(m_IniFileInfo.szTestStatuse , "FT"))/*FT SFIS mode*/
		{
			if(strcmp(m_IniFileInfo.szIsHaveSSIDLable , "1") == 0)
			{

				/*check Version*/
				if(strcmp(m_IniFileInfo.szUpLoadVersion,"1") == 0)
				{
					/*HH(25)+MAC(12)+PIN(12)+SSN(15)+SSID(32)+pass(32)+SSID(32)+pass(32)+FW(30)+PCNAME(12)+Err(4)*/
					sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-15s%-32s%-32s%-32s%-32s%-30s%-12s%-4s\r\n", 
						m_strSN , 
						m_strMAC , 
						m_strPincode , 
						m_strSSN , 
						m_strSSID_2G ,
						m_strPASS_2G ,
						m_strSSID_5G ,
						m_strPASS_5G ,
						szFWforSfis , 
						m_strPcName , 
						szErrorCode);//ADD
				}
				else
				{
					/*HH(25)+MAC(12)+PIN(12)+SSN(15)+SSID(32)+pass(32)+SSID(32)+pass(32)+PCNAME(12)+Err(4)*/
					sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-15s%-32s%-32s%-32s%-32s%-12s%-4s\r\n", 
						m_strSN, 
						m_strMAC, 
						m_strPincode, 
						m_strSSN, 
						m_strSSID_2G ,
						m_strPASS_2G ,
						m_strSSID_5G ,
						m_strPASS_5G ,
						m_strPcName , 
						szErrorCode);//ADD
				}
			}
			else
			{
				/*check Version*/
				if(strcmp(m_IniFileInfo.szUpLoadVersion,"1") == 0)
				{
					/*HH(25)+MAC(12)+PIN(12)+SSN(15)+FW(30)+PCNAME(12)+Err(4)*/
					sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-15s%-30s%-12s%-4s\r\n", m_strSN , m_strMAC , m_strPincode , m_strSSN , szFWforSfis , m_strPcName , szErrorCode);//ADD
				}
				else
				{
					/*HH(25)+MAC(12)+PIN(12)+SSN(15)+PCNAME(12)+Err(4)*/
					sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-15s%-12s%-4s\r\n", m_strSN, m_strMAC, m_strPincode, m_strSSN, m_strPcName , szErrorCode);
				}
			}
		}
		if(strstr(m_IniFileInfo.szTestStatuse , "RC"))/*RC SFIS mode*/
		{
			if(strcmp(m_IniFileInfo.szIsHaveSSIDLable , "1") == 0)
			{
				/*HH(25)+MAC(12)+PIN(12)+SSN(15)+SSID(32)+pass(32)+SSID(32)+pass(32)+PCNAME(12)+Err(4)*/
				sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-15s%-32s%-32s%-32s%-32s%-12s%-4s\r\n", 
					m_strSN, 
					m_strMAC, 
					m_strPincode, 
					m_strSSN, 
					m_strSSID_2G ,
					m_strPASS_2G ,
					m_strSSID_5G ,
					m_strPASS_5G ,
					m_strPcName , 
					szErrorCode);//ADD
			}
			else
			{
				/*HH(25)+MAC(12)+PIN(12)+SSN(15)+PCNAME(12)+Err(4)*/
				sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-15s%-12s%-4s\r\n", m_strSN, m_strMAC, m_strPincode, m_strSSN, m_strPcName , szErrorCode);
			}
		}
		if(strstr(m_IniFileInfo.szTestStatuse , "PT2"))/*PT2 SFIS mode*/
		{
			/*HH(25)+MAC(12)+PCNAME(12)+Err(4)*/
			sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-4s\r\n", m_strSN, m_strMAC, m_strPcName , szErrorCode);//ADD
		}
		ShowMsg(szSendSfisData);
		m_ComSfis.SendData(szSendSfisData, MINBUFSIZE);
	}	
	m_strRecordTestData  +=  "\t";
	m_strRecordTestData  += szErrorCode;
	TestFail(ErroCode);
}

void CRunInfo::SendSfisResultAndShowPass()
{
	if(m_blIsOpenSfis)
	{
		ShowMsg("*****************Send Result(Pass) to SFIS ******************");
		char szSendSfisData[MINBUFSIZE] = "";
		if(strstr(m_IniFileInfo.szTestStatuse , "FT"))/*FT SFIS mode*/
		{
			if(strcmp(m_IniFileInfo.szIsHaveSSIDLable , "1") == 0)
			{
				/*upload Version*/
				if(strcmp(m_IniFileInfo.szUpLoadVersion,"1") == 0)
				{
					/*HH(25)+MAC(12)+PIN(12)+SSN(15)+SSID(32)+PASS(32)+SSID(32)+PASS(32)+FW(30)+PCNAME(12)*/
					//AfxMessageBox(szSendSfisData);
					sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-15s%-30s%-32s%-32s%-32s%-32s%-12s\r\n", 
						m_strSN ,
						m_strMAC , 
						m_strPincode , 
						m_strSSN ,
						szFWforSfis ,
						m_strSSID_2G ,
						m_strPASS_2G ,
						m_strSSID_5G ,
						m_strPASS_5G ,						
						m_strPcName) ;
				}
				else
				{
					/*HH(25)+MAC(12)+PIN(12)+SSN(15)+SSID(32)+PASS(32)+SSID(32)+PASS(32)+PCNAME(12)*/
					sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-15s%-32s%-32s%-32s%-32s%-12s\r\n", 
						m_strSN, 
						m_strMAC, 
						m_strPincode , 
						m_strSSN, 
						m_strSSID_2G ,
						m_strPASS_2G ,
						m_strSSID_5G ,
						m_strPASS_5G ,
						m_strPcName) ;
				}
			}
			else
			{
				/*upload Version*/
				if(strcmp(m_IniFileInfo.szUpLoadVersion,"1") == 0)
				{
					/*HH(25)+MAC(12)+PIN(12)+SSN(15)+FW(30)+PCNAME(12)*/
					sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-15s%-30s%-12s\r\n", m_strSN , m_strMAC , m_strPincode , m_strSSN , szFWforSfis , m_strPcName);//
				}
				else
				{
					/*HH(25)+MAC(12)+PIN(12)+SSN(15)+PCNAME(12)*/
					sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-15s%-12s\r\n", m_strSN, m_strMAC, m_strPincode, m_strSSN, m_strPcName);//ADD
				}
			}
		}
		if(strstr(m_IniFileInfo.szTestStatuse , "RC"))/*RC SFIS mode*/
		{
			/*HH(25)+MAC(12)+PIN(12)+SSN(15)+SSID(32)+PASS(32)+SSID(32)+PASS(32)+PCNAME(12)*/
			sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s%-15s%-32s%-32s%-32s%-32s%-12s\r\n", 
				m_strSN, 
				m_strMAC, 
				m_strPincode, 
				m_strSSN, 
				m_strSSID_2G ,
				m_strPASS_2G ,
				m_strSSID_5G ,
				m_strPASS_5G ,
				m_strPcName);

		}
		if(strstr(m_IniFileInfo.szTestStatuse , "PT2"))/*PT2 SFIS mode*/
		{
			/*HH(25)+MAC(12)+PCNAME(12)*/
			sprintf_s(szSendSfisData, MINBUFSIZE, "%-25s%-12s%-12s\r\n", m_strSN, m_strMAC, m_strPcName);//ADD
		}
		ShowMsg(szSendSfisData);
		m_ComSfis.SendData(szSendSfisData, MINBUFSIZE);		
	}
	else
	{
		TestPass();
	}
}
void CRunInfo::TestFail(char* ErroCode)
{
    ShowMsg("*************** Test Function Fail **********************");
	CRunInfo*pRun=(CRunInfo*)(ErroCode);

	if(!m_blIsSfis)
	{
		m_BtnStart.EnableWindow(true);
		m_BtnQuit.EnableWindow(true);
	}

	m_strErroCode = ErroCode;
	bool  szPostToMydas = true;	
	if(strncmp(ErroCode, "BR", 2) ==0 || strncmp(ErroCode, "SR", 2) == 0 || strncmp(ErroCode, "SY", 2) == 0)//add
	{
		
		szPostToMydas =false;		
	}
	CountTestResult(false);

    char szRecordTestTime[MINBUFSIZE] = "";
	sprintf_s(szRecordTestTime, MINBUFSIZE, "%d(s)", m_TestTimeCount);
	m_strRecordTestData +='\t';
	m_strRecordTestData += szRecordTestTime;	

	m_strTestResult = "FAIL";
	if(m_blIsSfis)
	{
		char szRecordFailFilePath  [MAXBUFSIZE] = "";
		CTime    Time = CTime::GetCurrentTime();
		CString    strFileName = Time.Format("%Y%m%d");
		sprintf_s(szRecordFailFilePath, MAXBUFSIZE, "%s\\%s\\FAIL\\%s.txt", m_IniFileInfo.szStoreSrvPath,m_IniFileInfo.szTestStatuse, strFileName);
		fsFail.open (szRecordFailFilePath   ,fstream::out|fstream::app);
		fsFail<<m_strRecordTestData.GetBuffer(m_strRecordTestData.GetLength())<<endl;
		fsFail.close();
		ShowMsg(" ******Save Fail test log OK*******");
		
		
		if(strstr(m_IniFileInfo.szPostFlag,"1") && szPostToMydas) 
		{
			if(LinkDataBase())
			{
				ShowMsg("*******************Send FAIL Restust to MYDAS ****************");
				/*Detaillog*/
				sprintf_s(Detaillog,MINBUFSIZE,"%s,%s,%s,%s,%s,%s",
					m_strProductName,
					m_strSN.Trim(),
					m_strMAC.Trim(),
					m_strPincode.Trim(),
					m_strSSN.Trim(),
					TestDetaillog);
				/*ErrorDefine*/
				char temp[1024] = "";
				CString strErrInfo = ErroCode;
				sprintf_s(temp,1024,"%s,%s,|",strErrInfo.Trim().Left(4),strErrInfo.Trim().Mid(5,strErrInfo.GetLength()-5).Trim());
				strcpy_s(ErrorDefine,sizeof(ErrorDefine),temp);
				/*MainData*/
				SYSTEMTIME SystemTime;
				GetSystemTime(&SystemTime);
				char time[MINBUFSIZE] = "";
				sprintf_s(time,MINBUFSIZE,"%d-%d-%d %d:%d:%d",SystemTime.wYear,SystemTime.wMonth,SystemTime.wDay,SystemTime.wHour,SystemTime.wMinute,SystemTime.wSecond);
				sprintf_s(MainData,MINBUFSIZE,"%s,0,%s,V0.0.1,%s,%s,%s,%s,%s,%s,",
					m_strSN.Trim(),
					szRecordFailFilePath,
					m_strPcName,
					m_TestTimeCount.Format("%H%M%S"),
					time,
					m_strMAC.Trim(),
					m_strPincode.Trim(),
					m_strSSN.Trim());
				if(!SendDatatoMYDAS(Detaillog,ErrorDefine,MainData))
				{
					AfxMessageBox("Send Information to MYDAS Fail...");
				}
			}
			else
			{
				AfxMessageBox("Link to MYDAS Fail...");
			}
		}
	}

	/*Recode fail test UI log*/
	char sztemp[1024] = "";
	CString strErrInfo = ErroCode;
	sprintf_s(sztemp,1024,"%s",strErrInfo.Trim().Left(4));

	char szRecordFailFilePath_UI  [MAXBUFSIZE] = "";
	sprintf_s(szRecordFailFilePath_UI, MAXBUFSIZE, "D:\\FAIL\\%s_%s.txt", m_strMAC.Trim() , sztemp );
	fsFail.open (szRecordFailFilePath_UI   ,fstream::out|fstream::app);
	fsFail<<strRunInfo.GetBuffer(strRunInfo.GetLength())<<endl;
	fsFail.close();
	/*End*/

	KillTimer(1);
	IsDisplayErrorCode(true);
	UpdateData(false);

	m_strSN = "";
	m_strMAC = "";
	m_strPincode = "";
	m_strSSN = "";
	if(m_Socket!=INVALID_SOCKET)
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	shutdown(m_GoldenSocket , 0);	
	if(m_GoldenSocket!=INVALID_SOCKET)		
	{
		closesocket(m_GoldenSocket);
		m_GoldenSocket=INVALID_SOCKET;
	}
	return;
}

void CRunInfo::TestPass()
{
    ShowMsg("*************** Test Function PASS **********************");
	CRunInfo*pRun=(CRunInfo*)(NULL) ;

	if(!m_blIsSfis)
	{
		m_BtnStart.EnableWindow(true);
		m_BtnQuit.EnableWindow(true);
	}
	m_strTestResult = "PASS";
	char szRecordTestTime[MINBUFSIZE] = "";
	sprintf_s(szRecordTestTime, MINBUFSIZE, "%d(s)", m_TestTimeCount);
	m_strRecordTestData +='\t';
	m_strRecordTestData += szRecordTestTime;
	if(m_blIsSfis)
	{
		char       szRecordPassFilePath[MAXBUFSIZE] = "";
		CTime      Time = CTime::GetCurrentTime();
		CString    strFileName = Time.Format("%Y%m%d");
		sprintf_s(szRecordPassFilePath, MAXBUFSIZE, "%s\\%s\\PASS\\%s.txt", m_IniFileInfo.szStoreSrvPath,m_IniFileInfo.szTestStatuse, strFileName);
		fsPass.open(szRecordPassFilePath,fstream::out|fstream::app);
		fsPass<<m_strRecordTestData.GetBuffer(m_strRecordTestData.GetLength())<<endl;

		fsPass.close();
		ShowMsg(" ******Save Pass test log OK*******");
		if(strstr(m_IniFileInfo.szPostFlag,"1"))
		{
			if(LinkDataBase())
			{
				ShowMsg("*******************Send PASS Restust to MYDAS ****************");
				/*Detaillog*/
				sprintf_s(Detaillog,MINBUFSIZE,"%s,%s,%s,%s,%s,%s",
					m_strProductName,
					m_strSN.Trim(),
					m_strMAC.Trim(),
					m_strPincode.Trim(),
					m_strSSN.Trim(),
					TestDetaillog);
				/*ErrorDefine*/
				sprintf_s(ErrorDefine,",,|");
				/*MainData*/
				SYSTEMTIME SystemTime;
				GetSystemTime(&SystemTime);
				char time[MINBUFSIZE] = "";
				sprintf_s(time,MINBUFSIZE,"%d-%d-%d %d:%d:%d",SystemTime.wYear,SystemTime.wMonth,SystemTime.wDay,SystemTime.wHour,SystemTime.wMinute,SystemTime.wSecond);
				sprintf_s(MainData,MINBUFSIZE,"%s,1,%s,V0.0.1,%s,%s,%s,%s,%s,%s,",
					m_strSN.Trim(),
					szRecordPassFilePath,
					m_strPcName,
					m_TestTimeCount.Format("%H%M%S"),
					time,
					m_strMAC.Trim(),
					m_strPincode.Trim(),
					m_strSSN.Trim());
				if(!SendDatatoMYDAS(Detaillog,ErrorDefine,MainData))
				{
					AfxMessageBox("Send Information to MYDAS Fail...");
				}
			}
			else
			{
				AfxMessageBox("Link to MYDAS Fail...");
			}
		}
	}

	/*Recode pass test UI log*/
	char szRecordPassFilePath_UI  [MAXBUFSIZE] = "";	

	sprintf_s(szRecordPassFilePath_UI, MAXBUFSIZE, "D:\\PASS\\%s.txt", m_strMAC.Trim());
	fsPass.open (szRecordPassFilePath_UI   ,fstream::out|fstream::app);
	fsPass<<strRunInfo.GetBuffer(strRunInfo.GetLength())<<endl;
	fsPass.close();
	/*End*/

	KillTimer(1);
	CountTestResult(true);	
	UpdateData(false);
	m_strSN = "";
	m_strMAC = "";
	m_strPincode = "";
	m_strSSN = "";
	
	shutdown(m_Socket , 0);
	if(m_Socket!=INVALID_SOCKET)
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	shutdown(m_GoldenSocket , 0);	
	if(m_GoldenSocket!=INVALID_SOCKET)		
	{
		closesocket(m_GoldenSocket);
		m_GoldenSocket=INVALID_SOCKET;
	}

	return;
}

void CRunInfo::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	m_TestTimeCount+=1;
	CString strTimeCount = m_TestTimeCount.Format("%M:%S");	
	m_strTestTime = strTimeCount;
	UpdateData(FALSE);
	CDialog::OnTimer(nIDEvent);
}

int CRunInfo::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	return 0;
}


void CRunInfo::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
}

bool CRunInfo::ReadIniFileForGeneric()
{
	CString  strReadLine;
	char     *token = NULL;
	char     *nextToken = NULL;
	char     Seps[] = "= \t";
	char     szTitleFomat[MINBUFSIZE] = "";
	bool     bProductName = false;
	sprintf_s(szTitleFomat,MINBUFSIZE, "[%s]", "General");
	try
	{
		CStdioFile InitFile(m_IniFileInfo.szTestInfoFileName,CFile::modeRead);
		ShowMsg("********ReadIniFileForGeneric**********");
		while(InitFile.ReadString(strReadLine)&&strcmp(szTitleFomat, strReadLine))
			;
		while(InitFile.ReadString(strReadLine))
		{
			if(strReadLine.GetAt(0) == ';')
				continue;
			if(strReadLine.IsEmpty())
				break;
			_strupr_s(strReadLine.GetBuffer(strReadLine.GetLength()), strReadLine.GetLength()+1);  //add
			token = strtok_s(strReadLine.GetBuffer(strReadLine.GetLength()), Seps, &nextToken);
			while(token != NULL)
			{
				if(!_tcscmp(token,"TEST_CHANNEL_NUM_2G_1"))
				{
					bProductName = true;
					char  szChannelNum[MINBUFSIZE] = "";
					token = _tcstok_s(NULL,Seps,&nextToken);
					if(token!= NULL)
						strcpy_s(szChannelNum, MINBUFSIZE, token);
					m_IniFileInfo.nChannelNum_2G_1 = atoi(szChannelNum);
					if(m_IniFileInfo.nChannelNum_2G_1 < 0 ||  m_IniFileInfo.nChannelNum_2G_1 >3)
					{
						m_IniFileInfo.nChannelNum_2G_1 = 3;
					}
					break;
				}
				else if(!_tcscmp(token,"TEST_CHANNEL_NUM_5G_1"))
				{
					bProductName = true;
					char  szChannelNum[MINBUFSIZE] = "";
					token = _tcstok_s(NULL,Seps,&nextToken);
					if(token!= NULL)
						strcpy_s(szChannelNum, MINBUFSIZE, token);
					m_IniFileInfo.nChannelNum_5G_1 = atoi(szChannelNum);
					if(m_IniFileInfo.nChannelNum_5G_1 < 0 ||  m_IniFileInfo.nChannelNum_5G_1 >3)
					{
						m_IniFileInfo.nChannelNum_5G_1 = 3;
					}
					break;
				}
				else if(!_tcscmp(token,"TEST_CHANNEL_NUM_2G_2"))
				{
					bProductName = true;
					char  szChannelNum[MINBUFSIZE] = "";
					token = _tcstok_s(NULL,Seps,&nextToken);
					if(token!= NULL)
						strcpy_s(szChannelNum, MINBUFSIZE, token);
					m_IniFileInfo.nChannelNum_2G_2 = atoi(szChannelNum);
					if(m_IniFileInfo.nChannelNum_2G_2 < 0 ||  m_IniFileInfo.nChannelNum_2G_2 >3)
					{
						m_IniFileInfo.nChannelNum_2G_2 = 3;
					}
					break;
				}
				else if(!_tcscmp(token,"TEST_CHANNEL_NUM_5G_2"))
				{
					bProductName = true;
					char  szChannelNum[MINBUFSIZE] = "";
					token = _tcstok_s(NULL,Seps,&nextToken);
					if(token!= NULL)
						strcpy_s(szChannelNum, MINBUFSIZE, token);
					m_IniFileInfo.nChannelNum_5G_2 = atoi(szChannelNum);
					if(m_IniFileInfo.nChannelNum_5G_2 < 0 ||  m_IniFileInfo.nChannelNum_5G_2 >3)
					{
						m_IniFileInfo.nChannelNum_5G_2 = 3;
					}
					break;
				}
				else if(!_tcscmp(token,"TQMED"))
				{
					bProductName = true;
					token = _tcstok_s(NULL,Seps,&nextToken);
					if(token!= NULL)
						strcpy_s(m_IniFileInfo.szAllowBurn, MINBUFSIZE, token);
					break;

				}	
				else if(!_tcscmp(token,"SET_CHANNEL"))
				{
					bProductName = true;
					token = _tcstok_s(NULL,Seps,&nextToken);
					if(token!= NULL)
						strcpy_s(m_IniFileInfo.szSetChannel, MINBUFSIZE, token);
					break;
				}
				else if (!_tcscmp(token,"MAC1"))
				{
					bProductName  = true;
					token = _tcstok_s(NULL,Seps,&nextToken);
					if(token!= NULL)
					{
						strcpy_s(m_IniFileInfo.szMac1, MINBUFSIZE, token);
					}
					break;
				}
				else if (!_tcscmp(token,"MAC2"))
				{
					bProductName  = true;
					token = _tcstok_s(NULL,Seps,&nextToken);
					if(token!= NULL)
					{
						strcpy_s(m_IniFileInfo.szMac2, MINBUFSIZE, token);
					}
					break;
				}
				else if (!_tcscmp(token,"MAC3"))
				{
					bProductName  = true;
					token = _tcstok_s(NULL,Seps,&nextToken);
					if(token!= NULL)
					{
						strcpy_s(m_IniFileInfo.szMac3, MINBUFSIZE, token);
					}
					break;
				}
				else if (!_tcscmp(token,"TEST_STATION"))
				{
					bProductName  = true;
					token = _tcstok_s(NULL,Seps,&nextToken);
					if(token!= NULL)
					{
						strcpy_s(m_IniFileInfo.szTestStatuse, MINBUFSIZE, token);
						m_Station=m_IniFileInfo.szTestStatuse;
						UpdateData(false);
					}
					break;
				}
				else
					break;
			}
		}
		InitFile.Close();
	}
	catch (CFileException* e) 
	{
		if( e->m_cause == CFileException::fileNotFound )
		{
			return 0; 
		}
		if( e->m_cause == CFileException::accessDenied )
		{
			return 0; 
		}
	}
	if(!bProductName)
	{
		return 0;
	}
	ShowMsg("***********Read General config file end ******");
	return 1;
}

bool CRunInfo::ReadIniFile()
{
	char szDutName[128]="",szTemp[256]="";
	char szProPath[MINBUFSIZE]="";
	m_strProductName.Trim();
	ShowMsg("********ReadIniFile**********");
	lstrcpy(szDutName,m_strProductName.Trim().GetBuffer(m_strProductName.GetLength()));
	GetCurrentDirectory(sizeof(szProPath),szProPath);

	char szBuf[128] = "";
	sprintf_s(szBuf , sizeof(szBuf) , "\\%s" , m_IniFileInfo.szTestInfoFileName);

	lstrcat(szProPath,"\\");
	lstrcat(szProPath,m_IniFileInfo.szTestInfoFileName);
	GetPrivateProfileString(szDutName,"Product_Name","",szTemp,MINBUFSIZE,szProPath);
	if(strlen(szTemp)<=0)
	{
		return 0; // no such dut name found.
	}
	GetPrivateProfileString(szDutName,"FixIP_Address","",m_IniFileInfo.szStoreFixIPAddress, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"Firmware","",m_IniFileInfo.szStoreFirmware, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"BootCode","V1.0.14",m_IniFileInfo.szStoreBootCode, MINBUFSIZE,szProPath);
	
	GetPrivateProfileString(szDutName,"BoardID","",m_IniFileInfo.szStoreBoardID, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"CURRENT_TEST","0",m_IniFileInfo.szCurrentTest, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"Moduleinfor","",m_IniFileInfo.szModuleInfor, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"LOCK_CODE","1",m_IniFileInfo.szLockCode, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"ModuleVersion","",m_IniFileInfo.szModuleVersion, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"Record_File_Path","",m_IniFileInfo.szStoreSrvPath, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"Iperf_para","",m_IniFileInfo.szStoreIperfPara, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"Iperf_para(FAI)","",m_IniFileInfo.szStoreIperfParaFAI, MINBUFSIZE,szProPath);

	GetPrivateProfileString(szDutName,"Iperf_para_D","",m_IniFileInfo.szStoreIperfPara_D, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"Iperf_para_R","",m_IniFileInfo.szStoreIperfPara_R, MINBUFSIZE,szProPath);


	GetPrivateProfileString(szDutName,"REWORK","",m_IniFileInfo.szRework, MINBUFSIZE,szProPath);

	GetPrivateProfileString(szDutName,"TEST_RSSI","",m_IniFileInfo.szStoreTestRssi, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"DEFAULT_IP1","192.168.1.1",m_IniFileInfo.szDefaultIP1, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"DEFAULT_IP2","192.168.1.250",m_IniFileInfo.szDefaultIP2, MINBUFSIZE,szProPath);	

	GetPrivateProfileString(szDutName,"Test_ADSL","1",m_IniFileInfo.szStoreTestAdsl, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"CHECK_VERSION","1",m_IniFileInfo.szCheckVersion, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"UpLoad_VERSION","1",m_IniFileInfo.szUpLoadVersion, MINBUFSIZE,szProPath);//wheather upload FW
	
	GetPrivateProfileString(szDutName,"SSID_Lable","0",m_IniFileInfo.szIsHaveSSIDLable, MINBUFSIZE,szProPath);//wheather have SSID lable
	
	GetPrivateProfileString(szDutName,"Test_Rssi","1",m_IniFileInfo.szStoreTestRssi, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"POST_MYDAS_FLAG","1",m_IniFileInfo.szPostFlag, MINBUFSIZE,szProPath);//wheather post Mydas
	GetPrivateProfileString(szDutName,"Disable_Wireless","1",m_IniFileInfo.szDisableWireless, MINBUFSIZE,szProPath);//wheather Disable_Wireless
	GetPrivateProfileString(szDutName,"Region_Code","0",m_IniFileInfo.szRegionCode, MINBUFSIZE,szProPath);//burn region code
	GetPrivateProfileString(szDutName,"Region_Code_Flag","0",m_IniFileInfo.szRegionCodeFlag, MINBUFSIZE,szProPath);//burn region code flag

	m_IniFileInfo.nPingDUTCount=GetPrivateProfileInt(szDutName,"Ping_DUT_Count",2,szProPath);
	m_IniFileInfo.nPingGoldenCount=GetPrivateProfileInt(szDutName,"Ping_Golden_Count",5,szProPath);

	/*ThroughPut */
	m_IniFileInfo.nThroughputGoldenFlag=GetPrivateProfileInt(szDutName,"ThroughputGoldenFlag",0,szProPath);

	GetPrivateProfileString(szDutName,"Test_Throughput","1",m_IniFileInfo.szStoreTestThroughput, MINBUFSIZE,szProPath);
	GetPrivateProfileString(szDutName,"Test_Throughput_Single","0",m_IniFileInfo.szStoreTestThroughputSingle, MINBUFSIZE,szProPath);

	m_IniFileInfo.nRetryNum=GetPrivateProfileInt(szDutName,"Retry_Num",8,szProPath);

	m_IniFileInfo.nTXThroughputSpec=GetPrivateProfileInt(szDutName,"2G_TX_Throughput_Sep",110,szProPath);
	m_IniFileInfo.nRXThroughputSpec=GetPrivateProfileInt(szDutName,"2G_RX_Throughput_Sep",110,szProPath);
	m_IniFileInfo.nTXRXThroughputSpec=GetPrivateProfileInt(szDutName,"2G_TXRX_Throughput_Sep",110,szProPath);

	m_IniFileInfo.nTXThroughputSpec5G=GetPrivateProfileInt(szDutName,"5G_TX_Throughput_Sep",130,szProPath);
	m_IniFileInfo.nRXThroughputSpec5G=GetPrivateProfileInt(szDutName,"5G_RX_Throughput_Sep",130,szProPath);
	m_IniFileInfo.nTXRXThroughputSpec5G=GetPrivateProfileInt(szDutName,"5G_TXRX_Throughput_Sep",150,szProPath);

	m_IniFileInfo.nTXThroughputSpecHigt=GetPrivateProfileInt(szDutName,"2G_TX_Throughput_Sep_Higt",300,szProPath);
	m_IniFileInfo.nRXThroughputSpecHigt=GetPrivateProfileInt(szDutName,"2G_RX_Throughput_Sep_Higt",300,szProPath);
	m_IniFileInfo.nTXRXThroughputSpecHigt=GetPrivateProfileInt(szDutName,"2G_TXRX_Throughput_Sep_Higt",300,szProPath);

	m_IniFileInfo.nTXThroughputSpecHigt5G=GetPrivateProfileInt(szDutName,"5G_TX_Throughput_Sep_Higt",450,szProPath);
	m_IniFileInfo.nRXThroughputSpecHigt5G=GetPrivateProfileInt(szDutName,"5G_RX_Throughput_Sep_Higt",450,szProPath);
	m_IniFileInfo.nTXRXThroughputSpecHigt5G=GetPrivateProfileInt(szDutName,"5G_TXRX_Throughput_Sep_Higt",450,szProPath);
	
	m_IniFileInfo.nTXThroughputSpec_2=GetPrivateProfileInt(szDutName,"2G_TX_Throughput_Sep_2",110,szProPath);
	m_IniFileInfo.nRXThroughputSpec_2=GetPrivateProfileInt(szDutName,"2G_RX_Throughput_Sep_2",110,szProPath);
	m_IniFileInfo.nTXRXThroughputSpec_2=GetPrivateProfileInt(szDutName,"2G_TXRX_Throughput_Sep_2",110,szProPath);

	m_IniFileInfo.nTXThroughputSpec5G_2=GetPrivateProfileInt(szDutName,"5G_TX_Throughput_Sep_2",130,szProPath);
	m_IniFileInfo.nRXThroughputSpec5G_2=GetPrivateProfileInt(szDutName,"5G_RX_Throughput_Sep_2",130,szProPath);
	m_IniFileInfo.nTXRXThroughputSpec5G_2=GetPrivateProfileInt(szDutName,"5G_TXRX_Throughput_Sep_2",150,szProPath);

	m_IniFileInfo.nTXThroughputSpecHigt_2=GetPrivateProfileInt(szDutName,"2G_TX_Throughput_Sep_Higt_2",300,szProPath);
	m_IniFileInfo.nRXThroughputSpecHigt_2=GetPrivateProfileInt(szDutName,"2G_RX_Throughput_Sep_Higt_2",300,szProPath);
	m_IniFileInfo.nTXRXThroughputSpecHigt_2=GetPrivateProfileInt(szDutName,"2G_TXRX_Throughput_Sep_Higt_2",300,szProPath);

	m_IniFileInfo.nTXThroughputSpecHigt5G_2=GetPrivateProfileInt(szDutName,"5G_TX_Throughput_Sep_Higt_2",450,szProPath);
	m_IniFileInfo.nRXThroughputSpecHigt5G_2=GetPrivateProfileInt(szDutName,"5G_RX_Throughput_Sep_Higt_2",450,szProPath);
	m_IniFileInfo.nTXRXThroughputSpecHigt5G_2=GetPrivateProfileInt(szDutName,"5G_TXRX_Throughput_Sep_Higt_2",450,szProPath);

	GetPrivateProfileString("General","2G_Throtghput_Golden_SSID","0",m_IniFileInfo.sz2GThrotghputGoldenSSID, MINBUFSIZE,".\\Throughput_Setting.ini");//2G Throtghput Golden SSID
	GetPrivateProfileString("General","5G_Throtghput_Golden_SSID","0",m_IniFileInfo.sz5GThrotghputGoldenSSID, MINBUFSIZE,".\\Throughput_Setting.ini");//5G Throtghput Golden SSID
	
	GetPrivateProfileString("General","2G_Throtghput_Golden_IP","0",m_IniFileInfo.sz2GThrotghputGoldenIP, MINBUFSIZE,".\\Throughput_Setting.ini");//2G Throtghput Golden IP
	GetPrivateProfileString("General","5G_Throtghput_Golden_IP","0",m_IniFileInfo.sz5GThrotghputGoldenIP, MINBUFSIZE,".\\Throughput_Setting.ini");//2G Throtghput Golden IP

	m_IniFileInfo.nTXThroughputSpec_Factroy=GetPrivateProfileInt(szDutName,"2_T_TP_S_F",220,szProPath);
	m_IniFileInfo.nRXThroughputSpec_Factroy=GetPrivateProfileInt(szDutName,"2_R_TP_S_F",220,szProPath);
	m_IniFileInfo.nTXRXThroughputSpec_Factroy=GetPrivateProfileInt(szDutName,"2_TR_TP_S_F",0,szProPath);

	m_IniFileInfo.nTXThroughputSpec5G_Factroy=GetPrivateProfileInt(szDutName,"5_T_TP_S_F",320,szProPath);
	m_IniFileInfo.nRXThroughputSpec5G_Factroy=GetPrivateProfileInt(szDutName,"5_R_TP_S_F",320,szProPath);
	m_IniFileInfo.nTXRXThroughputSpec5G_Factroy=GetPrivateProfileInt(szDutName,"5_TR_TP_S_F",0,szProPath);

	m_IniFileInfo.nThroughputFalg_Factroy=GetPrivateProfileInt(szDutName,"2_TP_F_F",0,szProPath);
	m_IniFileInfo.nThroughputFalg5G_Factroy=GetPrivateProfileInt(szDutName,"5_TP_F_F",0,szProPath);

	m_IniFileInfo.nThroughputRange_Factroy=GetPrivateProfileInt(szDutName,"2_TP_R_F",40,szProPath);
	m_IniFileInfo.nThroughputRange5G_Factroy=GetPrivateProfileInt(szDutName,"5_TP_R_F",70,szProPath); 

	/*End*/

	m_IniFileInfo.MURssiSpec=GetPrivateProfileInt(szDutName,"EXTERION_U",0,szProPath);
	m_IniFileInfo.MDRssiSpec=GetPrivateProfileInt(szDutName,"EXTERION_D",0,szProPath);
	m_IniFileInfo.NURssiSpec=GetPrivateProfileInt(szDutName,"NO_EXTERION_U",0,szProPath);
	m_IniFileInfo.NDRssiSpec=GetPrivateProfileInt(szDutName,"NO_EXTERION_D",0,szProPath);
	m_IniFileInfo.DeltaUPSpec=GetPrivateProfileInt(szDutName,"DELTA_UP",0,szProPath);
	m_IniFileInfo.DeltaDownSpec=GetPrivateProfileInt(szDutName,"DELTA_DOWN",0,szProPath);
	m_IniFileInfo.nDSLUpStream=GetPrivateProfileInt(szDutName,"ADSL_Upstream",1000000,szProPath);
	m_IniFileInfo.nDSLDownStream=GetPrivateProfileInt(szDutName,"ADSL_Downstream",20500000,szProPath);
	m_IniFileInfo.nTimeOut=GetPrivateProfileInt(szDutName,"TIMEOUT",15000,szProPath);
	m_IniFileInfo.nSleepOrNo=GetPrivateProfileInt(szDutName,"SLEEP",0,szProPath);

	/*USB*/	
	m_IniFileInfo.nTestUsb=GetPrivateProfileInt(szDutName,"USB_TEST_FLAG",0,szProPath);	//USB dect flag
	GetPrivateProfileString(szDutName,"USB_DECT_STRING","",m_IniFileInfo.szUsbDectString, MINBUFSIZE,szProPath);//USB dect string, if have:dect pass	
	GetPrivateProfileString(szDutName,"USB_DECT_CMD","",m_IniFileInfo.szUsbDectCmd, MINBUFSIZE,szProPath);//USB dect command	
	//USB 1
	GetPrivateProfileString(szDutName,"USB_RESULT_FILE_1","",m_IniFileInfo.szUsbResultFile_1, MINBUFSIZE,szProPath);//USB throughput test result file name	
	m_IniFileInfo.nUsbTXThroughputFlag_1=GetPrivateProfileInt(szDutName,"USB_TX_FLAG_1",0,szProPath);//USB TX throughput run flag
	m_IniFileInfo.nUsbRXThroughputFlag_1=GetPrivateProfileInt(szDutName,"USB_RX_FLAG_1",0,szProPath);//USB RX throughput run flag
	GetPrivateProfileString(szDutName,"USB_TX_RUN_FILE_1","",m_IniFileInfo.szUsbTXThroughputRunFie_1, MINBUFSIZE,szProPath);//USB TX throughput run file
	GetPrivateProfileString(szDutName,"USB_RX_RUN_FILE_1","",m_IniFileInfo.szUsbRXThroughputRunFie_1, MINBUFSIZE,szProPath);//USB RX throughput run file
	m_IniFileInfo.nUsbTXThroughputSpec_1=GetPrivateProfileInt(szDutName,"USB_TX_SPEC_1",0,szProPath);//USB TX throughput test SPEC
	m_IniFileInfo.nUsbRXThroughputSpec_1=GetPrivateProfileInt(szDutName,"USB_RX_SPEC_1",0,szProPath);//USB RX throughput test SPEC	
	GetPrivateProfileString(szDutName,"USB_FILE_1","",m_IniFileInfo.szUsbFilename_1, MINBUFSIZE,szProPath);//USB throughput test need file(usually is named:data.bin)

	//USB 2
	GetPrivateProfileString(szDutName,"USB_RESULT_FILE_2","",m_IniFileInfo.szUsbResultFile_2, MINBUFSIZE,szProPath);//USB throughput test result file name	
	m_IniFileInfo.nUsbTXThroughputFlag_2=GetPrivateProfileInt(szDutName,"USB_TX_FLAG_2",0,szProPath);//USB TX throughput run flag
	m_IniFileInfo.nUsbRXThroughputFlag_2=GetPrivateProfileInt(szDutName,"USB_RX_FLAG_2",0,szProPath);//USB RX throughput run flag
	GetPrivateProfileString(szDutName,"USB_TX_RUN_FILE_2","",m_IniFileInfo.szUsbTXThroughputRunFie_2, MINBUFSIZE,szProPath);//USB TX throughput run file
	GetPrivateProfileString(szDutName,"USB_RX_RUN_FILE_2","",m_IniFileInfo.szUsbRXThroughputRunFie_2, MINBUFSIZE,szProPath);//USB RX throughput run file
	m_IniFileInfo.nUsbTXThroughputSpec_2=GetPrivateProfileInt(szDutName,"USB_TX_SPEC_2",0,szProPath);//USB TX throughput test SPEC
	m_IniFileInfo.nUsbRXThroughputSpec_2=GetPrivateProfileInt(szDutName,"USB_RX_SPEC_21",0,szProPath);//USB RX throughput test SPEC
	GetPrivateProfileString(szDutName,"USB_FILE_2","",m_IniFileInfo.szUsbFilename_2, MINBUFSIZE,szProPath);//USB throughput test need file(usually is named:data.bin)

	/*Lan Port*/
	m_IniFileInfo.nLanPort_Throughput_Flag=GetPrivateProfileInt(szDutName,"LAN_LAN_FLAG",0,szProPath);
	m_IniFileInfo.nLanPort_Throughout_SPEC_LOW=GetPrivateProfileInt(szDutName,"LAN_LAN_SPEC_LOW",0,szProPath);
	m_IniFileInfo.nLanPort_Throughout_SPEC_HIGH=GetPrivateProfileInt(szDutName,"LAN_LAN_SPEC_HIGH",0,szProPath);
	GetPrivateProfileString(szDutName,"LAN_LAN_IPERF","",m_IniFileInfo.szLanPort_Throughput_Iperf, MINBUFSIZE,szProPath);
	/*End*/

	/*Wan Port*/
	m_IniFileInfo.nWanPort_Flag=GetPrivateProfileInt(szDutName,"WAN_FLAG",0,szProPath);
	m_IniFileInfo.nWanPort_Throughput_Flag=GetPrivateProfileInt(szDutName,"LAN_WAN_FLAG",0,szProPath);	
	m_IniFileInfo.nWanPort_Throughout_SPEC_LOW=GetPrivateProfileInt(szDutName,"LAN_WAN_SPEC_LOW",0,szProPath);
	m_IniFileInfo.nWanPort_Throughout_SPEC_HIGH=GetPrivateProfileInt(szDutName,"LAN_WAN_SPEC_HIGH",0,szProPath);
	GetPrivateProfileString(szDutName,"LAN_WAN_IPERF","",m_IniFileInfo.szWanPort_Throughput_Iperf, MINBUFSIZE,szProPath);

	m_IniFileInfo.nWanPort_Throughput_Flag_Factroy=GetPrivateProfileInt(szDutName,"L_W_T_F",0,szProPath);
	m_IniFileInfo.nWanPort_Throughout_SPEC_LOW_Factroy=GetPrivateProfileInt(szDutName,"L_W_T_S_F",0,szProPath);
	m_IniFileInfo.nWanPort_Throughout_RANGE_Factroy=GetPrivateProfileInt(szDutName,"L_W_T_R_F",100,szProPath);

	/*End*/

	/*check String table*/
	m_IniFileInfo.nCheckSumFlag=GetPrivateProfileInt(szDutName,"CheckSum_Flag",0,szProPath);
	m_IniFileInfo.nCheckSumNum=GetPrivateProfileInt(szDutName,"CheckSum_Num",6,szProPath);

	int iFlag = m_IniFileInfo.nCheckSumNum;
	memset(m_IniFileInfo.szCkecksum[m_IniFileInfo.nCheckSumNum-iFlag],0,MINBUFSIZE);
	for(iFlag ; iFlag > 0 ; iFlag--)
	{	
		memset(m_IniFileInfo.szCkecksum[m_IniFileInfo.nCheckSumNum-iFlag+1],0,MINBUFSIZE);
		char StoreChksum[MINBUFSIZE]="Checksum";
		char str[2]="";
		sprintf_s(str,2,"%d",m_IniFileInfo.nCheckSumNum-iFlag+1);
		strcat_s(StoreChksum,sizeof(StoreChksum),str);		
		//Get string table checksum
		GetPrivateProfileString(szDutName,StoreChksum,"",m_IniFileInfo.szCkecksum[m_IniFileInfo.nCheckSumNum-iFlag+1], MINBUFSIZE,szProPath);
	}
	/*End*/

	/*disable wifi channel*/
	switch(m_IniFileInfo.nChannelNum_2G_1)
	{
	case 1:
		((CStatic*)GetDlgItem(IDC_STATIC_2G_CHANNEL2))->EnableWindow(false);
		((CStatic*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->EnableWindow(false);
		break;
	case 2:
		break;
	case 3:
		((CStatic*)GetDlgItem(IDC_STATIC_2G_CHANNEL1))->EnableWindow(false);
		((CStatic*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->EnableWindow(false);
		((CStatic*)GetDlgItem(IDC_STATIC_2G_CHANNEL2))->EnableWindow(false);
		((CStatic*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->EnableWindow(false);
		break;
	default:
		break;
	}
	switch(m_IniFileInfo.nChannelNum_5G_1)
	{
	case 1:
		((CStatic*)GetDlgItem(IDC_STATIC_5G_CHANNEL2))->EnableWindow(false);
		((CStatic*)GetDlgItem(IDC_EDIT_5G_CHANNEL2))->EnableWindow(false);
		break;
	case 2:
		break;
	case 3:
		((CStatic*)GetDlgItem(IDC_STATIC_5G_CHANNEL1))->EnableWindow(false);
		((CStatic*)GetDlgItem(IDC_EDIT_5G_CHANNEL1))->EnableWindow(false);
		((CStatic*)GetDlgItem(IDC_STATIC_5G_CHANNEL2))->EnableWindow(false);
		((CStatic*)GetDlgItem(IDC_EDIT_5G_CHANNEL2))->EnableWindow(false);
		break;
	default:
		break;
	}
	/*End*/


	ShowMsg("***********Read config file end ******");
	return 1;
}


int CRunInfo::RunSpecifyExeAndRead(CString& strArpOutInfo,char* RunFileName,bool bIsShow)
{
	strArpOutInfo.Empty();
	HANDLE hReadPipe,hWritePipe;
	SECURITY_ATTRIBUTES  saAttr; 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = true;
	saAttr.lpSecurityDescriptor = NULL;

	if(!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
	{
		DisplayRunTimeInfo("Create pipe fail");
		return 0;
	}

	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo;

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );

	GetStartupInfo(&siStartInfo);
	siStartInfo.cb = sizeof(STARTUPINFO); 
	siStartInfo.hStdError = hWritePipe;
	siStartInfo.hStdOutput = hWritePipe;
	siStartInfo.wShowWindow = SW_HIDE;
	siStartInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;


	if(!CreateProcess(NULL, RunFileName, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo))
	{
		char error[MINBUFSIZE] = "";
		sprintf_s(error, MAXBUFSIZE, "Run %s application fail", RunFileName);
		DisplayRunTimeInfo(error);
		return 0;
	}


	CloseHandle(hWritePipe);
	CloseHandle(piProcInfo.hThread);
	CloseHandle(piProcInfo.hProcess);


	char buffer[5000] = "";
	DWORD byteRead = 0;

	int iRtyNum = 100;

	while(iRtyNum--)
	{
		memset(buffer, 0, 5000);
		if( !ReadFile( hReadPipe, buffer, 4999, &byteRead, 
			NULL) || (byteRead == 0)) break;
		strArpOutInfo+=buffer;
		if(bIsShow)
		{
			DisplayRunTimeInfo(buffer);	
		}
		Sleep(200);
	}

	CloseHandle(hReadPipe);
	if(iRtyNum <= 0)
	{
		return 3;
	}
	else
	{
		return 1;
	}
}


void CRunInfo::GetMac(IN char* source,OUT char* target)
{
	char  *next_token;
	char  sep[] = _T(" \n\t");
	char  Temp[20];
	char  szMACBuf[100];
	char* tok = _tcstok_s(source,sep,&next_token);
	while(NULL!=tok)
	{
		if(!_tcscmp(tok,m_IniFileInfo.szDefaultIP1))
		{
			tok = _tcstok_s(NULL,sep,&next_token);
			_tcscpy_s(Temp,20,tok);
			int i ,j;
			for(i = 0,j = 0;i < 17;i++)
			{
				if(Temp[i]!='-')
				{
					target[j++]=Temp[i];
				}
			}
			target[j]='\0';
			break;
		}
		tok = _tcstok_s(NULL,sep,&next_token);
	}
	_strupr_s(target,MINBUFSIZE);

	sprintf_s(szMACBuf, MINBUFSIZE, "MAC: %s", target);
	Sleep(1000);
	DisplayRunTimeInfo(szMACBuf);
	DisplayRunTimeInfo("");
}

void CRunInfo::CountTestResult(bool IsPass)
{
	static DWORD dwPass = 0;
	static DWORD dwFail = 0;
	if(IsPass)
	{
		dwPass++;
		TCHAR szTempPass[10] = _T("");
		_stprintf_s(szTempPass, 10, _T("%d"), dwPass);
		SetDlgItemText(IDC_PASS_COUNT, szTempPass);
	}
	else
	{
		dwFail++;
		TCHAR szTempFail[10] = _T("");
		_stprintf_s(szTempFail, 10, _T("%d"), dwFail);
		SetDlgItemText(IDC_FAIL_COUNT, szTempFail);
	}

	static double YRate = 0;
	YRate = (dwPass * 100.0) / (dwPass + dwFail);
	TCHAR szTempYRate[10] = _T("");
	_stprintf_s(szTempYRate, 10, _T("%.2f%s"), YRate, "%");
	SetDlgItemText(IDC_EDIT_YRATE, szTempYRate);
}

void CRunInfo::DisplayRunTimeInfo(char* pRunInfo)
{
	strRunInfo += pRunInfo;
	strRunInfo += "\r\n";
	if(pRunInfo == NULL)
	{
		strRunInfo = "";
	}
	SetDlgItemText(IDC_EDIT_RUNINFO,strRunInfo.GetBuffer(strRunInfo.GetLength()));
	int a = ((CEdit*)GetDlgItem(IDC_EDIT_RUNINFO))->GetLineCount();
	((CEdit*)GetDlgItem(IDC_EDIT_RUNINFO))->LineScroll(a,0);
}
void CRunInfo::DisplayRunTimeInfo(char* pRunInfo, double iData)
{
	char szBuf[1024];
	memset(szBuf, 0 , 1024);
	sprintf_s(szBuf, sizeof(szBuf), "%s:%.2f\r\n", pRunInfo, iData);
	strRunInfo += szBuf;
	SetDlgItemText(IDC_EDIT_RUNINFO,strRunInfo.GetBuffer(strRunInfo.GetLength()));
	int a = ((CEdit*)GetDlgItem(IDC_EDIT_RUNINFO))->GetLineCount();
	((CEdit*)GetDlgItem(IDC_EDIT_RUNINFO))->LineScroll(a,0);
}

void CRunInfo::DisplayRunTimeInfo(char* pRunInfo, char* pData)
{
	char szBuf[1024];
	memset(szBuf, 0 , 1024);
	sprintf_s(szBuf, sizeof(szBuf), "%s:%s\r\n", pRunInfo, pData);
	strRunInfo += szBuf;
	SetDlgItemText(IDC_EDIT_RUNINFO,strRunInfo.GetBuffer(strRunInfo.GetLength()));
	int a = ((CEdit*)GetDlgItem(IDC_EDIT_RUNINFO))->GetLineCount();
	((CEdit*)GetDlgItem(IDC_EDIT_RUNINFO))->LineScroll(a,0);
}

void CRunInfo::IsDisplayErrorCode(bool Flag)
{
	if(Flag)
	{
		((CStatic*)GetDlgItem(IDC_ERRORCODE))->ShowWindow(SW_SHOW);
	}
	else
	{
		((CStatic*)GetDlgItem(IDC_ERRORCODE))->ShowWindow(SW_HIDE);
	}
}


void CRunInfo::SendCmdToGolden(char* pCmd)
{
	CStringA strSendData;
	DWORD    dwNumber;	
	WSABUF   sendbuf;
	sendbuf.buf=new char[300];
	sendbuf.len=300;
	memset(sendbuf.buf,0,300);
	
	strSendData  =pCmd;
	strSendData +="\n";
	
	strcpy_s(sendbuf.buf,300,strSendData.GetBuffer(strSendData.GetLength()));
	
	WSASend(m_GoldenSocket,&sendbuf,1,&dwNumber,0,NULL,NULL);
	
	delete []sendbuf.buf;
}

bool  CRunInfo::SendGoldenCommand(char *pCmd, char *pRtn, int iDelay)
{ 
	h_golenEvt = CreateEvent(NULL,true,false,NULL);
	ResetEvent(h_golenEvt);
	m_strStoreGoldenData ="";

	GoldenInfo.szReturnInfo = pRtn;
	GoldenInfo.nDelay = iDelay;
	GoldenInfo.GoldenSocket = m_GoldenSocket;
	GoldenInfo.szGoldenData = m_strStoreGoldenData;
	GoldenInfo.bResponeOK = false;


	if(GoldenInfo.bConnectOK == true)
	{	
		/*send*/
		//os<<"enter\n";os.flush();
		CStringA strSendData;
		DWORD    dwNumber;	
		WSABUF   sendbuf;
		sendbuf.buf=new char[1024];
		sendbuf.len=1024;
		memset(sendbuf.buf,0,sendbuf.len);

		strcpy_s(sendbuf.buf,sendbuf.len,pCmd);	
		try
		{
			WSASend(m_GoldenSocket,&sendbuf,1,&dwNumber,0,NULL,NULL);
			//os<<"send out\n";os.flush();
		}
		catch(...)
		{
			pRunInfo->DisplayRunTimeInfo("WSASend error!");
		}

		delete []sendbuf.buf;


		ResetEvent(h_golenEvt);

		//OnRecvGoldenData = sendcommand + recv + commpare
		AfxBeginThread(OnRecvGoldenData, &GoldenInfo);
		if(WAIT_OBJECT_0 == WaitForSingleObject(h_golenEvt,(iDelay-1)*1000))
		{
			if(GoldenInfo.bResponeOK == true)
			{
				return true;
			}
			else
			{
				//do nothing
			}			
		}
	}
	return false;
}

bool CRunInfo::InitGoldenSocket()
{
	GoldenInfo.bConnectOK = false;
	m_GoldenSocket = WSASocket(AF_INET,SOCK_DGRAM, IPPROTO_UDP, NULL,0, 0);
	if(INVALID_SOCKET==m_GoldenSocket)
	{
		DisplayRunTimeInfo("Create golden socket fail");
		return false;
	}	
	DWORD dwIP;
	((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_GOLDENIP2))->GetAddress(dwIP);
	SOCKADDR_IN addrSock;
	addrSock.sin_family=AF_INET;
	addrSock.sin_port=htons(2000);
	addrSock.sin_addr.S_un.S_addr=htonl(dwIP);

	int revt = connect(m_GoldenSocket,(sockaddr*)&addrSock,sizeof(sockaddr));
	
	if(SOCKET_ERROR==revt)
	{
		DisplayRunTimeInfo("Bind sock fail\n");
		return FALSE;
	}

	GoldenInfo.bConnectOK = true;
	return TRUE;

}

bool CRunInfo::InitSocket(char* DefaultIP)
{
	sockaddr_in  SocketAddr;
	m_Socket = WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,0);
	int iReturn = WSAGetLastError();
	if(INVALID_SOCKET ==m_Socket)
	{
		DisplayRunTimeInfo("Create socket fail\n");
		return FALSE;
	}
	
	SocketAddr.sin_family = AF_INET;
	SocketAddr.sin_addr.S_un.S_addr = inet_addr(DefaultIP);
	SocketAddr.sin_port = htons(23);
	
	int revt = connect(m_Socket,(sockaddr*)&SocketAddr,sizeof(sockaddr));
	
	if(SOCKET_ERROR==revt)
	{
		DisplayRunTimeInfo("Bind sock fail\n");
		return FALSE;
	}
	
	if(SOCKET_ERROR ==WSAAsyncSelect(m_Socket,m_hWnd,UM_SOCK,FD_READ))
	{
		DisplayRunTimeInfo("Create network event fail\n ");
		return FALSE;
	}
	return TRUE;
}

LRESULT  CRunInfo::OnRecvComData(WPARAM wParam,LPARAM lParam)
{
	char strBuf[MINBUFSIZE] = "";
	strcpy_s(strBuf, MINBUFSIZE, (char*)lParam);
	FillMemory((char*)lParam, MINBUFSIZE, 0);
	int len = (int)strlen(strBuf);

	DisplayRunTimeInfo(strBuf);
	
	if(len == 25+25+4+2) //SN+ModelOrMac+PASS
	{
		if(strcmp(m_strTestResult , "RUN") == 0)
		{
			return 0;
		}
		char szSN[MINBUFSIZE] = "";
		char szStatus[MINBUFSIZE] = "";
		char szProductName[MINBUFSIZE] = "";
		strncpy_s(szSN, MINBUFSIZE, strBuf, 25);
		strncpy_s(szProductName, MINBUFSIZE, strBuf+25, 25);
		strncpy_s(szStatus, MINBUFSIZE, strBuf+50, 4);
			
		m_strSN = szSN;
		m_strProductName.Trim() = szProductName;
		m_strMAC = "";
		m_strPincode = "";
		m_strSSN = "";
		UpdateData(false);
		static bool bReadIniOnlyOnce = true;
		UpdateData(false);
		if(bReadIniOnlyOnce)
		{
			bReadIniOnlyOnce = false;
			m_strIniProductName = m_strProductName;
			if(!ReadIniFile())
			{
				AfxMessageBox("請重新配置INI檔,確保有匹配的機種名稱,程式自動關閉!\n");
				::PostMessage(GetParent()->GetParent()->GetSafeHwnd(),WM_CLOSE,0,0);
				return 0;
			}

		}
		if(strncmp(szStatus, "PASS", 4))
			return 0;
	}	
	//else if(len == 25+12+12+4+2) //SN+MAC+PinCode+PASS
	//{
	//	if(!strcmp(m_strTestResult , "RUN"))
	//		return 0;
	//	char szSN[MINBUFSIZE] = "";		
	//	char szMac[MINBUFSIZE] = "";
	//	char szPinCode[MINBUFSIZE] = "";
	//	char szStatus[MINBUFSIZE] = "";
	//	strncpy_s(szSN, MINBUFSIZE, strBuf, 25);
	//	strncpy_s(szMac, MINBUFSIZE, strBuf+25, 12);
	//	strncpy_s(szPinCode, MINBUFSIZE, strBuf+37, 12);
	//	strncpy_s(szStatus, MINBUFSIZE, strBuf+49, 4);
	//	m_strPincode = szPinCode;
	//	m_strMAC = szMac;
	//	m_strSN = szSN;
	//	UpdateData(false);
	//	if(strncmp(szStatus, "PASS", 4))
	//		return 0;
	//}
	else if(len == 25+12+12+15+4+2) //SN+MAC+PinCode+SSN+PASS
	{
		if(strcmp(m_strTestResult , "RUN") == 0)
		{
			ShowMsg(m_strTestResult);
			return 0;
		}
		char szSN[MINBUFSIZE] = "";		
		char szMac[MINBUFSIZE] = "";
		char szPinCode[MINBUFSIZE] = "";
		char szCN[MINBUFSIZE] = "";
		char szStatus[MINBUFSIZE] = "";
		strncpy_s(szSN, MINBUFSIZE, strBuf, 25);
		strncpy_s(szMac, MINBUFSIZE, strBuf+25, 12);
		strncpy_s(szPinCode, MINBUFSIZE, strBuf+37, 12);
		strncpy_s(szCN, MINBUFSIZE, strBuf+49, 15);
		strncpy_s(szStatus, MINBUFSIZE, strBuf+64, 4);
		m_strSSN = szCN;
		m_strPincode = szPinCode;
		m_strMAC = szMac;
		m_strSN = szSN;
		ShowMsg(m_strSN);
		ShowMsg(m_strMAC);
		ShowMsg(m_strPincode);
		ShowMsg(m_strSSN);
		UpdateData(false);
		if(strncmp(szStatus, "PASS", 4))
			return 0;
		m_strTestResult = "RUN";
		UpdateData(false);
		OnBnClickedBtnStart();
	}
	else if(len == 25+12+12+15+12+4+2) //SN+MAC+PinCode+SSN+PC_name+PASS
	{
		if(strstr(strBuf, "PASS") == NULL)
		{
			TestFail(SR00);
		}
	}
	//else if(len == 25+12+12+15+12+4+4+2) //SN+MAC+PinCode+SSN+PC_name+ErrCode+PASS
	//{
	//	if(strstr(strBuf, "PASS") == NULL)
	//		TestFail(SR00);
	//}
	else if(len == 25+12+12+15+30+12+4+2) //SN+MAC+PinCode+SSN+FW+ATE_NO+PASS
	{
		if(strstr(strBuf, "PASS") == NULL)
		{
			TestFail(SR00);
		}
		else
		{
			TestPass();
		}
	}
	//else if(len == 25+12+12+15+30+12+4+4+2)////SN+MAC+PinCode+SSN+FW+ATE_NO+ErrCode+PASS
	//{
	//	if(strstr(strBuf, "PASS") == NULL)
	//		TestFail(SR00);
	//}	
	else if(len == 25+12+12+15+32+32+32+32+4+2) //SN+MAC+PinCode+SSN+SSID+PASSWORD+SSID+PASSWORD+PASS
	{
		if(strcmp(m_strTestResult , "RUN") == 0)
		{
			ShowMsg(m_strTestResult);
			return 0;
		}
		char szSN[MINBUFSIZE] = "";		
		char szMac[MINBUFSIZE] = "";
		char szPinCode[MINBUFSIZE] = "";
		char szCN[MINBUFSIZE] = "";
		
		char szSSID_2G[MINBUFSIZE] = "";		
		char szPASS_2G[MINBUFSIZE] = "";
		char szSSID_5G[MINBUFSIZE] = "";
		char szPASS_5G[MINBUFSIZE] = "";

		char szStatus[MINBUFSIZE] = "";
		strncpy_s(szSN, MINBUFSIZE, strBuf, 25);
		strncpy_s(szMac, MINBUFSIZE, strBuf+25, 12);
		strncpy_s(szPinCode, MINBUFSIZE, strBuf+37, 12);
		strncpy_s(szCN, MINBUFSIZE, strBuf+49, 15);

		strncpy_s(szSSID_2G, MINBUFSIZE, strBuf+64, 32);
		strncpy_s(szPASS_2G, MINBUFSIZE, strBuf+96, 32);
		strncpy_s(szSSID_5G, MINBUFSIZE, strBuf+128, 32);
		strncpy_s(szPASS_5G, MINBUFSIZE, strBuf+160, 32);


		strncpy_s(szStatus, MINBUFSIZE, strBuf+192, 4);

		m_strSSN = szCN;
		m_strPincode = szPinCode;
		m_strMAC = szMac;
		m_strSN = szSN;
		m_strSSID_2G = szSSID_2G;
		m_strPASS_2G = szPASS_2G;
		m_strSSID_5G = szSSID_5G;
		m_strPASS_5G = szPASS_5G ;

		ShowMsg(m_strSN);
		ShowMsg(m_strMAC);
		ShowMsg(m_strPincode);
		ShowMsg(m_strSSN);

		ShowMsg(m_strSSID_2G);
		ShowMsg(m_strPASS_2G);
		ShowMsg(m_strSSID_5G);
		ShowMsg(m_strPASS_5G);

		UpdateData(false);
		if(strncmp(szStatus, "PASS", 4))
			return 0;
		m_strTestResult = "RUN";
		UpdateData(false);
		OnBnClickedBtnStart();
	}
	else if(len == 25+12+12+15+32+32+32+32+12+4+2) //SN+MAC+PinCode+SSN+SSID+PASSWORD+SSID+PASSWORD+PC_name+PASS
	{
		if(strstr(strBuf, "PASS") == NULL)
		{
			TestFail(SR00);
		}
		else
		{
			TestPass();
		}
	}
	//else if(len == 25+12+12+15+12+32+32+32+32+4+4+2) //SN+MAC+PinCode+SSN+SSID+PASSWORD+SSID+PASSWORD+PC_name+ErrCode+PASS
	//{
	//	if(strstr(strBuf, "PASS") == NULL)
	//		TestFail(SR00);
	//}
	else if(len == 25+12+12+15+32+32+32+32+30+12+4+2) //SN+MAC+PinCode+SSN+SSID+PASSWORD+SSID+PASSWORD+FW+ATE_NO+PASS
	{
		if(strstr(strBuf, "PASS") == NULL)			
		{
			TestFail(SR00);
		}
		else
		{
			TestPass();
		}
	}
	//else if(len == 25+12+12+15+32+32+32+32+30+12+4+4+2)////SN+MAC+PinCode+SSN+SSID+PASSWORD+SSID+PASSWORD+FW+ATE_NO+ErrCode+PASS
	//{
	//	if(strstr(strBuf, "PASS") == NULL)
	//		TestFail(SR00);
	//}
	//12 start
	else if(len == 25+12+4+2)//SN+productname+PASS
	{
		if(strcmp(m_strTestResult , "RUN") == 0)
			return 0;
		char szSN[MINBUFSIZE] = "";
		char szStatus[MINBUFSIZE] = "";
		char szProductName[MINBUFSIZE] = "";
		strncpy_s(szSN, MINBUFSIZE, strBuf, 25);
		strncpy_s(szProductName, MINBUFSIZE, strBuf+25, 12);
		strncpy_s(szStatus, MINBUFSIZE, strBuf+37, 4);
		m_strSN = szSN;
		UpdateData(false);
		m_strProductName = szProductName;	
		UpdateData(false);

		static bool bReadIniOnlyOnce = true;
		UpdateData(false);
		if(bReadIniOnlyOnce)
		{
			bReadIniOnlyOnce = false;
			m_strIniProductName = m_strProductName;
			if(!ReadIniFile())
			{
				AfxMessageBox("請重新配置INI檔,確保有匹配的機種名稱,程式自動關閉!\n");
				::PostMessage(GetParent()->GetParent()->GetSafeHwnd(),WM_CLOSE,0,0);
				return 0;
			}

		}

		if(strncmp(szStatus, "PASS", 4))
			return 0;
		OnBnClickedBtnStart();
	}
	//12 pass
	else if(len == 25+12+12+4+2)//SN+MAC+PC+PASS
	{
		if(strstr(strBuf, "PASS") == NULL)
		{
			TestFail(SR00);
			return 0;
		}
		else
		{
			TestPass();
		}
	}
	//12 failed
	//else if(len == 25+12+12+4+4+2)//SN+MAC+PC+ERR+PASS
	//{
	//	if(strstr(strBuf, "PASS") == NULL)
	//	{
	//		TestFail(SR00);
	//		return 0;
	//	}
	//}
	else
	{
		//do nothing
	}
	return 1;
}
void CRunInfo::ShowMsg(CString strMsg)
{
	DisplayRunTimeInfo("");
	DisplayRunTimeInfo(strMsg.GetBuffer(strMsg.GetLength()));
	DisplayRunTimeInfo("");
}

UINT __cdecl OnRecvGoldenData(LPVOID pParam)
{
	ofstream os;
	os.open("1.txt",ios_base::app|ios_base::out);
	_GoldenInfo* pbtn = (_GoldenInfo*) pParam ;
	int iSubDelay = pbtn->nDelay*10;
	while(iSubDelay--)
	{
		/*recv*/		
		pbtn->szGoldenData.Empty();
		WSABUF  ReceiveBuff;
		ReceiveBuff.buf = new char[10240];
		ReceiveBuff.len = 10240;
		memset(ReceiveBuff.buf,0,10240);
		DWORD NumberOfBytesRecvd = 0;
		DWORD dwflag = 0;

		try
		{
			os<<"rece start\n";os.flush();
			WSARecv(pbtn->GoldenSocket,&ReceiveBuff,1,&NumberOfBytesRecvd,&dwflag,NULL,NULL);	
			os<<"rece ok\n";os.flush();
		}
		catch(...)
		{
			pRunInfo->DisplayRunTimeInfo("WSARecv error!");
		}
		
		char szbuf[10240];
		sprintf_s(szbuf,sizeof(szbuf),"%s",ReceiveBuff.buf);
		os<<"ReceiveBuff="<<szbuf<<endl;os.flush();
		os<<""<<endl;os.flush();

		pbtn->szGoldenData = ReceiveBuff.buf;			
		os<<"szGoldenData="<<pbtn->szGoldenData<<endl;os.flush();
		os<<""<<endl;os.flush();
		os.close();
		delete []ReceiveBuff.buf;
	
		if(strstr(GoldenInfo.szGoldenData,GoldenInfo.szReturnInfo) != NULL)
		{
			GoldenInfo.bResponeOK = true;
			SetEvent(h_golenEvt);
			break;
		}
		else
		{
			Sleep(100);
		}	
	}	
	if(iSubDelay = 1)
	{
		return 0;
	}
	return 1;

} 

LRESULT  CRunInfo::OnRecvEthernetData(WPARAM wParam,LPARAM lParam)
{
	switch(LOWORD(lParam)) 
	{
	case FD_READ:
		m_strStoreEthernetData.Empty();
		WSABUF  ReceiveBuff;
		ReceiveBuff.buf = new char[1024];
		ReceiveBuff.len = 1024;
		memset(ReceiveBuff.buf,0,1024);
		DWORD NumberOfBytesRecvd = 0;
		DWORD dwflag = 0;
		m_strStoreEthernetData.Empty();
		Sleep(500);
		WSARecv(m_Socket,&ReceiveBuff,1,&NumberOfBytesRecvd,&dwflag,NULL,NULL);
		DisplayRunTimeInfo(ReceiveBuff.buf);
		m_strStoreEthernetData = ReceiveBuff.buf;
		g_data += ReceiveBuff.buf;
		delete []ReceiveBuff.buf;
		SetEvent(m_hEvt);
		break;
	}
	
	return 0;
}

bool  CRunInfo::CheckMac(CString strMac)
{
	if (strMac.GetLength() == 12)
    {
        for (int iCount = 1; iCount <= 12; iCount++)
        {
            if ( ((strMac[iCount] >= '0') && (strMac[iCount] <= '9')) ||
                 ((strMac[iCount] >= 'A') && (strMac[iCount] <= 'F')) )
            {
                continue;
            }
            else
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

void CRunInfo::GetThroughputValue(IN char* SourceValue,OUT double& TXValue,double& RXValue)
{
	char* pFindTX = strstr(SourceValue, "/sec");
	
	if(pFindTX == NULL)
	{
		TXValue = 0;
		return;
	}
	char szTX[MINBUFSIZE] = "";
	
	strncpy_s(szTX, MINBUFSIZE, pFindTX-10, 10);
	
	if(strstr(szTX, "Gbits")!=NULL)
	{
		strncpy_s(szTX, MINBUFSIZE, szTX,4);
		TXValue = _tstof(szTX)*1024;
	}
	else if(strstr(szTX, "Mbits")!=NULL)
	{
		strncpy_s(szTX, MINBUFSIZE, szTX, 4);
		TXValue = _tstof(szTX);
	}
	else if(strstr(szTX, "Kbits")!=NULL)
	{
		strncpy_s(szTX, MINBUFSIZE, szTX, 4);
		TXValue = _tstof(szTX)/1024;
	}
	else
		TXValue = 0;

	char* pFindRX = strstr(pFindTX+4, "/sec");
	if(pFindRX==NULL)
	{
		RXValue = 0;
		return;
	}
	char szRX[MINBUFSIZE] = "";
	strncpy_s(szRX, MINBUFSIZE, pFindRX-10, 10);
	if(strstr(szRX, "Gbits")!=NULL)
	{
		strncpy_s(szRX, MINBUFSIZE, szRX, 4);
		RXValue = _tstof(szRX)*1024;
	}
	else if(_tcsstr(szRX, "Mbits")!=NULL)
	{
		strncpy_s(szRX, MINBUFSIZE, szRX, 4);
		RXValue = _tstof(szRX);
	}
	else if(strstr(szRX, "Kbits")!=NULL)
	{
		strncpy_s(szRX ,MINBUFSIZE, szRX, 4);
		RXValue = _tstof(szRX)/1024;
	}
	else
		RXValue = 0;

}

void CRunInfo::SendEthernetCmd(char* SendData)
{
	CStringA strSendData;
	DWORD    dwNumber;	
	WSABUF   sendbuf;
	sendbuf.buf=new char[300];
	sendbuf.len=300;
	memset(sendbuf.buf,0,300);
	
	strSendData  =SendData;
	strSendData +="\n";
	
	strcpy_s(sendbuf.buf,300,strSendData.GetBuffer(strSendData.GetLength()+1));
	
	WSASend(m_Socket,&sendbuf,1,&dwNumber,0,NULL,NULL);
	
	delete []sendbuf.buf;
}

UINT __cdecl  CRunInfo::ReadComForSfis(LPVOID param)
{
	CCom* SfisCom = (CCom*)param;
	DWORD   dwEvt;
	DWORD   dwErrorFlags;
	COMSTAT ComStat;
	char   szReadBuf[MINBUFSIZE] = "";
	
	SetCommMask(SfisCom->m_hCom, EV_RXCHAR);

	while(1)
	{
			
		WaitCommEvent(SfisCom->m_hCom, &dwEvt, NULL);
		if((dwEvt & EV_RXCHAR) == EV_RXCHAR)
		{
			ClearCommError(SfisCom->m_hCom ,&dwErrorFlags,&ComStat);
			if(ComStat.cbInQue>0)
			{
				SfisCom->ReadData(szReadBuf, MINBUFSIZE,1200);
				::PostMessage(SfisCom->m_hParentWnd, WM_COMET, 0, (LPARAM)szReadBuf);
			}
		}
	}
	return 1;
}

ULONG  CRunInfo::CovertMacToNumber(char* Mac,int lenth)
{
	ULONG dbResult = 0;
	double z       = 16.0;
	 char* revMac = _strrev(_strdup( Mac ));
	_tcsupr_s(revMac,lenth+1);
	int  IntMac[12] = {0};
	for(int i = 0; i<lenth;++i)
	{
		if('A' == revMac[i])
			IntMac[i] = 10;
		else if('B' == revMac[i])
			IntMac[i] = 11;
		else if('C' == revMac[i])
			IntMac[i] = 12;
		else if('D' == revMac[i])
			IntMac[i] = 13;	
		else if('E' == revMac[i])
			IntMac[i] = 14;
		else if('F' == revMac[i])
			IntMac[i] = 15;
		else
			IntMac[i] = revMac[i] - 48;
	}
	for(int j =0;j<lenth;++j)
	{
		dbResult+=(ULONG)(IntMac[j]*pow(z , j));
	}
	Mac = _strrev(revMac);
	return dbResult;
}

bool CRunInfo::GetIsSfis()
{
	return m_blIsSfis;
}

void CRunInfo::OnBnClickedBtnStart()
{
	IsDisplayErrorCode(false);
	DisplayRunTimeInfo(NULL);

	static bool bOnce = true;

	
	if(m_blIsOpenSfis&&m_blIsSfis)
	{
		/*szFWforSfis default value,if no this step program will error when DUT fail before telnet*/
		szFWforSfis = "00000000000000000000000000000";
		m_strIniProductName.Trim();
		m_strProductName.Trim();

		if(strcmp(m_strIniProductName,m_strProductName))
		{
			SendSfisResultAndShowFail(SY15);
			return ;	
		}

		if(m_strSN.GetLength() != 25)
		{
			AfxMessageBox("SN lable length error,please scan again!SN條碼長度錯誤，請重新掃描！");
			return ;
		}

		if(!strstr(m_IniFileInfo.szTestStatuse , "PT2"))
		{


			if(m_strMAC.GetLength() != 12)
			{
				AfxMessageBox("MAC lable length error,please scan again!MAC條碼長度錯誤，請重新掃描！");
				return ;
			}
			if(CheckMac(m_strMAC))
			{
				AfxMessageBox("MAC lable rule error,please scan again!MAC條碼規則錯誤，請重新掃描！");
				return ;
			}

			if(m_strPincode.GetLength() != 12)
			{	
				AfxMessageBox("PinCode lable length error,please scan again!PinCode條碼長度錯誤，請重新掃描！");
				return ;
			}
			else
			{
				for(int i=0;i<8;i++)
				{
					if(atoi(m_strPincode.Mid(i,1)) < 0 || atoi(m_strPincode.Mid(i,1)) > 9)
					{
						AfxMessageBox("PinCode lable rule error,please scan again!PinCode條碼規則錯誤，請重新掃描！");
						return ;
					}	
				}
			}

			if(m_strSSN.GetLength() != 15)
			{
				AfxMessageBox("SSN lable length error,please scan again!SSN條碼長度錯誤，請重新掃描！");
				return ;
			}
		}
	}
	
	else
	{
		m_BtnStart.EnableWindow(false);
		m_BtnQuit.EnableWindow(false);
		UpdateData();
		if(m_strSN.IsEmpty())
		{
			SendSfisResultAndShowFail(BR00);
			return ;
		}
		if(m_strMAC.IsEmpty())
		{
			SendSfisResultAndShowFail(BR00);
			return ;
		}
		if(m_strPincode.IsEmpty())
		{
			SendSfisResultAndShowFail(BR00);
			return ;
		}
		
		if(m_strSSN.IsEmpty())
		{
			SendSfisResultAndShowFail(BR00);
			return ;
		}

	}

	m_strStoreEthernetData.Empty();
	m_strRecordTestData.Empty();
	memset(TestDetaillog,0,sizeof(TestDetaillog));
	g_data.Empty();
	i_flag=0;

	CTime Time = CTime::GetCurrentTime();
	m_strRecordTestData  += Time.Format("%H:%M:%S");
	m_strRecordTestData  +=  "\t";
	m_TestRecordinfo.szRecordTime = Time.Format("%H:%M:%S");
	m_strRecordTestData  +=  m_strSN;
	m_strRecordTestData  +=  "\t";
	m_TestRecordinfo.szRecordSN = m_strSN;
	m_strRecordTestData  +=  m_strMAC;
	m_strRecordTestData  +=  "\t";
	m_TestRecordinfo.szRecordMac = m_strMAC;
	m_strRecordTestData  +=  m_strPincode;
	m_strRecordTestData  +=  "\t";
	m_TestRecordinfo.szRecordPincode = m_strPincode;
	m_strRecordTestData  +=  m_strSSN;
	m_strRecordTestData  +=  "\t";
	m_TestRecordinfo.szRecordPincode = m_strSSN;
	m_strRecordTestData  +=  m_strProductName;
	m_strRecordTestData  +=  "\t";
	m_TestRecordinfo.szRecordProductName = m_strProductName;
	m_strRecordTestData  +=  m_strPcName;
	m_strRecordTestData  +=  "\t";
	m_TestRecordinfo.szRecordPcName = m_strPcName;	
	
	m_strRecordTestData  +=  m_strSSID_2G;
	m_strRecordTestData  +=  "\t";
	m_strRecordTestData  +=  m_strPASS_2G;
	m_strRecordTestData  +=  "\t";
	m_strRecordTestData  +=  m_strSSID_5G;
	m_strRecordTestData  +=  "\t";
	m_strRecordTestData  +=  m_strPASS_5G;
	m_strRecordTestData  +=  "\t";

	m_TestTimeCount = 0;
	m_strTestResult = "RUN";
	m_strTestTime = "00:00";
	 
	UpdateData(false);

	SetTimer(1,1000,NULL);

	char szPassFilePath[MINBUFSIZE] = "";
	char szFailFilePath[MINBUFSIZE] = "";
	char szStationFilePath[MINBUFSIZE] = "";

	sprintf_s(szStationFilePath, MINBUFSIZE, "%s\\%s\\", m_IniFileInfo.szStoreSrvPath ,m_Station);
	sprintf_s(szPassFilePath, MINBUFSIZE, "%s\\%s\\PASS\\", m_IniFileInfo.szStoreSrvPath ,m_Station);
	sprintf_s(szFailFilePath, MINBUFSIZE, "%s\\%s\\FAIL\\", m_IniFileInfo.szStoreSrvPath ,m_Station);

	//creat local log path
	if(CreateDirectory("D:\\PASS",NULL) || (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		// do nothing
	}
	else
	{
		CString strMsg="Create local log pass path failed";
		AfxMessageBox(strMsg);
	}
	
	if(CreateDirectory("D:\\FAIL",NULL) || (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		// do nothing
	}
	else
	{
		CString strMsg="Create local log fail path failed";
		AfxMessageBox(strMsg);
	}


	//F:\lsy\ID\U12H197 : "F:\lsy\ID\" have yet exist,just need creat "\U12H197 "
	if(CreateDirectory(m_IniFileInfo.szStoreSrvPath,NULL) || (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		// do nothing
	}
	else
	{
		CString strMsg="Create " + CString(m_IniFileInfo.szStoreSrvPath)+" failed";
		AfxMessageBox(strMsg);
	}
	//F:\lsy\ID\U12H197\FT : "F:\lsy\ID\U12H197\" have yet exist,just need creat "\FT "
	if(CreateDirectory(szStationFilePath,NULL) || (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		// do nothing
	}
	else
	{
		CString strMsg="Create " + CString(szStationFilePath)+" failed";
		AfxMessageBox(strMsg);
	}

	if(CreateDirectory(szPassFilePath,NULL) || (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		// do nothing
	}
	else
	{
		CString strMsg="Create " + CString(szPassFilePath)+" failed";
		AfxMessageBox(strMsg);
	}

	if(CreateDirectory(szFailFilePath,NULL) || (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		// do nothing
	}
	else
	{
		CString strMsg=_T("Create ") +CString(szFailFilePath)+_T(" failed");
		AfxMessageBox(strMsg);
	}
	
	if(m_blIsSfis)
	{		
		if (_access(szPassFilePath,00) == -1 || _access(szFailFilePath, 00) == -1)
		{
			ShowMsg(szFailFilePath);
			SendSfisResultAndShowFail(SY05);
			return ;
		}
	}

	m_nLanMAC = CovertMacToNumber(m_strMAC.GetBuffer(m_strMAC.GetLength()), m_strMAC.GetLength());
    m_nWanMAC=  m_nLanMAC+1;

	if(
		(m_IniFileInfo.szTestStatuse == "") || (
		(!strstr(m_IniFileInfo.szTestStatuse , "DB")) && 
		(!strstr(m_IniFileInfo.szTestStatuse , "FT")) &&
		(!strstr(m_IniFileInfo.szTestStatuse , "PT2")) &&
		(!strstr(m_IniFileInfo.szTestStatuse , "RC")))
	  )
	{
		AfxMessageBox("TestInformation.ini 檔案中測試站別配置不對請確認");
		SendSfisResultAndShowFail(SY45);
		return;
	}
	
	if(strstr(m_IniFileInfo.szTestStatuse , "FT"))
	{			
		AfxBeginThread(FT_FunctionTestRun, this);
	}
	if(strstr(m_IniFileInfo.szTestStatuse , "RC"))
	{
		AfxBeginThread(RC_FunctionTestRun, this);
	}
	if(strstr(m_IniFileInfo.szTestStatuse , "DB"))
	{
		AfxBeginThread(Debug_FunctionTestRun, this);
	}
	if(strstr(m_IniFileInfo.szTestStatuse , "PT2"))
	{
		AfxBeginThread(PT2_FunctionTestRun, this);
	}
}
void CRunInfo::CollectServerLogData()
{
	m_strRecordTestData.Format
	("%25s\t%12s\t%12s\t%15s\t%20s\t%8s\t%12s\t%4s\t%30s\t",  
		m_TestRecordinfo.szRecordSN,
		m_TestRecordinfo.szRecordMac,
		m_TestRecordinfo.szRecordPincode,
		m_TestRecordinfo.szRecordSSN,
		m_TestRecordinfo.szRecordTime,
		m_TestRecordinfo.szRecordProductName,
		m_TestRecordinfo.szRecordPcName,
		m_TestRecordinfo.szRecordResult,
		m_TestRecordinfo.szRecordFirmware
		);		
}
bool CRunInfo::PingSpecifyIP(char* IP, int nSuccessCount)
{
	HANDLE hWritePipe  = NULL;
	HANDLE hReadPipe   = NULL;

	char  szPing[MINBUFSIZE] = "";
	sprintf_s(szPing, MINBUFSIZE, "ping.exe %s -n 35 -w %d", IP , m_IniFileInfo.nPingPacketCount);
	char   szReadFromPipeData[MAXBUFSIZE] = "";
	DWORD  byteRead    = 0;

	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle =true;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	if(!CreatePipe(&hReadPipe,&hWritePipe,&sa,0))
	{
		DisplayRunTimeInfo("Create pipe fail");
		return 0;
	}

	PROCESS_INFORMATION pi;
	STARTUPINFO        si;
	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
	si.hStdError  = hWritePipe;
	si.hStdOutput = hWritePipe;
	si.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow=SW_HIDE;

	if(!CreateProcess(NULL,szPing,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
	{
		DisplayRunTimeInfo("Start run ping.exe fail!");
		return 0;
	}

	CloseHandle(pi.hThread);
	CloseHandle(hWritePipe);
	CloseHandle(pi.hProcess);

	DWORD  dwPingFailCount    = 35;
	DWORD  dwPingSuccessCount = nSuccessCount;
	
	while(TRUE)
	{
		memset(szReadFromPipeData,0,MAXBUFSIZE);
		Sleep(100);
		int bResult = ReadFile(hReadPipe,szReadFromPipeData,MAXBUFSIZE,&byteRead,NULL);
		
		if(!bResult)
		{
			DisplayRunTimeInfo("Read ping.exe fail!");
			return 0;
		}

		Sleep(200);

		char IPInfo[MINBUFSIZE] ="";
		sprintf_s(IPInfo, MINBUFSIZE, "Reply from %s", IP);

		
		if(strstr(szReadFromPipeData,IPInfo))
		{
			dwPingSuccessCount--;
		}
		else
		{
			dwPingFailCount--;
		}
		DisplayRunTimeInfo(szReadFromPipeData);
		if(!dwPingSuccessCount)
		{
			ReadFile(hReadPipe,szReadFromPipeData,MAXBUFSIZE,&byteRead,NULL);
			DisplayRunTimeInfo(szReadFromPipeData);
			return 1;
		}
		if(!dwPingFailCount)
			return 0;
	}
}

bool CRunInfo::PingSpecifyIP_2(char* IP, int nSuccessCount)
{
	HANDLE hWritePipe  = NULL;
	HANDLE hReadPipe   = NULL;

	char  szPing[MINBUFSIZE] = "";
	sprintf_s(szPing, MINBUFSIZE, "ping.exe %s -n 10 -w %d", IP , m_IniFileInfo.nPingPacketCount);
	char   szReadFromPipeData[MAXBUFSIZE] = "";
	DWORD  byteRead    = 0;

	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle =true;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	if(!CreatePipe(&hReadPipe,&hWritePipe,&sa,0))
	{
		DisplayRunTimeInfo("Create pipe fail");
		return 0;
	}

	PROCESS_INFORMATION pi;
	STARTUPINFO        si;
	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
	si.hStdError  = hWritePipe;
	si.hStdOutput = hWritePipe;
	si.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow=SW_HIDE;

	if(!CreateProcess(NULL,szPing,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
	{
		DisplayRunTimeInfo("Start run ping.exe fail!");
		return 0;
	}

	CloseHandle(pi.hThread);
	CloseHandle(hWritePipe);
	CloseHandle(pi.hProcess);

	DWORD  dwPingFailCount    = 10;
	DWORD  dwPingSuccessCount = nSuccessCount;
	
	while(TRUE)
	{
		memset(szReadFromPipeData,0,MAXBUFSIZE);
		Sleep(100);
		int bResult = ReadFile(hReadPipe,szReadFromPipeData,MAXBUFSIZE,&byteRead,NULL);
		
		if(!bResult)
		{
			DisplayRunTimeInfo("Read ping.exe fail!");
			return 0;
		}

		Sleep(200);
		char IPInfo[MINBUFSIZE] ="";
		sprintf_s(IPInfo, MINBUFSIZE, "Reply from %s", IP);
		
		if(strstr(szReadFromPipeData,IPInfo))
		{
			dwPingSuccessCount--;
		}
		else
		{
			dwPingFailCount--;
		}
		DisplayRunTimeInfo(szReadFromPipeData);
		if(!dwPingSuccessCount)
		{
			ReadFile(hReadPipe,szReadFromPipeData,MAXBUFSIZE,&byteRead,NULL);
			DisplayRunTimeInfo(szReadFromPipeData);
			return 1;
		}
		if(!dwPingFailCount)
			return 0;
	}
}

bool CRunInfo::RunTelnetExe(char* GetMacValue)
{
	char szRunTelnetExeFullName[MAXBUFSIZE] = "";
	sprintf_s(szRunTelnetExeFullName, MAXBUFSIZE, "telnetenable %s %s Gearguy Geardog", m_IniFileInfo.szDefaultIP1, GetMacValue);
	
	STARTUPINFO si;
	GetStartupInfo(&si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi;

	if(!CreateProcess(NULL,szRunTelnetExeFullName,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
	{
		DisplayRunTimeInfo("Not found Telnetenble.exe\n Test over");
		return false;
	}

	if(WAIT_TIMEOUT == WaitForSingleObject(pi.hProcess,5000))
	{
			TerminateProcess(pi.hProcess,1);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return false;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}
UINT __cdecl  CRunInfo::Debug_FunctionTestRun(LPVOID parameter)
{
	CRunInfo* pRun = (CRunInfo*)parameter;
	int iRet=-1;

	pRun->TestPass();
	pRun->TestFail(IN10);
	pRun->TestPass();
	pRun->TestPass();
	pRun->TestPass();
	pRun->TestPass();
	pRun->TestFail(IN10);


	return 1;
}

UINT __cdecl  CRunInfo::FT_FunctionTestRun(LPVOID parameter)
{
	CRunInfo* pRun = (CRunInfo*)parameter;
	int iRet=-1;
	
	//check DUT BootUp
	if(!pRun->CheckDutBootUp())
	{
		return 0;
	}

	//change "/"in FW to "_" for SFIS
	if(!pRun->DutCheckVer())
	{
		return 0;
	}

	//open Throughput Golden server
	if(pRun->m_IniFileInfo.nThroughputGoldenFlag == 1)
	{
		//open Golden socket
		pRun->InitGoldenSocket();
	}

	
	// start wifi throughput test
	iRet = pRun->DutTestWiFiThroughput_1();
	char szBuf[128];
	sprintf_s(szBuf , 128 , "iRet=%d" , iRet);
	pRun->DisplayRunTimeInfo(szBuf);
	switch (iRet)
	{
	case 0:
		break;
	case 1:
		pRun->SendSfisResultAndShowFail(SY55);
		return 0;
	case 2:
		pRun->SendSfisResultAndShowFail(EH20);
		return 0;
	case 3:
		pRun->SendSfisResultAndShowFail(EH50);
		return 0;
	case 4: 
		pRun->SendSfisResultAndShowFail(EH25);
		return 0;
	case 5: 
		pRun->SendSfisResultAndShowFail(EH22);
		return 0;
	case 6: 
		pRun->SendSfisResultAndShowFail(TP51);
		return 0;
	case 7:
		pRun->SendSfisResultAndShowFail(TP56);
		return 0;
	case 8:
		pRun->SendSfisResultAndShowFail(TP21);
		return 0;
	case 9:
		pRun->SendSfisResultAndShowFail(TP26);
		return 0;
	case 10:
		pRun->SendSfisResultAndShowFail(TP52);
		return 0;
	case 11: 
		pRun->SendSfisResultAndShowFail(TP53);
		return 0;
	case 12: 
		pRun->SendSfisResultAndShowFail(TP55);
		return 0;
	case 13: 
		pRun->SendSfisResultAndShowFail(TP22);
		return 0;
	case 14: 
		pRun->SendSfisResultAndShowFail(TP23);
		return 0;
	case 15: 
		pRun->SendSfisResultAndShowFail(TP25);
		return 0;
	case 88: 
		pRun->SendSfisResultAndShowFail(IP00);
		return 0;
	default:
		break;
	}	

	// Burn mac
	if(!pRun->DutBurnMac())
	{
		pRun->SendSfisResultAndShowFail(WC10);
		return 0;
	}

	// Burn Security Pin Code
	if(!pRun->DutBurnPin())
	{
		pRun->SendSfisResultAndShowFail(WC30);
		return 0;
	}

	// Burn Customer Serial Number
	if(!pRun->DutBurnSn())
	{
		pRun->SendSfisResultAndShowFail(WC40);
		return 0;
	}


	if(strcmp(pRun->m_IniFileInfo.szIsHaveSSIDLable , "1") == 0)
	{		
		char sz2gSSID[128] = "";
		sprintf_s(sz2gSSID , sizeof(sz2gSSID) , "%s" , pRun->m_strSSID_2G.Trim());
		//Check 2.4G SSID format
		if(!(pRun->Check_SSIDFormat(sz2gSSID)))
		{
			pRun->SendSfisResultAndShowFail(CC75);
			return 0;
		}
		//burn 2.4G SSID
		if(!(pRun->Write_Code("burnssid" , sz2gSSID , "OK")))
		{
			pRun->SendSfisResultAndShowFail(WC71);
			return 0;
		}
			
		char sz2gPASS[128] = "";
		sprintf_s(sz2gPASS , sizeof(sz2gPASS) , "%s" , pRun->m_strPASS_2G.Trim());
		//Check 2.4G Passphrase format
		if(!pRun->Check_Passphrase(sz2gPASS))
		{
			pRun->SendSfisResultAndShowFail(CC76);
			return 0;	
		}
		//burn 2.4G pass	
		if(!(pRun->Write_Code("burnpass" , sz2gPASS , "OK")))
		{
			pRun->SendSfisResultAndShowFail(WC72);
			return 0;
		}
		
		char sz5gSSID[128] = "";
		sprintf_s(sz5gSSID , sizeof(sz5gSSID) , "%s-5G" , pRun->m_strSSID_5G.Trim());
		//Check 5G SSID format
		if(!(pRun->Check_SSIDFormat(sz5gSSID)))
		{
			pRun->SendSfisResultAndShowFail(CC75);
			return 0;
		}
		//burn 5G SSID
		if(!(pRun->Write_Code("burn5gssid" , sz5gSSID , "OK")))
		{
			pRun->SendSfisResultAndShowFail(WC73);
			return 0;
		}

		
		char sz5gPASS[128] = "";
		sprintf_s(sz5gPASS , sizeof(sz5gPASS) , "%s" , pRun->m_strPASS_5G.Trim());
		//Check 5G Passphrase format
		if(!pRun->Check_Passphrase(sz5gPASS))
		{
			pRun->SendSfisResultAndShowFail(CC78);
			return 0;
		}
		//burn 5G pass
		if(!(pRun->Write_Code("burn5gpass" , sz5gPASS , "OK")))
		{
			pRun->SendSfisResultAndShowFail(WC74);
			return 0;
		}
	}

	//Burn Region Code
	if(!pRun->DutBurnRegion())
	{
		pRun->SendSfisResultAndShowFail(WC50);
		return 0;		
	}

	/*
	VN OOBA　found:nvram parameters and board data parameters not same.
	VN TE co-work with SW and found:if add nvram loaddefault at FT station and after setting wan port, can found this issue.
	This action useful all only NAND falsh product.（AC1450/R6300v2/R6250）
	*/

	//loaddefault
	if(!pRun->DutLoadDefault())
	{
		pRun->SendSfisResultAndShowFail(OS50);
		return 0;
	}

	//Disable wireless
	if(!pRun->DutDisableWireless())
	{
		pRun->SendSfisResultAndShowFail(DS10);
		return 0;
	}

	//fix wan port
	if(!pRun->DutFixWanSetting())
	{
		pRun->SendSfisResultAndShowFail(SY75);
		return 0;
	}

	//Sleep a time:RC find some wan port setting failed DUT
	Sleep(5000);

	// Test end, show pass
	pRun->SendSfisResultAndShowPass();

	return 1;
}

UINT __cdecl  CRunInfo::RC_FunctionTestRun(LPVOID parameter)
{
	CRunInfo* pRun = (CRunInfo*)parameter;
	int iRet=-1;
	
	// Create socket connection with DUT, telent port 23
	if(!pRun->CheckDutBootUp())
	{
		return 0;
	}

	//Check DUT Lan port
	iRet = pRun->CHECK_LAN_PORT();	
	pRun->DisplayRunTimeInfo("pRun->CHECK_LAN_PORT() return value iRet=" , iRet);
	switch (iRet)
	{
	case -1:
		break;
	case 2:
		pRun->SendSfisResultAndShowFail(IN12);
		return 0;
	case 3:
		pRun->SendSfisResultAndShowFail(IN13);
		return 0;
	case 4:
		pRun->SendSfisResultAndShowFail(IN14);
		return 0;
	case 12: 
		pRun->SendSfisResultAndShowFail(TP12);
		return 0;
	case 13: 
		pRun->SendSfisResultAndShowFail(TP13);
		return 0;
	case 14: 
		pRun->SendSfisResultAndShowFail(TP14);
		return 0;	
	default:
		break;
	}	

	//Check DUT Wan_port
	iRet = pRun->CHECK_WAN_PORT();	
	pRun->DisplayRunTimeInfo("pRun->CHECK_WAN_PORT() return value iRet=" , iRet);
	switch (iRet)
	{
	case -1:
		break;
	case 1:
		pRun->SendSfisResultAndShowFail(SY75);
		return 0;
	case 2:
		pRun->SendSfisResultAndShowFail(IN50);
		return 0;
	case 3:
		pRun->SendSfisResultAndShowFail(TP50);
		return 0;
	default:
		break;
	}

	//check USB	
	
	iRet = pRun->DutCheckUsb();		
	//kill httpd process
	pRun->SendDutCommand("killall httpd","#",10000);

	pRun->DisplayRunTimeInfo("pRun->DutCheckUsb() return value iRet=" , iRet);
	switch(iRet)
	{
	case -1:
		break;
	case 1:
		pRun->SendSfisResultAndShowFail(UB01);
		return 0;
	case 2:
		pRun->SendSfisResultAndShowFail(UB02);
		return 0;
	case 4:
		pRun->SendSfisResultAndShowFail(UB03);
		return 0;
	case 5:
		pRun->SendSfisResultAndShowFail(UB04);
		return 0;
	case 6:
		pRun->SendSfisResultAndShowFail(UB05);
		return 0;
	case 7:
		pRun->SendSfisResultAndShowFail(UB06);
		return 0;
	case 8:
		pRun->SendSfisResultAndShowFail(UB07);
		return 0;
	case 9:
		pRun->SendSfisResultAndShowFail(UB08);
		return 0;
	case 10:
		pRun->SendSfisResultAndShowFail(UB08);
		return 0;
	case 11:
		pRun->SendSfisResultAndShowFail(UB09);
		return 0;
	case 12:
		pRun->SendSfisResultAndShowFail(UB10);
		return 0;
	case 91:
		pRun->SendSfisResultAndShowFail(UB00);
		return 0;
	}

	// check dut version information
	if(!pRun->CheckFirmwareVersion())
	{	
		pRun->SendSfisResultAndShowFail(CC70);
		return 0;
	}
	//check BoardId
	if(!pRun->DutCheckBoardID())
	{
		pRun->SendSfisResultAndShowFail(CC60);
		return 0;
	}

	// Check mac
	if(!pRun->DutCheckMAC())
	{
		pRun->SendSfisResultAndShowFail(CC10);
		return 0;
	}

	// Check Security Pin Code
	if(!pRun->DutCheckPin())
	{
		pRun->SendSfisResultAndShowFail(CC30);
		return 0;
	}

	// Check Customer Serial Number
	if(!pRun->DutCheckSn())
	{
		pRun->SendSfisResultAndShowFail(CC40);
		return 0;
	}


	if(strcmp(pRun->m_IniFileInfo.szIsHaveSSIDLable , "1") == 0)
	{
		
		////Check Passphrase format
		//if(!pRun->Check_Passphrase(pRun->(m_strPASS_2G.Trim()).GetBuffer(pRun->(m_strPASS_2G.Trim()).GetAllocLength())))
		//{
		//	pRun->SendSfisResultAndShowFail(CC72);
		//	return 0;	
		//}
		//if(!pRun->Check_Passphrase(pRun->m_strPASS_5G.GetBuffer(pRun->m_strPASS_5G.GetAllocLength())))
		//{
		//	pRun->SendSfisResultAndShowFail(CC74);
		//	return 0;	
		//}

		//check 2.4G SSID

		if(!pRun->CHECK_SSID_2G( pRun->m_strSSID_2G.Trim().GetBuffer(pRun->m_strSSID_2G.GetLength())))
		{
			pRun->SendSfisResultAndShowFail(CC71);
			return 0;
		}
		//check 5G SSID
		if(!pRun->CHECK_SSID_5G( pRun->m_strSSID_5G.Trim().GetBuffer(pRun->m_strSSID_5G.GetLength())))
		{
			pRun->SendSfisResultAndShowFail(CC73);
			return 0;
		}
		//check 2.4G PASS
		if(!pRun->CHECK_WPA_PASSEPHRASE_2G( pRun->m_strPASS_2G.Trim().GetBuffer(pRun->m_strPASS_2G.GetLength())))
		{
			pRun->SendSfisResultAndShowFail(CC72);
			return 0;
		}

		//check 5G PASS
		if(!pRun->CHECK_WPA_PASSEPHRASE_5G( pRun->m_strPASS_5G.Trim().GetBuffer(pRun->m_strPASS_5G.GetLength())))
		{
			pRun->SendSfisResultAndShowFail(CC74);
			return 0;
		}
	}
	//check String table chksum
	if(!pRun->DutCheckStringTableChecksum())
	{
		pRun->SendSfisResultAndShowFail(CC80);
		return 0;
	}

	// Check Region Code
	if(!pRun->DutCheckRegion())
	{
		pRun->SendSfisResultAndShowFail(CC50);
		return 0;
	}

	//Check Button
	if(!pRun->DutCheckButton_Auto())
	{
		return 0;
	}

	//Check LED
	if(!pRun->DutCheckLed())	
	{
		return 0;
	}

	//loaddefault
	if(!pRun->DutLoadDefault())
	{
		pRun->SendSfisResultAndShowFail(OS50);
		return 0;
	}

	//pot stop
	if(!pRun->DutPot())
	{
		pRun->SendSfisResultAndShowFail(OS10);
		return 0;
	}

	// Test end, show pass
	pRun->SendSfisResultAndShowPass();

	return 1;
}

UINT __cdecl  CRunInfo::PT2_FunctionTestRun(LPVOID parameter)
{
	CRunInfo* pRun = (CRunInfo*)parameter;
	int iRet=-1;
		

	// Create socket connection with DUT, telent port 23
	if(!pRun->CheckDutBootUp())
	{
		return 0;
	}

	////Check USB
	//if(!pRun->DECT_USB())
	//{
	//	pRun->SendSfisResultAndShowFail(UB00);
	//	return 0;
	//}

	
	//Check DUT Lan port
	iRet = pRun->CHECK_LAN_PORT();	
	pRun->DisplayRunTimeInfo("pRun->CHECK_LAN_PORT() return value iRet=" , iRet);
	switch (iRet)
	{
	case -1:
		break;
	case 2:
		pRun->SendSfisResultAndShowFail(IN12);
		return 0;
	case 3:
		pRun->SendSfisResultAndShowFail(IN13);
		return 0;
	case 4:
		pRun->SendSfisResultAndShowFail(IN14);
		return 0;
	case 12: 
		pRun->SendSfisResultAndShowFail(TP12);
		return 0;
	case 13: 
		pRun->SendSfisResultAndShowFail(TP13);
		return 0;
	case 14: 
		pRun->SendSfisResultAndShowFail(TP14);
		return 0;	
	default:
		break;
	}

	//Check DUT Wan_port
	iRet = pRun->CHECK_WAN_PORT();	
	pRun->DisplayRunTimeInfo("pRun->CHECK_WAN_PORT() return value iRet=" , iRet);
	switch (iRet)
	{
	case -1:
		break;
	case 1:
		pRun->SendSfisResultAndShowFail(SY55);
		return 0;
	case 2:
		pRun->SendSfisResultAndShowFail(IN50);
		return 0;
	case 3:
		pRun->SendSfisResultAndShowFail(TP50);
		return 0;
	default:
		break;
	}


	//// check dut version information
	//if(!pRun->CheckFirmwareVersion())
	//{	
	//	pRun->SendSfisResultAndShowFail(CC70);
	//	return 0;
	//}
	////check BoardId
	//if(!pRun->DutCheckBoardID())
	//{
	//	pRun->SendSfisResultAndShowFail(CC60);
	//	return 0;
	//}	

	//Check Button
	if(!pRun->DutCheckButton_Auto())
	{
		return 0;
	}
	//Check USB
	if(!pRun->DECT_USB())
	{
		pRun->SendSfisResultAndShowFail(UB00);
		return 0;
	}

	//Check LED
	if(!pRun->DutCheckLed_PT2())	
	{
		return 0;
	}
	//Disable wireless
	if(!pRun->DutDisableWireless())
	{
		pRun->SendSfisResultAndShowFail(DS10);
		return 0;
	}
	// Test end, show pass
	pRun->SendSfisResultAndShowPass();

	return 1;
}

bool CRunInfo::CheckFirmwareVersion(void)
{
	bool bResult = false;
	ResetEvent(m_hEvt);
	SendDutCommand("version", "#", 10000);

	if(strstr(g_data.GetBuffer(g_data.GetLength()), m_IniFileInfo.szStoreBootCode) == NULL )//check bootcode version
	{
		bResult = false;
		goto __EXIT;
	}


	char    Seps[] = "\r\n";
	char    *token;
	char    *Context ;
	token  = strtok_s((g_data.GetBuffer(g_data.GetLength())), Seps, &Context);
	while(token != NULL)
	{
		token = strtok_s(NULL, Seps, &Context);

		CString str = token;
		str.Trim();

		if(strcmp(str.GetBuffer(str.GetLength()), m_IniFileInfo.szStoreFirmware) == 0)//Check FW Version
		{
			bResult = true;		
			break;			
		}
	}

__EXIT:
	/*collect test data*/
	sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s,", TestDetaillog , m_IniFileInfo.szStoreFirmware, m_IniFileInfo.szStoreBootCode);
	/*End*/
	return bResult ;
}

bool CRunInfo::GetSSIS(char*  szEthernetInfo,CString& str)
{
	char    SepsPin[] = "";
	char    *token;
	char    *ContextPin ;
	token  = strtok_s(szEthernetInfo, SepsPin, &ContextPin);
	if(token != NULL)
	{
		token = strtok_s(NULL, SepsPin, &ContextPin);

		str = token;
		str.Trim();
		return true;
	}
	else
	{
		return false;
	}
}

bool CRunInfo::GetSecType(char*  szEthernetInfo,CString& str)
{
	char    SepsPin[] = "";
	char    *token;
	char    *ContextPin ;
	token  = strtok_s(szEthernetInfo, SepsPin, &ContextPin);
	if(token != NULL)
	{
		token = strtok_s(NULL, SepsPin, &ContextPin);

		str = token;
		str.Trim();
		return true;
	}
	else
	{
		return false;
	}
}


void CRunInfo::OnBnClickedBtnQuit()
{
	// TODO: Add your control notification handler code her

	shutdown(m_Socket , 0);
	if(m_Socket!=INVALID_SOCKET)
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
	shutdown(m_GoldenSocket , 0);
	if(m_GoldenSocket!=INVALID_SOCKET)
	{
		closesocket(m_GoldenSocket);
		m_GoldenSocket = INVALID_SOCKET;
	}
	
	WSACleanup();
	::PostMessage(GetParent()->GetParent()->m_hWnd,WM_CLOSE,0,0);
}


void CRunInfo::OnCbnSelchangeCom()
{
	// TODO: Add your control notification handler code here
	if(!m_blIsOpenSfis && m_blIsSfis)
	{
		GetDlgItemText(IDC_COM,m_strCom);
		if(!m_ComSfis.Open(m_strCom.GetBuffer(m_strCom.GetLength()),9600))
		{
			AfxMessageBox("選擇的COM口不存在或者被其他應用程序占用\n請從新選擇COM與SFIS連接\nThe select COM port don't exit or is used by other application!\nPls choose COM port again");
			return;
		}
		
		m_ComSfis.SendData(("Hello\r\n"), (int)strlen("Hello\r\n"));

		char pBuf[MINBUFSIZE] = "";
		m_ComSfis.ReadData(pBuf, MINBUFSIZE, 1500);

		if(strstr(pBuf, "ERRO") == NULL)
		{
			AfxMessageBox(_T("請從新選擇COM與SFIS連接\n"));
			m_ComSfis.Close();
			return ;
		}
		else
		{
			m_strSfisStatus = "SFIS ON";
			UpdateData(false);
			m_blIsOpenSfis = true;
			AfxBeginThread(ReadComForSfis, &m_ComSfis);
		}
	}
}

bool CRunInfo::LinkDataBase(void)
{
	if(pInstMydas != NULL)
	{
		if(PTSC_GetLinkStatus(pInstMydas))
		{
			return 1;
		}
	}
	char dutName[20];
	char svip[15];
	char version[15];
	char productName[15];
	memset(dutName,0,sizeof(dutName));
	memset(svip,0,sizeof(svip));	
	memset(version,0,sizeof(version));	
	memset(productName,0,sizeof(productName));

	char szPath[MAX_PATH] = "";
	char szFullPath[MAX_PATH] = "";
	char ProductName[MINBUFSIZE] = "";
	GetCurrentDirectory(MAX_PATH, szPath);

	sprintf_s(szFullPath, MAX_PATH, "%s\\%s", szPath , m_IniFileInfo.szTestInfoFileName);

	GetPrivateProfileString("General","MYDAS_IP","10.117.32.106",svip,15,szFullPath);
	GetPrivateProfileString("General","DUT_NAME","U12H197",dutName,15,szFullPath);
	GetPrivateProfileString("General","MYDAS_VERSION","1.0",version,15,szFullPath);


	strcpy_s(m_Mydas.Ip, sizeof(m_Mydas.Ip),svip);
	strcpy_s(m_Mydas.ProductPN, sizeof(m_Mydas.ProductPN),m_strProductName.Trim());
	strcpy_s(m_Mydas.Product,sizeof(productName),dutName);
	strcpy_s(m_Mydas.ComName, sizeof(m_Mydas.ComName),m_strPcName);	
	strcpy_s(m_Mydas.TestStaName, sizeof(m_Mydas.TestStaName),m_IniFileInfo.szTestStatuse);
	strcpy_s(m_Mydas.DaigVersion, sizeof(m_Mydas.DaigVersion),"V0.0.1");
	strcpy_s(m_Mydas.TitleVersion,sizeof(m_Mydas.TitleVersion),version);
	if (!PTSC_Connect(&hDev,&m_Mydas))
	{
		return 0;
	}

	return 1;
}
bool CRunInfo::SendDatatoMYDAS(char* DetailLog,char* ErrDef,char* SendDataBuf)
{
	HANDLE hEventRev=CreateEvent( 
		NULL,       // default security attributes
		TRUE,       // auto-reset event
		FALSE,      // initial state is unsignaled
		NULL		// object name
		); 

	PTSC_SetSendData(hDev,DetailLog,(unsigned int)strlen(DetailLog),0);
	PTSC_SetSendData(hDev,ErrDef,(unsigned int)strlen(ErrDef),1);
	PTSC_SetSendData(hDev,SendDataBuf,(unsigned int)strlen(SendDataBuf),2);
	PTSC_Send(hDev);
	CloseHandle(hEventRev);

	return TRUE;
}

void CRunInfo::GetLockCode(char* pIMEI, char* pLockCode)
{
	char szTemp[128]="";
	
	HMODULE hDLL=LoadLibrary("WPA_KEY.dll");
	if(hDLL == NULL)
	{
		DisplayRunTimeInfo("Load Library WPA_KEY.dll failed");
		return;
	}

	pFunction fnGetLockCode = (pFunction)GetProcAddress(hDLL,"_telkomGenDefaultsKey");
	fnGetLockCode(pIMEI,pLockCode);
	sprintf_s(szTemp,sizeof(szTemp),"IMEI: %s -> Lock Code: %s",pIMEI,pLockCode);
	DisplayRunTimeInfo(szTemp);

	FreeLibrary(hDLL);
	::Sleep(100);
	if(hDLL!=NULL)
	{
		DisplayRunTimeInfo("Free Library WPA_KEY.dll fail");
	}	
}
void CRunInfo::RunArpDelelte(char* pszIpAddr)
{
	CString strTemp;
	TCHAR szArpCommand[MINBUFSIZE] = "";
	sprintf_s(szArpCommand,"arp -d %s",pszIpAddr);
	RunSpecifyExeAndRead(strTemp,szArpCommand);   
}

bool CRunInfo::PingDUT(char* pszIpAddr, int iCount)
{
	DisplayRunTimeInfo("--------------Ping DUT----------------\n");
	
	if(PingSpecifyIP(pszIpAddr,iCount))
	{
		return true;
	}

	return false;
}

bool CRunInfo::RunTelnetEnable(char* pszIpAddr)
{
	if( strcmp(m_IniFileInfo.szRework,"1") == 0)
	{
		CString strArpInfo;
		char szMac[MINBUFSIZE] = "";
		char szTemp[MINBUFSIZE] ="";
		sprintf_s(szTemp,sizeof(szTemp),"arp -a %s",pszIpAddr);
		RunSpecifyExeAndRead(strArpInfo,szTemp,true);
		GetMac(strArpInfo.GetBuffer(strArpInfo.GetLength())+1,szMac);
		_strupr_s(szMac,MINBUFSIZE);

		if(!RunTelnetExe(szMac))
		{			
			return false;
		}
		Sleep(1000);
	}

	return true;
}

bool CRunInfo::DutSocketConnection(char* DefaultIP)
{
	/*
	true	:pass
	false	:failed
	*/
	ResetEvent(m_hEvt);
	if(!InitSocket(DefaultIP))
	{
		return false;
	}

	if(WAIT_OBJECT_0 != WaitForSingleObject(m_hEvt,9000))
	{
		return false;
	}
	Sleep(500);
	if(strstr(m_strStoreEthernetData,"BusyBox") == NULL)
	{
		return false;
	}

	return true;
}

bool CRunInfo::DutCheckVer(void)
{
	/*
	true	:pass
	false	:failed
	*/
	if(strcmp(m_IniFileInfo.szCheckVersion, "1") == 0)
	{
		ResetEvent(m_hEvt);
		SendEthernetCmd("version");
		if(WAIT_OBJECT_0 != WaitForSingleObject(m_hEvt,15000))
		{
			TestFail(SY55);
			return false;
		}
		CString szFW;
		/*Change '/' to '_'*/
		szFW = CustomerFirmwareVersion(m_strStoreEthernetData.GetBuffer(m_strStoreEthernetData.GetLength()));
		/*End*/
		if(szFW.Trim().GetLength() > 30)
		{
			szFWforSfis = szFW.Trim().Left(30);
		}
		else
		{
			szFWforSfis = szFW.Trim();
		}
		/*End*/
		m_strRecordTestData  += szFWforSfis;
		m_strRecordTestData  +=  "\t";
		m_TestRecordinfo.szRecordFirmware = szFWforSfis;
		sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,",szFWforSfis);

	}

	return true;
}

bool CRunInfo::DutCheckButton(void)
{
		ResetEvent(m_hEvt);
		/*Button test Command*/
		if(IDNO == MessageBox("請按 WPS 按鈕，持續1秒鐘！然後檢查 WPS 燈變綠色并不斷閃爍！", "LED Check" , MB_YESNO))
		{
			SendSfisResultAndShowFail(LD10);
			return false;
		}
		if(!SendDutCommand("nvram set wla_wlanstate=Enable","#",1000))
			return false;
		SendDutCommand("nvram set TE_TEST=1","#",2000);
		SendDutCommand("killall swresetd","#",2000);
		SendDutCommand("swresetd","#",2000);
		/*End*/
		if(WAIT_OBJECT_0 != WaitForSingleObject(m_hEvt,15000))
		{
			TestFail(SY55);
			return false;
		}
		/*button 1*/

		bool blPassY = false;
		for(int i = 0; i <3; i++)
		{
			if(IDOK == AfxMessageBox("請按一下WPS按鈕,持續一秒鐘!!!"))
			{
				if(strstr(m_strStoreEthernetData,"WPS button is pressed"))
				{
					blPassY = true;
					break;
				}
			}
		}
		if(!blPassY)
		{
			SendSfisResultAndShowFail(BC30);  
			return false;
		}
		/*button 2*/
		blPassY = false;
		for(int i = 0; i <3; i++)
		{
			if(IDOK == AfxMessageBox("請按一下Reset按鈕,持續一秒鐘!!!"))
			{
				if(strstr(m_strStoreEthernetData,"reset button is pressed"))
				{
					blPassY = true;
					break;
				}
			}
		}
		if(!blPassY)
		{
			SendSfisResultAndShowFail(BC20);  
			return false;
		}
		
	return true;
}
bool CRunInfo::DutCheckButton_Auto(void)
{
	/*
	true	:pass
	false	:failed
			BC20
			BC30
			BC40
	*/
	ResetEvent(m_hEvt);

	/*Button test Command*/
	SendDutCommand("reset_no_reboot","#",5000);
	SendDutCommand("killall swresetd","#",5000);
	SendDutCommand("swresetd","#",5000);
	/*End*/
	if(WAIT_OBJECT_0 != WaitForSingleObject(m_hEvt,15000))
	{
		TestFail(SY55);
		return false;
	}
	
	int iButtonRelaseState = CheckButtonRelease();
	if( iButtonRelaseState != 0 )
	{
		if(iButtonRelaseState == 1)//reset button fail
		{
			SendSfisResultAndShowFail(BC21);
			return false;
		}
		else if(iButtonRelaseState == 2)//WPS button fail
		{
			SendSfisResultAndShowFail(BC31);
			return false;
		}
		else if(iButtonRelaseState == 3)//WIFI button fail
		{
			SendSfisResultAndShowFail(BC41);
			return false;
		}
	}

	char*  szText;
	char*  szWindowName;
	char*  szReturnValue;
	int	   iRty;

	/*button 1*/
	szText = "請按一下WPS按鈕,持續一秒鐘!!!";	/*change Text you nend*/
	szWindowName = "WPS Button Check";			/*change WindowName code you nend*/
	szReturnValue = WPS_BTN_VALUE;
	//szReturnValue = "WPS button is pressed";	/*change ReturnValue you nend*/

	ButtonInfo.bTestOK = false;
	ButtonInfo.szReturnInfo = szReturnValue;
	ButtonInfo.szWindowName = szWindowName;

	iRty = 3;
	while(iRty)
	{
		iRty--;
		AfxBeginThread( AutoPressButton , &ButtonInfo);
		MessageBox(szText , szWindowName ,NULL);
		if(!ButtonInfo.bTestOK)
		{
			continue;
		}
		else
		{
			break;
		}
	}
	if(!ButtonInfo.bTestOK)
	{
		SendSfisResultAndShowFail(BC30);
		return false;	
	}

	/*button 2*/
	szText = "請按一下WiFi按鈕,持續一秒鐘!!!";	/*change Text you nend*/
	szWindowName = "WiFi Button Check";		/*change WindowName code you nend*/
	szReturnValue = WIFI_BTN_VALUE;	/*change ReturnValue you nend*/
	//szReturnValue = "Upgrading firmware, don't config wireless";
	

	ButtonInfo.bTestOK = false;
	ButtonInfo.szReturnInfo = szReturnValue;
	ButtonInfo.szWindowName = szWindowName;

	iRty = 3;
	while(iRty)
	{
		iRty--;
		AfxBeginThread( AutoPressButton , &ButtonInfo);
		MessageBox(szText , szWindowName ,NULL);
		if(!ButtonInfo.bTestOK)
		{
			continue;
		}
		else
		{
			break;
		}
	}
	if(!ButtonInfo.bTestOK)
	{
		SendSfisResultAndShowFail(BC40);
		return false;	
	}

	/*button 3*/
	szText = "請按一下Reset按鈕,持續一秒鐘!!!";	/*change Text you nend*/
	szWindowName = "Reset Button Check";		/*change WindowName code you nend*/
	szReturnValue = RST_BTN_VALUE;;	/*change ReturnValue you nend*/
	//szReturnValue = "Upgrading firmware, don't reboot";

	ButtonInfo.bTestOK = false;
	ButtonInfo.szReturnInfo = szReturnValue;
	ButtonInfo.szWindowName = szWindowName;

	iRty = 3;
	while(iRty)
	{
		iRty--;
		AfxBeginThread( AutoPressButton , &ButtonInfo);
		MessageBox(szText , szWindowName ,NULL);
		if(!ButtonInfo.bTestOK)
		{
			continue;
		}
		else
		{
			break;
		}
	}
	if(!ButtonInfo.bTestOK)
	{
		SendSfisResultAndShowFail(BC20);
		return false;	
	}	
	
	Sleep(500);

	iButtonRelaseState = CheckButtonRelease();
	if( iButtonRelaseState != 0 )
	{
		if(iButtonRelaseState == 1)//reset button fail
		{
			SendSfisResultAndShowFail(BC22);
			return false;
		}
		else if(iButtonRelaseState == 2)//WPS button fail
		{
			SendSfisResultAndShowFail(BC32);
			return false;
		}
		else if(iButtonRelaseState == 3)//WIFI button fail
		{
			SendSfisResultAndShowFail(BC42);
			return false;
		}
	}

	return true;
}
bool CRunInfo::DutBurnSn(void)
{
	// No sfis on, no write action, return directly
	if(!GetIsSfis() && (0 != strcmp(m_IniFileInfo.szAllowBurn, "1")))
	{
		return true;
	}

	DisplayRunTimeInfo("---Start burn CN---");
	char    szBurnCNCmd[MINBUFSIZE]  = "";
	sprintf_s(szBurnCNCmd,MINBUFSIZE,"burnsn %s",m_strSSN);
	
	if(SendDutCommand(szBurnCNCmd,"burnsn OK",8000))
	{
		return true;
	}

	return false;
}

bool CRunInfo::DutBurnMac(void)
{
	// No sfis on, no write action, return directly
	if(!GetIsSfis() && (0 != strcmp(m_IniFileInfo.szAllowBurn, "1")))
	{
		return true;
	}

	DisplayRunTimeInfo("---Start burn ethermac---");
	char  szBurnMacCmd[MINBUFSIZE]  = "";

	sprintf_s(szBurnMacCmd,MINBUFSIZE,"burnethermac %s",m_strMAC);
	SendDutCommand("\n","#",500);
	if(SendDutCommand(szBurnMacCmd,"BurnEtherMAC OK",8000))
	{
		return true;
	}

	return false;
}

bool CRunInfo::DutBurnPin(void)
{
	// No sfis on, no write action, return directly
	if(!GetIsSfis() && (0 != strcmp(m_IniFileInfo.szAllowBurn, "1")))
	{
		return true;
	}

	DisplayRunTimeInfo("---Start burn Pincode---");
	char    szBurnPincodeCmd[MINBUFSIZE]  = "";
	sprintf_s(szBurnPincodeCmd,MINBUFSIZE,"burnpin %s",m_strPincode);

	if(SendDutCommand(szBurnPincodeCmd,"burnpin OK",8000))
	{
		return true;
	}

	return false;
}

bool CRunInfo::DutBurnRegion(void)
{
	// No sfis on, no write action, return directly
	if(!GetIsSfis() && (0 != strcmp(m_IniFileInfo.szAllowBurn, "1")) && (0 != strcmp(m_IniFileInfo.szRegionCodeFlag, "1")))
	{
		return true;
	}

	DisplayRunTimeInfo("---Start burn region code---");
	char    szBurnRegionCmd[MINBUFSIZE]  = "";
	sprintf_s(szBurnRegionCmd,MINBUFSIZE,"burnsku %s",m_IniFileInfo.szRegionCode);

	if(SendDutCommand(szBurnRegionCmd,"burnsku OK",8000))
	{
		return true;
	}

	return false;
}


bool  CRunInfo::Write_Code( char* BurnCmd  , char* BurnWhat , char* ReturnValue)
{
	// No sfis on, no write action, return directly
	//In debug mode can burn code 

	if(!GetIsSfis() && (0 != strcmp(m_IniFileInfo.szAllowBurn, "1")))
	{
		return true;//1：don't burn code , return directly
	}
	//if any value is NULL,return directly
	if(BurnCmd == NULL)
	{
		return true;//1：don't burn code , return directly
	}
	if(BurnWhat == NULL)
	{
		return true;//1：don't burn code , return directly
	}
	if(ReturnValue == NULL)
	{
		return true;//1：don't burn code , return directly
	}
	char szBuf[MINBUFSIZE] = "";
	
	sprintf_s(szBuf,MINBUFSIZE,"*****Use %s burn %s*****", BurnCmd , BurnWhat);

	DisplayRunTimeInfo(szBuf);

	char szBurnCmd[MINBUFSIZE]  = "";

	sprintf_s(szBurnCmd,MINBUFSIZE,"%s %s", BurnCmd , BurnWhat);

	if(!SendDutCommand(szBurnCmd , ReturnValue , 10000))
	{
		return false;//2.BurnCode fail
	}

	return true;
}

bool CRunInfo::DutCheckLedAll(void)
{

	if(!SendDutCommand("leddown","#",5000)) return false;
	if(!SendDutCommand("extled set 0 1;extled set 2 1;extled set 5 1;extled set 6 1;extled set 7 1;extled set 4 1;gpio 15 0","#",2000))
	{
		return false;
	}
	
	if(AfxMessageBox("測試PASS請檢查燈是否全亮 ALL lighted?",MB_YESNO) == IDNO)
	{
		
		return false;
	}

	return true;
}

/******don't test WAN LED in PT2 station******/
bool CRunInfo::DutCheckLed_PT2(void)
{
	/*
	true	:pass
	false	:failed
	*/
	if(!SendDutCommand("leddown;ledup","#",5000))
	{
		return false;
	}
	
	if(AfxMessageBox("請檢查產品背面中間位置2個LED亮白色?\n",MB_YESNO) == IDNO)
	{		
		SendSfisResultAndShowFail(LD00);
		return false;
	}

	if(AfxMessageBox("請檢查wireless和USB燈<藍色>\n其餘所有燈(2個)<綠色>?\nwireless and USB lighted Blue,other 2 LED lighted Green?\n",MB_YESNO) == IDNO)//change text you need
	{		
		SendSfisResultAndShowFail(LD00);
		return false;
	}
	return true;
}
bool CRunInfo::DutCheckLed(void)
{
	/*
	true	:pass
	false	:failed
			LD00
			LD20
			LD30
			LD40
	*/

	if(!SendDutCommand("ledup","#",5000))
	{
		return false;
	}
	if(AfxMessageBox("請檢查wireless和USB燈<藍色>\n其餘所有燈(2個)<綠色>?\nwireless and USB lighted Blue,other 2 LED lighted Green?\n",MB_YESNO) == IDNO)//change text you need
	{		
		SendSfisResultAndShowFail(LD00);
		return false;
	}
	//Case 2
	if(!SendDutCommand("wanled stop;wanled 1;wanled start;","#",5000))
	{
		
		return false;
	}	
	if(AfxMessageBox("請檢查WAN燈<橙色>?\n Please check LED colors:\n WAN LED<Amber>?\n",MB_YESNO) == IDNO)
	{		
		SendSfisResultAndShowFail(LD00);
		return false;
	}

	if(AfxMessageBox("請檢查前面板\"NETGEAR\"logo是否均勻亮白色?\n",MB_YESNO) == IDNO)
	{		
		SendSfisResultAndShowFail(LD00);
		return false;
	}


	return true;
}

bool CRunInfo::DutFixWanSetting(void)
{
	ShowMsg("***********Set WAN Port ***************");

	//if(!SendDutCommand("nvram set wan0_ipaddr=111.111.111.111;nvram set wan0_gateway=111.111.111.1;nvram set wan0_netmask=255.255.255.0","#",5000))	
	//	return false;
	if(!SendDutCommand("nvram set wan_ipaddr=111.111.111.111;nvram set wan_gateway=111.111.111.1;nvram set wan_netmask=255.255.255.0","#",5000))
		return false;	
	if(!SendDutCommand("nvram set wan0_dns1=111.111.111.1;nvram set wan_dns1=111.111.111.1;nvram set static0_ipaddr=111.111.111.111","#",5000))
		return false;	
	if(!SendDutCommand("nvram set static0_netmask=255.255.255.0;nvram set static0_gateway=111.111.111.1;nvram set wan0_proto=static","#",5000))
		return false;	
	if(!SendDutCommand("nvram set wan_proto=static;nvram set wla_wlanstate=Disable","#",5000))
		return false;	
	if(!SendDutCommand("nvram commit","#",8000))
		return false;	

	return true;
}

bool CRunInfo::DutTestLockCode(void)
{
	// If no lockcode need, return true directly
	if(0 != strcmp(m_IniFileInfo.szLockCode,"1"))
		return true;

	// lock
	ShowMsg("***************Lock Code****************");
	CString tempIMEI = szIMEIFromDUT;
	tempIMEI.Trim();
	char LockCode[12]="";
	GetLockCode(tempIMEI.GetBuffer(tempIMEI.GetLength()),LockCode);

	m_strLockCode="";
	m_strLockCode=LockCode;
	m_strLockCode.Trim();

	char szTemp[200] = "";
	sprintf_s(szTemp,200,"wwan -ph1 %s",m_strLockCode.Trim());
	if(!SendDutCommand(szTemp,"Lock status: 1",10000))
	{
		if(!SendDutCommand(szTemp,"Lock status: 1",10000))
		{
			return false;
		}
	}

	// unlock
	ShowMsg("**************unLock code*****************");
	sprintf_s(szTemp,200,"wwan -ph0 %s",m_strLockCode.Trim());
	if(!SendDutCommand(szTemp,"Lock status: 0",10000))
	{
		if(!SendDutCommand(szTemp,"Lock status: 0",10000))
		{
			return false;
		}
	}			

	// lock again
	SendDutCommand("\n","#",20);
	ShowMsg("**************Lock code*****************");		
	sprintf_s(szTemp,200,"wwan -ph1 %s",m_strLockCode.Trim());
	if(!SendDutCommand(szTemp,"Lock status: 1",10000))
	{
		if(!SendDutCommand(szTemp,"Lock status:1",10000))
		{
			return false;
		}
	}

	sprintf_s(szTemp,200,"nvram set wsy=%s",m_strLockCode.Trim());
	if(!SendDutCommand(szTemp,"#",1000))
	{
		return false;
	}

	m_strRecordTestData  +=  m_strLockCode.Trim();
	m_strRecordTestData  +=  "\t";

	return true;
}

bool CRunInfo::DutGetIMEI(void)
{
	ShowMsg("************Get IMEI *******************");
	char szTemp[128]="";
	char SepsIMEI[]     = "";
	char *tokenIMEI;
	char *ContextIMEI ;  
	memset(szIMEIFromDUT,0,20);

	if(!SendDutCommand("nvram get wwan_runtime_imei","#",2500))
	{
		return false;
	}
	tokenIMEI = strtok_s(m_strStoreEthernetData.GetBuffer(m_strStoreEthernetData.GetLength()),SepsIMEI,&ContextIMEI);
	tokenIMEI = _tcstok_s(NULL,SepsIMEI,&ContextIMEI);
	strncpy_s(szIMEIFromDUT, MINBUFSIZE, tokenIMEI, 15);
	if(strlen(szIMEIFromDUT) != 15)
		return false;

	m_strRecordTestData  +=  szIMEIFromDUT;
	m_strRecordTestData  +=  "\t";
	m_strimei="";
    m_strimei=szIMEIFromDUT;

	return true;
}

bool CRunInfo::DutClearSSID(void)
{
	ShowMsg("*************Clear SSID**********************");
	if(!SendDutCommand("nvram set wla_ssid=""","#",5000)) return false;
	if(!SendDutCommand("nvram set wla_passphrase=""","#",5000)) return false;
	if(!SendDutCommand("nvram commit","#",4000)) return false;

	return true;
}

bool CRunInfo::DutCheckSimCard(void)
{
	if(!SendDutCommand("sim","inserted",5000))
	{
		if(IDOK == AfxMessageBox("請確認SIM是否有插好PLS check the SIM card insert status !!!"))
		{
			if(!SendDutCommand("sim","inserted",5000))
			{
				return false;
			}
		}
	}

	return true;
}

bool CRunInfo::DutCheckSimRW(void)
{
	if(!DutCheckSimCard())
	{
		return false;
	}

	if(!SendDutCommand("nvram get wwan_SIM_rw","OK",5000))
	{
		return false;
	}
	
	return true;
}

bool CRunInfo::DutCheck3gModelType(void)
{
	if(!SendDutCommand("nvram get wwan_runtime_modem_type",m_IniFileInfo.szModuleInfor,5000))
	{
		return false;
	}

	return true;
}

bool CRunInfo::DutCheck3gModuleVer(void)
{
	if(!SendDutCommand("nvram get wwan_runtime_fw_ver",m_IniFileInfo.szModuleVersion,5000))
	{
		return false;
	}

	return true;
}
bool CRunInfo::DutDetect3gModule(void)
{

	ShowMsg("**********Check 3G Module Info*****************");
	SendDutCommand("killall heartbeat;killall wwan;killall queryusb","#",5000);
	SendDutCommand("killall cald;killall pppd","#",5000);
	SendDutCommand("nvram set wwan_pin_code=0000","#",5000);	// for sim card read. write

	if(SendDutCommand("3gmodule","Modem detected done",20000))
	{
		return true;
	}

	return false;
}

int CRunInfo::DutTestRssi(void)
{
	// if not test, return true directly
	if(0 != strcmp(m_IniFileInfo.szStoreTestRssi,"1"))
	{
		return -1;
	}
	
	char sep[]=":";
	char *pRet, *pNext;

	ShowMsg("*********************Rssi Test(Main Antenna out side)********************************");
	char rssi_token[5120]="";
	float d_RssiValue = 0;

	if(!SendDutCommand("\n","#",1000))	return 0;
	if(!SendDutCommand("cald","#",1000))	return 0;
	if(!SendDutCommand("queryusb","#",1000))	return 0;
	if(!SendDutCommand("killall wwan","#",1000))	return 0;
	if(!SendDutCommand("killall heartbeat","#",1000))	return 0;
	if(!SendDutCommand("\n","#",1000))	return 0;

	// change to main attenna with external antenna
	if(!SendDutCommand("gpio 0 0","#",1000))	return 0;
	if(!SendDutCommand("\n","#",1000))	return 0;
	if(!SendDutCommand("wwan -rx0","Init done",20000))	
	{
		// the following command is to solve initialize module fail issue
		if(!SendDutCommand("","#",1000))	return 0;
		if(SendDutCommand("gpio 5 0","#",1000))
		{
			Sleep(1000);
		}
		else
		{
			return 0;
		}
	
		if(SendDutCommand("gpio 5 1","#",1000))
		{
			Sleep(6000);
		}
		else
		{
			return 0;
		}

		if(!SendDutCommand("wwan -rx0","Init done",20000))
		{
			return 0;
		}
	}
	// start to get rssi, here need optimization
	if(!SendDutCommand("wwan -rx1","Main:",20000))	
	{
		if(!SendDutCommand("wwan -rx1","Main:",20000))
		{
			return 0;
		}
	}

	strcpy_s(rssi_token,sizeof(rssi_token),m_strStoreEthernetData);
	char *pFindRssi = strstr(rssi_token,"Main:");
	pRet = strtok_s(pFindRssi,sep,&pNext);
	pRet = strtok_s(NULL,sep,&pNext);

	char szRssi[24]="";
	strcpy_s(szRssi, MINBUFSIZE, pRet);
	d_RssiValue = (float)atof(szRssi);
	memset(rssi_token,0,MINBUFSIZE);  
	sprintf_s(szRssiValue,MINBUFSIZE,"%.2f(RSSI)",d_RssiValue);

    m_strMainrssi="";
	m_strMainrssi=szRssi;
	m_strRecordTestData += szRssiValue;
	m_strRecordTestData  +=  "\t";

	m_strStoreEthernetData.Empty();
	if(d_RssiValue<m_IniFileInfo.MURssiSpec || d_RssiValue>m_IniFileInfo.MDRssiSpec)
	{
		return 1;
	}	

	// remove external antenna and check rssi
	ShowMsg("*****Rssi Test(Without Main Antenna out side)******");
	AfxMessageBox("請打開屏蔽箱線擰下天線，確認屏蔽箱關閉，再點擊此按鈕!!!Open shielding box,take off antenna,and then shut down!!!");
	d_RssiValue=0.0;

	if(!SendDutCommand("wwan -rx1","Main:",20000))
	{
		if(!SendDutCommand("wwan -rx1","Main:",20000))
		{
			return 0;
		}
	}

	strcpy_s(rssi_token,sizeof(rssi_token),m_strStoreEthernetData);
	pFindRssi = strstr(rssi_token,"Main:");
	pRet = strtok_s(pFindRssi,sep,&pNext);
	pRet = strtok_s(NULL,sep,&pNext);	
	memset (szRssi,0,sizeof(szRssi));
	strcpy_s(szRssi, MINBUFSIZE, pRet);

	d_RssiValue = (float)atof(szRssi);
	memset(rssi_token,0,MINBUFSIZE);  

	CmpRsi=0.0;		
	CmpRsi=d_RssiValue;	

    m_strwizoutrssi="";
    m_strwizoutrssi=szRssi;
	sprintf_s(szRssiValue,MINBUFSIZE,"%.2f(RSSI)",d_RssiValue);
	m_strRecordTestData += szRssiValue;
	m_strRecordTestData  +=  "\t";	

	m_strStoreEthernetData.Empty();	

	if(d_RssiValue<m_IniFileInfo.NURssiSpec || d_RssiValue>m_IniFileInfo.NDRssiSpec)
	{
		return 1;
	}

	// start to test anx antenna 	
	ShowMsg("*****Rssi Test(Aux Antenna)******");
	if(!SendDutCommand("gpio 0 1","#",1000))	return 0;
	if(!SendDutCommand("wwan -rx2","Aux:",20000))	return 0;

	strcpy_s(rssi_token,sizeof(rssi_token),m_strStoreEthernetData);
	pFindRssi = strstr(rssi_token,"Aux:");
	pRet = strtok_s(pFindRssi,sep,&pNext);
	pRet = strtok_s(NULL,sep,&pNext);	
	memset (szRssi,0,sizeof(szRssi));
	strcpy_s(szRssi, MINBUFSIZE, pRet);

	d_RssiValue = (float)atof(szRssi);
	m_strauxrssi="";
	m_strauxrssi=szRssi;
	memset(rssi_token,0,MINBUFSIZE);    
	sprintf_s(szRssiValue,MINBUFSIZE,"%.2f(RSSI)",d_RssiValue);

	m_strRecordTestData += szRssiValue;
	m_strRecordTestData  +=  "\t";		
	m_strStoreEthernetData.Empty();
	if(d_RssiValue<m_IniFileInfo.MURssiSpec || d_RssiValue>m_IniFileInfo.MDRssiSpec)
	{
		return 3;
	}	

	// Test main printed antenna 
	ShowMsg("*****Rssi Test(inside Main Antenna)******");
	if(!SendDutCommand("wwan -rx1","Main:",20000))	return 0;

	strcpy_s(rssi_token,sizeof(rssi_token), m_strStoreEthernetData);
	pFindRssi = strstr(rssi_token,"Main:");
	pRet = strtok_s(pFindRssi,sep,&pNext);
	pRet = strtok_s(NULL,sep,&pNext);	

	memset(szRssi,0,sizeof(szRssi));
	strncpy_s(szRssi, MINBUFSIZE, pRet, 6);
	d_RssiValue = (float)atof(szRssi);

	memset(rssi_token,0,MINBUFSIZE); 
	char soi[50]="111111";
	CmpRso=d_RssiValue-CmpRsi;
	sprintf_s(soi,MINBUFSIZE,"RSSI delta: %.2f(RSSI)",CmpRso);
	ShowMsg(soi);
	sprintf_s(szRssiValue,MINBUFSIZE,"%.2f(RSSI)",d_RssiValue);
	m_strinsidrssi="";

	m_strRecordTestData += szRssiValue;
	m_strRecordTestData  +=  "\t";		
	m_strStoreEthernetData.Empty();

	if( d_RssiValue<m_IniFileInfo.MURssiSpec || d_RssiValue>m_IniFileInfo.MDRssiSpec )
	{
		return 2;
	}

	return -1;
}

int CRunInfo::DutTestWiFiThroughput_1(void)
{
	// if test channel number from ini file is 1, read channel from UI
	// if test channel number is 3, fix channel to 1, 6, 11

	// if no throughput test, return directly
	ShowMsg("***************Test WiFi Throughput****************");
	int iRst=-1;	

	char szSetChannelCommand[MINBUFSIZE]  = "";

	//2G
	char szBuf_1[256] = "";
	sprintf_s(szBuf_1 , 256 , "m_IniFileInfo.nChannelNum_2G_1 = %d " , m_IniFileInfo.nChannelNum_2G_1);
	DisplayRunTimeInfo(szBuf_1) ;

	if(m_IniFileInfo.nChannelNum_2G_1 == 0)
	{
		// no test channel, skip
		//return -1; 
		//2.4G needn't ship,if just need test 5G,ship will not test
		ShowMsg("At test config file:TestInformation.ini find 2.4G WiFi throughput test channel number=0!");
	}
	else if (m_IniFileInfo.nChannelNum_2G_1 == 1)
	{
		ShowMsg("At test config file:TestInformation.ini find 2.4G WiFi throughput test channel number=1!");
		// run random channel from UI
		((CEdit*)GetDlgItem(IDC_STATIC_2G_CHANNEL2))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->EnableWindow(false);
		CString m_wifiChannel;
		GetDlgItemText(IDC_EDIT_2G_CHANNEL1,m_wifiChannel);
		int iChannel = atoi(m_wifiChannel.GetBuffer(m_wifiChannel.GetLength()));

		iRst = RunThroughputTest(iChannel);
		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel, iRst);
		DisplayRunTimeInfo(szBuffer) ;

		SendDutCommand("","#",5000);
		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}
	}
	else if (m_IniFileInfo.nChannelNum_2G_1 == 2)
	{
		ShowMsg("At test config file:TestInformation.ini find 2.4G WiFi throughput test channel number=2!");
		// run random channel from UI
		//CHANNEL 1
		CString m_wifiChannel_1;
		GetDlgItemText(IDC_EDIT_2G_CHANNEL1,m_wifiChannel_1);
		int iChannel_1 = atoi(m_wifiChannel_1.GetBuffer(m_wifiChannel_1.GetLength()));

		iRst = RunThroughputTest(iChannel_1);

		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel_1, iRst);
		DisplayRunTimeInfo(szBuffer) ;
		
		SendDutCommand("","#",5000);
		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}
		//CHANNEL 2
		CString m_wifiChannel_2;
		GetDlgItemText(IDC_EDIT_2G_CHANNEL2,m_wifiChannel_2);
		int iChannel_2 = atoi(m_wifiChannel_2.GetBuffer(m_wifiChannel_2.GetLength()));

		iRst = RunThroughputTest(iChannel_2);

		
		memset(szBuffer , 0 , 256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel_2, iRst);
		DisplayRunTimeInfo(szBuffer) ;
		
		SendDutCommand("","#",5000);
		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}
	}
	else if(m_IniFileInfo.nChannelNum_2G_1 == 3)
	{
		ShowMsg("At test config file:TestInformation.ini find 2.4G WiFi throughput test channel number=3!");
		// run fixed channel

		((CEdit*)GetDlgItem(IDC_STATIC_2G_CHANNEL1))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_STATIC_2G_CHANNEL2))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->EnableWindow(false);

		iRst = RunThroughputTest(3);
		
		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(3) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;

		SendDutCommand("","#",5000);
		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}

		iRst = RunThroughputTest(6);

		memset(szBuffer , 0 , 256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(6) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;

		SendDutCommand("","#",5000);
		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{	
			return iRst;
		}
		
		iRst = RunThroughputTest(9);

		memset(szBuffer , 0 , 256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(9) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;

		SendDutCommand("","#",5000);
		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{	
			return iRst;
		}	
	}	

	//5G
	char szBuf_2[256] = "";
	sprintf_s(szBuf_2 , 256 , "m_IniFileInfo.nChannelNum_5G_1 = %d " , m_IniFileInfo.nChannelNum_5G_1);
	DisplayRunTimeInfo(szBuf_2) ;

	if(m_IniFileInfo.nChannelNum_5G_1 == 0)
	{
		ShowMsg("At test config file:TestInformation.ini find 5G WiFi throughput test channel number=0!");
		
		if(m_IniFileInfo.nChannelNum_2G_1 == 0)
		{
			return -1; // no test channel, skip
		}
	}
	else if (m_IniFileInfo.nChannelNum_5G_1 == 1)
	{
		ShowMsg("At test config file:TestInformation.ini find 5G WiFi throughput test channel number=1!");
		// run random channel from UI
		((CEdit*)GetDlgItem(IDC_STATIC_5G_CHANNEL2))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_5G_CHANNEL2))->EnableWindow(false);
		CString m_wifiChannel;
		GetDlgItemText(IDC_EDIT_5G_CHANNEL1,m_wifiChannel);
		int iChannel = atoi(m_wifiChannel.GetBuffer(m_wifiChannel.GetLength()));
		
		iRst = RunThroughputTest(iChannel);
		
		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel, iRst);
		DisplayRunTimeInfo(szBuffer) ;

		SendDutCommand("","#",5000);
		SendDutCommand("wl -i eth2 down","#",5000);	
		if(iRst != -1)
		{			
			return iRst;
		}
	}
	else if (m_IniFileInfo.nChannelNum_5G_1 == 2)
	{
		ShowMsg("At test config file:TestInformation.ini find 5G WiFi throughput test channel number=2!");
		// run random channel from UI
		//CHANNEL 1
		CString m_wifiChannel_1;
		GetDlgItemText(IDC_EDIT_5G_CHANNEL1,m_wifiChannel_1);
		int iChannel_1 = atoi(m_wifiChannel_1.GetBuffer(m_wifiChannel_1.GetLength()));

		iRst = RunThroughputTest(iChannel_1);

		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel_1, iRst);
		DisplayRunTimeInfo(szBuffer) ;

		SendDutCommand("","#",5000);
		SendDutCommand("wl -i eth2 down","#",5000);	
		if(iRst != -1)
		{
			return iRst;
		}
		//CHANNEL 2
		CString m_wifiChannel_2;
		GetDlgItemText(IDC_EDIT_5G_CHANNEL2,m_wifiChannel_2);
		int iChannel_2 = atoi(m_wifiChannel_2.GetBuffer(m_wifiChannel_2.GetLength()));

		iRst = RunThroughputTest(iChannel_2);

		memset(szBuffer , 0 ,256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel_2, iRst);
		DisplayRunTimeInfo(szBuffer) ;
		
		SendDutCommand("","#",5000);
		SendDutCommand("wl -i eth2 down","#",5000);	

		if(iRst != -1)
		{
			return iRst;
		}
	}
	else if(m_IniFileInfo.nChannelNum_5G_1 == 3)
	{
		ShowMsg("At test config file:TestInformation.ini find 5G WiFi throughput test channel number=3!");
		// run fixed channel

		((CEdit*)GetDlgItem(IDC_STATIC_5G_CHANNEL1))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_5G_CHANNEL1))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_STATIC_5G_CHANNEL2))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_5G_CHANNEL2))->EnableWindow(false);

		iRst = RunThroughputTest(36);

		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(36) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;

		SendDutCommand("","#",5000);
		SendDutCommand("wl -i eth2 down","#",5000);	
		if(iRst != -1)
		{	
			return iRst;
		}

		iRst = RunThroughputTest(44);

		memset(szBuffer , 0 ,256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(44) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;

		SendDutCommand("","#",5000);
		SendDutCommand("wl -i eth2 down","#",5000);	
		if(iRst != -1)
		{	
			return iRst;
		}
		
		iRst = RunThroughputTest(157);

		memset(szBuffer , 0 ,256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(157) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;

		SendDutCommand("","#",5000);
		SendDutCommand("wl -i eth2 down","#",5000);	
		if(iRst != -1)
		{
			return iRst;
		}	
	}
	SendDutCommand("","#",5000);
	return -1;
}

int CRunInfo::DutTestWiFiThroughput_2(void)
{
	// if test channel number from ini file is 1, read channel from UI
	// if test channel number is 3, fix channel to 1, 6, 11

	// if no throughput test, return directly
	ShowMsg("***************Test WiFi Throughput****************");
	int iRst=-1;

	char szSetChannelCommand[MINBUFSIZE]  = "";

	//2G
	char szBuf_1[256] = "";
	sprintf_s(szBuf_1 , 256 , "m_IniFileInfo.nChannelNum_2G_2 = %d " , m_IniFileInfo.nChannelNum_2G_2);
	DisplayRunTimeInfo(szBuf_1) ;

	if(m_IniFileInfo.nChannelNum_2G_2 == 0)
	{
		// no test channel, skip
		//return -1; 
		//2.4G needn't ship,if just need test 5G,ship will not test
		ShowMsg("At test config file:TestInformation.ini find 2.4G WiFi throughput test channel number=0!");
	}
	else if (m_IniFileInfo.nChannelNum_2G_2 == 1)
	{
		ShowMsg("At test config file:TestInformation.ini find 2.4G WiFi throughput test channel number=1!");
		// run random channel from UI
		((CEdit*)GetDlgItem(IDC_STATIC_2G_CHANNEL2))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->EnableWindow(false);
		CString m_wifiChannel;
		GetDlgItemText(IDC_EDIT_2G_CHANNEL1,m_wifiChannel);
		int iChannel = atoi(m_wifiChannel.GetBuffer(m_wifiChannel.GetLength()));

		iRst = RunThroughputTest(iChannel , 2);
		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel, iRst);
		DisplayRunTimeInfo(szBuffer) ;

		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{			
			return iRst;
		}
	}
	else if (m_IniFileInfo.nChannelNum_2G_2 == 2)
	{
		ShowMsg("At test config file:TestInformation.ini find 2.4G WiFi throughput test channel number=2!");
		// run random channel from UI
		//CHANNEL 1
		CString m_wifiChannel_1;
		GetDlgItemText(IDC_EDIT_2G_CHANNEL1,m_wifiChannel_1);
		int iChannel_1 = atoi(m_wifiChannel_1.GetBuffer(m_wifiChannel_1.GetLength()));

		iRst = RunThroughputTest(iChannel_1 , 2);

		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel_1, iRst);
		DisplayRunTimeInfo(szBuffer) ;

		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}
		//CHANNEL 2
		CString m_wifiChannel_2;
		GetDlgItemText(IDC_EDIT_2G_CHANNEL2,m_wifiChannel_2);
		int iChannel_2 = atoi(m_wifiChannel_2.GetBuffer(m_wifiChannel_2.GetLength()));

		iRst = RunThroughputTest(iChannel_2 , 2);

		
		memset(szBuffer , 0 , 256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel_2, iRst);
		DisplayRunTimeInfo(szBuffer) ;

		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}
	}
	else if(m_IniFileInfo.nChannelNum_2G_2 == 3)
	{
		ShowMsg("At test config file:TestInformation.ini find 2.4G WiFi throughput test channel number=3!");
		// run fixed channel

		((CEdit*)GetDlgItem(IDC_STATIC_2G_CHANNEL1))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_2G_CHANNEL1))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_STATIC_2G_CHANNEL2))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_2G_CHANNEL2))->EnableWindow(false);

		iRst = RunThroughputTest(3 , 2);
		
		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(3) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;

		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}

		iRst = RunThroughputTest(6 , 2);

		memset(szBuffer , 0 , 256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(6) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;

		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}
		
		iRst = RunThroughputTest(9 , 2);

		memset(szBuffer , 0 , 256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(9) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;

		if(!SendDutCommand("wl down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}	
	}	

	//5G
	char szBuf_2[256] = "";
	sprintf_s(szBuf_2 , 256 , "m_IniFileInfo.nChannelNum_5G_2 = %d " , m_IniFileInfo.nChannelNum_5G_2);
	DisplayRunTimeInfo(szBuf_2) ;

	if(m_IniFileInfo.nChannelNum_5G_2 == 0)
	{
		ShowMsg("At test config file:TestInformation.ini find 5G WiFi throughput test channel number!");
		
		if(m_IniFileInfo.nChannelNum_2G_2 == 0)
		{
			return -1; // no test channel, skip
		}
	}
	else if (m_IniFileInfo.nChannelNum_5G_2 == 1)
	{
		ShowMsg("At test config file:TestInformation.ini find 5G WiFi throughput test channel number=1!");
		// run random channel from UI
		((CEdit*)GetDlgItem(IDC_STATIC_5G_CHANNEL2))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_5G_CHANNEL2))->EnableWindow(false);
		CString m_wifiChannel;
		GetDlgItemText(IDC_EDIT_5G_CHANNEL1,m_wifiChannel);
		int iChannel = atoi(m_wifiChannel.GetBuffer(m_wifiChannel.GetLength()));
		
		iRst = RunThroughputTest(iChannel , 2);
		
		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel, iRst);
		DisplayRunTimeInfo(szBuffer) ;

		if(!SendDutCommand("wl -i eth2 down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{			
			return iRst;
		}
	}
	else if (m_IniFileInfo.nChannelNum_5G_2 == 2)
	{
		ShowMsg("At test config file:TestInformation.ini find 5G WiFi throughput test channel number=2!");
		// run random channel from UI
		//CHANNEL 1
		CString m_wifiChannel_1;
		GetDlgItemText(IDC_EDIT_5G_CHANNEL1,m_wifiChannel_1);
		int iChannel_1 = atoi(m_wifiChannel_1.GetBuffer(m_wifiChannel_1.GetLength()));

		iRst = RunThroughputTest(iChannel_1 , 2);

		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel_1, iRst);
		DisplayRunTimeInfo(szBuffer) ;

		if(!SendDutCommand("wl -i eth2 down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}
		//CHANNEL 2
		CString m_wifiChannel_2;
		GetDlgItemText(IDC_EDIT_5G_CHANNEL2,m_wifiChannel_2);
		int iChannel_2 = atoi(m_wifiChannel_2.GetBuffer(m_wifiChannel_2.GetLength()));

		iRst = RunThroughputTest(iChannel_2 , 2);

		memset(szBuffer , 0 ,256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(%d) return value = %d " ,iChannel_2, iRst);
		DisplayRunTimeInfo(szBuffer) ;

		if(!SendDutCommand("wl -i eth2 down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}
	}
	else if(m_IniFileInfo.nChannelNum_5G_2 == 3)
	{
		ShowMsg("At test config file:TestInformation.ini find 5G WiFi throughput test channel number=3!");
		// run fixed channel

		((CEdit*)GetDlgItem(IDC_STATIC_5G_CHANNEL1))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_5G_CHANNEL1))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_STATIC_5G_CHANNEL2))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_5G_CHANNEL2))->EnableWindow(false);

		iRst = RunThroughputTest(38 , 2);

		char szBuffer[256] = "";
		sprintf_s(szBuffer , 256 , "RunThroughputTest(38) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;
		
		if(!SendDutCommand("wl -i eth2 down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{	
			return iRst;
		}

		iRst = RunThroughputTest(44 , 2);

		memset(szBuffer , 0 ,256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(44) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;

		if(!SendDutCommand("wl -i eth2 down","#",5000))
		{	
			return 1;
		}
		Sleep(3000);

		if(iRst != -1)
		{
			return iRst;
		}
		
		iRst = RunThroughputTest(157 , 2);

		memset(szBuffer , 0 ,256);
		sprintf_s(szBuffer , 256 , "RunThroughputTest(157) return value = %d " , iRst);
		DisplayRunTimeInfo(szBuffer) ;

		SendDutCommand("wl -i eth2 down","#",5000);	
		if(iRst != -1)
		{
			return iRst;
		}	
	}

	if(!SendDutCommand("wl -i eth2 down","#",5000))
	{	
		return 1;
	}	
	if(!SendDutCommand("wl -i eth2 down","#",5000))
	{	
		return 1;
	}	
	
	SendDutCommand("\n","#",5000);
	return -1;
}


int CRunInfo::RunThroughputTest(int iChannel , int iChipsetIndex)
{
	SendDutCommand("","#",2000);
	double     TXValue = 0;
	double     RXValue = 0;
	double     TXRXValue = 0;
	double     TXValue_Rand = 0;
	double     RXValue_Rand = 0;
	double     TXRXValue_Rand = 0;
	char       szShowTXResult[MINBUFSIZE] = "";
	char       szShowRXResult[MINBUFSIZE] = "";
	char       szShowTXRXResult[MINBUFSIZE] = "";
	char       szCovertValueToCharChannelTX[MINBUFSIZE] ;
	char       szCovertValueToCharChannelRX[MINBUFSIZE] ;
	char       szCovertValueToCharChannelTXRX[MINBUFSIZE] ;
	memset(szCovertValueToCharChannelTX , 0, MINBUFSIZE);
	memset(szCovertValueToCharChannelRX , 0 , MINBUFSIZE);
	memset(szCovertValueToCharChannelTXRX , 0 , MINBUFSIZE);

	char szstrIP[MINBUFSIZE] = "";
	CString szIperfInfo;
	CString strSSID;
	char    szIperfCmd_D[MINBUFSIZE] = "";
	char    szIperfCmd_R[MINBUFSIZE] = "";
	char    szPingGolden[MAXBUFSIZE] = "";

	if(iChannel > 14)
	{
		sprintf_s(szstrIP, MINBUFSIZE, "%s", m_IniFileInfo.sz5GThrotghputGoldenIP);
	}
	else
	{
		sprintf_s(szstrIP, MINBUFSIZE, "%s", m_IniFileInfo.sz2GThrotghputGoldenIP);
	}
	
	sprintf_s(szPingGolden  ,MAXBUFSIZE,"ping %s -t",szstrIP);
	sprintf_s(szIperfCmd_D  ,MINBUFSIZE, m_IniFileInfo.szStoreIperfPara_D, szstrIP);
	sprintf_s(szIperfCmd_R  ,MINBUFSIZE, m_IniFileInfo.szStoreIperfPara_R, szstrIP);
	
	//setting DUT
	int SettingDUTValue = SettingDUT(iChannel , iChipsetIndex);
	if(SettingDUTValue != -1)
	{
		return SettingDUTValue;	
	}


	//DUT Golden server
	if(m_IniFileInfo.nThroughputGoldenFlag == 1)
	{
		int SettingGoldenServerValue = SettingGoldenServer( iChannel );
		if( SettingGoldenServerValue != -1 )
		{
			return SettingGoldenServerValue;
		}
	}


	//Sleep or not
	if(m_IniFileInfo.nSleepOrNo == 1)
	{
		Sleep(m_IniFileInfo.nTimeOut*1000);
	}


// for tx+rx
#if 1
	// for bi-direction throughput test
	if(0 == strcmp(m_IniFileInfo.szStoreTestThroughput,"1") )
	{
		int  dwReTestCount = m_IniFileInfo.nRetryNum ;
		while(dwReTestCount--)
		{
			//send Golden setting command

			//int SettingGoldenServerValue = SettingGoldenServer( iChannel );
			//if( SettingGoldenServerValue != -1 )
			//{
			//	return SettingGoldenServerValue;
			//}

			DeleteSpecifyExe("iperf.exe"); // kill iperf.exe process to make sure the program not hang up.
			szIperfInfo.Empty();
			DisplayRunTimeInfo("-Begin ping golden PC");
			if(!PingSpecifyIP_2(szstrIP, m_IniFileInfo.nPingGoldenCount))
			{	
				if(dwReTestCount)
				{
					continue;
				}
				else
				{
					if(iChannel > 14)//5G
						return 4;	// ping 5G golden failed
					else
						return 5;   // ping 2G golden failed
				}
			}		

			TXValue = 0;
			RXValue = 0;
			DisplayRunTimeInfo("Start TX+RX throughput test");
			
			int iReturn = RunSpecifyExeAndRead(szIperfInfo,szIperfCmd_D,false);  
			DeleteSpecifyExe("iperf.exe");
			if(iReturn == 3)
			{
				return 88;//exceptional in throughput test process 
			}

			GetThroughputValue(szIperfInfo.GetBuffer(szIperfInfo.GetLength()), TXValue, RXValue);		

			TXRXValue_Rand = TXValue + RXValue;
	
			if(iChannel > 14)//5G
			{
				if(m_IniFileInfo.nThroughputFalg5G_Factroy == 1)
				{
					TXRXValue = __RandResult(TXRXValue_Rand, m_IniFileInfo.nTXRXThroughputSpec5G, m_IniFileInfo.nTXRXThroughputSpec5G_Factroy, m_IniFileInfo.nThroughputRange5G_Factroy);
				}
				else
				{
					TXRXValue = TXRXValue_Rand;
				}			
			}
			if(iChannel < 14)//2G
			{
				if(m_IniFileInfo.nThroughputFalg_Factroy == 1)
				{
					TXRXValue = __RandResult(TXRXValue_Rand, m_IniFileInfo.nTXRXThroughputSpec, m_IniFileInfo.nTXRXThroughputSpec_Factroy, m_IniFileInfo.nThroughputRange_Factroy);
				}
				else
				{
					TXRXValue = TXRXValue_Rand;
				}			
			}
			//end

			sprintf_s(szShowTXRXResult, MINBUFSIZE, "Channel %d TX+RX throughput: %.2f Mbit", iChannel, TXRXValue);
			DisplayRunTimeInfo(szShowTXRXResult);
			szIperfInfo.Empty();
			DisplayRunTimeInfo("");


			if( iChipsetIndex == 1)
			{
				if(iChannel > 14)//5G
				{
					if(TXRXValue >= m_IniFileInfo.nTXRXThroughputSpec5G && TXRXValue <= m_IniFileInfo.nTXRXThroughputSpecHigt5G )
					{
						break;
					}
					else if(TXRXValue <= m_IniFileInfo.nTXRXThroughputSpec5G)
					{	
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTXRX, MINBUFSIZE, "%.2f(TXRX)", TXRXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTXRX;
							m_strRecordTestData  +=  "\t";
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,",TestDetaillog,szCovertValueToCharChannelTXRX);
							return 6; //5G TX+RX LOWER THAN SPEC
						}
					}
					else if(TXRXValue >= m_IniFileInfo.nTXRXThroughputSpecHigt5G)
					{	
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTXRX, MINBUFSIZE, "%.2f(TXRX)", TXRXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTXRX;
							m_strRecordTestData  +=  "\t";
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s," , TestDetaillog , szCovertValueToCharChannelTXRX);
							return 7; //5G TX+RX HIGTER THAN SPEC
						}
					}
				}
				else//2.4G
				{
					if(TXRXValue >= m_IniFileInfo.nTXRXThroughputSpec && TXRXValue <= m_IniFileInfo.nTXRXThroughputSpecHigt )
					{
						break;
					}
					else if(TXRXValue <= m_IniFileInfo.nTXRXThroughputSpec)
					{	
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTXRX, MINBUFSIZE, "%.2f(TXRX)", TXRXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTXRX;
							m_strRecordTestData  +=  "\t";
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,",TestDetaillog,szCovertValueToCharChannelTXRX);
							return 8; //2G TX+RX LOWER THAN SPEC
						}
					}
					else if(TXRXValue >= m_IniFileInfo.nTXRXThroughputSpecHigt)
					{	
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTXRX, MINBUFSIZE, "%.2f(TXRX)", TXRXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTXRX;
							m_strRecordTestData  +=  "\t";
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s," , TestDetaillog , szCovertValueToCharChannelTXRX);
							return 9; //2G TX+RX HIGTER THAN SPEC
						}
					}
				}
			}
			else if(iChipsetIndex == 2)
			{		
				if(iChannel > 14)//5G
				{
					if(TXRXValue >= m_IniFileInfo.nTXRXThroughputSpec5G_2 && TXRXValue <= m_IniFileInfo.nTXRXThroughputSpecHigt5G_2 )
					{
						break;
					}
					else if(TXRXValue <= m_IniFileInfo.nTXRXThroughputSpec5G_2)
					{	
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTXRX, MINBUFSIZE, "%.2f(TXRX)", TXRXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTXRX;
							m_strRecordTestData  +=  "\t";
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,",TestDetaillog,szCovertValueToCharChannelTXRX);
							return 6; //5G TX+RX LOWER THAN SPEC
						}
					}
					else if(TXRXValue >= m_IniFileInfo.nTXRXThroughputSpecHigt5G_2)
					{	
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTXRX, MINBUFSIZE, "%.2f(TXRX)", TXRXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTXRX;
							m_strRecordTestData  +=  "\t";
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s," , TestDetaillog , szCovertValueToCharChannelTXRX);
							return 7; //5G TX+RX HIGTER THAN SPEC
						}
					}
				}
				else//2.4G
				{
					if(TXRXValue >= m_IniFileInfo.nTXRXThroughputSpec_2 && TXRXValue <= m_IniFileInfo.nTXRXThroughputSpecHigt_2 )
					{
						break;
					}
					else if(TXRXValue <= m_IniFileInfo.nTXRXThroughputSpec_2)
					{	
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTXRX, MINBUFSIZE, "%.2f(TXRX)", TXRXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTXRX;
							m_strRecordTestData  +=  "\t";
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,",TestDetaillog,szCovertValueToCharChannelTXRX);
							return 8; //2G TX+RX LOWER THAN SPEC
						}
					}
					else if(TXRXValue >= m_IniFileInfo.nTXRXThroughputSpecHigt_2)
					{	
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTXRX, MINBUFSIZE, "%.2f(TXRX)", TXRXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTXRX;
							m_strRecordTestData  +=  "\t";
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s," , TestDetaillog , szCovertValueToCharChannelTXRX);
							return 9; //2G TX+RX HIGTER THAN SPEC
						}
					}
				}
			}
			else
			{
				//do nothing
			}
		}
		sprintf_s(szCovertValueToCharChannelTXRX,MINBUFSIZE,"%.2f(TXRX)",TXRXValue);
		m_strRecordTestData  +=  szCovertValueToCharChannelTXRX;
		m_strRecordTestData  +=  "\t";
		/*collect test data*/
		sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,",TestDetaillog,szCovertValueToCharChannelTXRX);
		/*End*/
		SendDutCommand("\n","#",1000);
	}

#endif
// for tx, rx
#if 1
	if(0 == strcmp(m_IniFileInfo.szStoreTestThroughputSingle,"1"))
	{
		int  dwReTestCount = m_IniFileInfo.nRetryNum ;
		while(dwReTestCount--)
		{
			////send Golden setting command

			//int SettingGoldenServerValue = SettingGoldenServer( iChannel );
			//if( SettingGoldenServerValue != -1 )
			//{
			//	return SettingGoldenServerValue;
			//}

			DeleteSpecifyExe("iperf.exe"); // kill iperf.exe process to make sure the program not hang up.
			szIperfInfo.Empty();
			DisplayRunTimeInfo("Begin ping golden PC");
			if(!PingSpecifyIP_2(szstrIP, m_IniFileInfo.nPingGoldenCount))
			{
				if(dwReTestCount)
				{
					continue;
				}
				else
				{
					if(iChannel > 14)//5G
						return 4;	// ping 5G golden failed
					else
						return 5;   // ping 2G golden failed
				}
			}
			TXValue = 0;
			RXValue = 0;
			DisplayRunTimeInfo("Start TX/RX throughput test");

			RunSpecifyExeAndRead(szIperfInfo,szIperfCmd_R,false);  //modyfy 06.11
			DeleteSpecifyExe("iperf.exe");
			GetThroughputValue(szIperfInfo.GetBuffer(szIperfInfo.GetLength()), TXValue_Rand, RXValue_Rand);

			/************The BUG********************/
			/*
			The function GetThroughputValue() return value is stroage at 'TXValue_Rand' and 'RXValue_Rand',
			but	following the if statement are judge 'TXValue' and 'RXValue',they are still equal zero.
			So function is always return 88(will show 'IP00' error code).			
			*/
			/*************BUG Fixed*****************/
			/*
			Change 'TXValue' parameter to 'TXValue_Rand'.
			Change 'RXValue' parameter to 'RXValue_Rand'.
			*/
			if((TXValue_Rand <= 0) || (RXValue_Rand <= 0))
			{
				return 88;//exceptional in throughput test process 
			}

			//201205010 add
			if(iChannel > 14)//5G
			{
				if(m_IniFileInfo.nThroughputFalg5G_Factroy == 1)
				{
					TXValue = __RandResult(TXValue_Rand, m_IniFileInfo.nTXThroughputSpec5G, m_IniFileInfo.nTXThroughputSpec5G_Factroy, m_IniFileInfo.nThroughputRange5G_Factroy);
					RXValue = __RandResult(RXValue_Rand, m_IniFileInfo.nRXThroughputSpec5G, m_IniFileInfo.nRXThroughputSpec5G_Factroy, m_IniFileInfo.nThroughputRange5G_Factroy);
				}
				else
				{
					TXValue = TXValue_Rand ;
					RXValue = RXValue_Rand ;
					
				}			
			}
			if(iChannel < 14)//2G
			{
				if(m_IniFileInfo.nThroughputFalg_Factroy == 1)
				{
					TXValue = __RandResult(TXValue_Rand, m_IniFileInfo.nTXThroughputSpec, m_IniFileInfo.nTXThroughputSpec_Factroy, m_IniFileInfo.nThroughputRange_Factroy);
					RXValue = __RandResult(RXValue_Rand, m_IniFileInfo.nRXThroughputSpec, m_IniFileInfo.nRXThroughputSpec_Factroy, m_IniFileInfo.nThroughputRange_Factroy);
				}
				else
				{
					TXValue = TXValue_Rand ;
					RXValue = RXValue_Rand ;					
				}			
			}
			//end


			sprintf_s(szShowTXResult, MINBUFSIZE, "Channel %d TX throughput: %.2f Mbit", iChannel, TXValue);
			sprintf_s(szShowRXResult, MINBUFSIZE, "Channel %d RX throughput: %.2f Mbit", iChannel, RXValue);
			DisplayRunTimeInfo(szShowTXResult);
			DisplayRunTimeInfo(szShowRXResult);

			DisplayRunTimeInfo("\n");

			if(iChipsetIndex == 1)
			{
				if(iChannel > 14)//5G
				{
					if(TXValue >= m_IniFileInfo.nTXThroughputSpec5G 
						&& RXValue >= m_IniFileInfo.nRXThroughputSpec5G
						&& TXValue <= m_IniFileInfo.nTXThroughputSpecHigt5G
						&& RXValue <= m_IniFileInfo.nRXThroughputSpecHigt5G)
					{
						break;
					}
					else if(TXValue < m_IniFileInfo.nTXThroughputSpec5G)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTX, MINBUFSIZE, "%.2f(TX)", TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTX;
							m_strRecordTestData  +=  "\t";		
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 10;	// 5G TX LOWER THAN SPEC
						}
					}
					else if(RXValue < m_IniFileInfo.nRXThroughputSpec5G)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelRX,MINBUFSIZE,"%.2f(RX)",TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelRX;
							m_strRecordTestData  +=  "\t";
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 11; 	// 5G RX LOWER THAN SPEC						
						}
					}
					else if(TXValue >= m_IniFileInfo.nTXThroughputSpecHigt
						|| RXValue >= m_IniFileInfo.nRXThroughputSpecHigt)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelRX,MINBUFSIZE,"%.2f(RX)",TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelRX;
							m_strRecordTestData  +=  "\t";
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s,",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 12; 	// 5G TX/RX HIGTER THAN SPEC					
						}
					}
				}
				else
				{
					if(TXValue >= m_IniFileInfo.nTXThroughputSpec 
						&& RXValue >= m_IniFileInfo.nRXThroughputSpec
						&& TXValue <= m_IniFileInfo.nTXThroughputSpecHigt
						&& RXValue <= m_IniFileInfo.nRXThroughputSpecHigt)
					{
						break;
					}
					else if(TXValue < m_IniFileInfo.nTXThroughputSpec)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTX, MINBUFSIZE, "%.2f(TX)", TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTX;
							m_strRecordTestData  +=  "\t";		
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 13;	// 2G TX LOWER THAN SPEC
						}
					}
					else if(RXValue < m_IniFileInfo.nRXThroughputSpec)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelRX,MINBUFSIZE,"%.2f(RX)",TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelRX;
							m_strRecordTestData  +=  "\t";
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 14; 		//2G RX LOWER THAN SPEC						
						}
					}
					else if(TXValue >= m_IniFileInfo.nTXThroughputSpecHigt
						|| RXValue >= m_IniFileInfo.nRXThroughputSpecHigt)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelRX,MINBUFSIZE,"%.2f(RX)",TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelRX;
							m_strRecordTestData  +=  "\t";
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s,",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 15; 		// 2G TX/RX HIGTER THAN SPEC						
						}
					}
				}
			}
			else if(iChipsetIndex == 2)
			{
				if(iChannel > 14)//5G
				{
					if(TXValue >= m_IniFileInfo.nTXThroughputSpec5G_2 
						&& RXValue >= m_IniFileInfo.nRXThroughputSpec5G_2
						&& TXValue <= m_IniFileInfo.nTXThroughputSpecHigt5G_2
						&& RXValue <= m_IniFileInfo.nRXThroughputSpecHigt5G_2)
					{
						break;
					}
					else if(TXValue < m_IniFileInfo.nTXThroughputSpec5G_2)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTX, MINBUFSIZE, "%.2f(TX)", TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTX;
							m_strRecordTestData  +=  "\t";		
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 10;	// 5G TX LOWER THAN SPEC
						}
					}
					else if(RXValue < m_IniFileInfo.nRXThroughputSpec5G_2)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelRX,MINBUFSIZE,"%.2f(RX)",TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelRX;
							m_strRecordTestData  +=  "\t";
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 11; 	// 5G RX LOWER THAN SPEC						
						}
					}
					else if(TXValue >= m_IniFileInfo.nTXThroughputSpecHigt_2
						|| RXValue >= m_IniFileInfo.nRXThroughputSpecHigt_2)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelRX,MINBUFSIZE,"%.2f(RX)",TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelRX;
							m_strRecordTestData  +=  "\t";
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s,",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 12; 	// 5G TX/RX HIGTER THAN SPEC					
						}
					}
				}
				else
				{
					if(TXValue >= m_IniFileInfo.nTXThroughputSpec_2 
						&& RXValue >= m_IniFileInfo.nRXThroughputSpec_2
						&& TXValue <= m_IniFileInfo.nTXThroughputSpecHigt_2
						&& RXValue <= m_IniFileInfo.nRXThroughputSpecHigt_2)
					{
						break;
					}
					else if(TXValue < m_IniFileInfo.nTXThroughputSpec_2)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelTX, MINBUFSIZE, "%.2f(TX)", TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelTX;
							m_strRecordTestData  +=  "\t";		
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 13;	// 2G TX LOWER THAN SPEC
						}
					}
					else if(RXValue < m_IniFileInfo.nRXThroughputSpec_2)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelRX,MINBUFSIZE,"%.2f(RX)",TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelRX;
							m_strRecordTestData  +=  "\t";
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 14; 		//2G RX LOWER THAN SPEC						
						}
					}
					else if(TXValue >= m_IniFileInfo.nTXThroughputSpecHigt_2
						|| RXValue >= m_IniFileInfo.nRXThroughputSpecHigt_2)
					{
						if(dwReTestCount)
							continue;
						else
						{
							sprintf_s(szCovertValueToCharChannelRX,MINBUFSIZE,"%.2f(RX)",TXValue);
							m_strRecordTestData  +=  szCovertValueToCharChannelRX;
							m_strRecordTestData  +=  "\t";
							/*collect test data*/
							sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s,",TestDetaillog,szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
							/*End*/
							return 15; 		// 2G TX/RX HIGTER THAN SPEC						
						}
					}
				}

			}
			else
			{
				//do nothing
			}
		}
		sprintf_s(szCovertValueToCharChannelTX,MINBUFSIZE,"%.2f(TX)",TXValue);
		m_strRecordTestData  +=  szCovertValueToCharChannelTX;
		m_strRecordTestData  +=  "\t";
		sprintf_s(szCovertValueToCharChannelRX,MINBUFSIZE,"%.2f(RX)",RXValue);
		m_strRecordTestData  +=  szCovertValueToCharChannelRX;
		m_strRecordTestData  +=  "\t";

		/*collect test data*/
		sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,%s,", TestDetaillog , szCovertValueToCharChannelTX,szCovertValueToCharChannelRX);
		/*End*/
	}
#endif 
	
	return -1;
}
void CRunInfo::DeleteSpecifyExe(char* exeName)
{
	if(exeName == NULL)
		return;

	HANDLE hProcess = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(INVALID_HANDLE_VALUE == hProcess)
		return;
	BOOL fPOK = Process32First(hProcess,&pe);
	for(; fPOK;fPOK = Process32Next(hProcess,&pe))
	{
		if(strstr(_strlwr(pe.szExeFile),exeName) != NULL)
		{
			HANDLE hPr = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID );
			if( hPr == NULL )
				return;
			else
			{
				TerminateProcess(hPr,0);
			}
		}
	}
	if(INVALID_HANDLE_VALUE != hProcess)
		CloseHandle(hProcess);
	
}

void CRunInfo::InitTestRecordData(void)
{
	m_TestRecordinfo.szRecordTestTime = "000(s)";
	m_TestRecordinfo.szRecordFirmware = "NULL";
	m_TestRecordinfo.szRecordMac = "NULL";
	m_TestRecordinfo.szRecordPcName = "NULL";
	m_TestRecordinfo.szRecordPincode = "NULL";
	m_TestRecordinfo.szRecordSSN = "NULL";
	m_TestRecordinfo.szRecordProductName = "NULL";
	m_TestRecordinfo.szRecordResult = "NULL";
	m_TestRecordinfo.szRecordSN = "0";
	m_TestRecordinfo.szRecordTime = "00:00:00";
	
}

CString CRunInfo::CustomerFirmwareVersion(char*  DutFirmwareVersion)
{
	char    Seps[] = "\r\n";
	char    *token;
	char    *Context;
	token  = strtok_s(DutFirmwareVersion, Seps, &Context);
	while(token != NULL)
	{
		token = strtok_s(NULL, Seps, &Context);

		CString str = token;
		str.Trim();
		if(strstr(str , "/"))
		{
			int nPostion = str.Find("/");
			str.Delete(0 , nPostion + 1);
			str.Replace('/','_');
			str.Trim();
			return str;
		}
		else
		{
			continue;
		}
	}
	return DutFirmwareVersion;
}
bool CRunInfo::DutCheckBoardID(void)
{
	/*
	true	:pass
	false	:failed
	*/
	DisplayRunTimeInfo("---Start check boardid---");

	if(!SendDutCommand("burnboardid","#",5000)) return false;
	if(strstr(m_strStoreEthernetData, m_IniFileInfo.szStoreBoardID ) == NULL || (!strcmp("", m_IniFileInfo.szStoreBoardID)) || strlen(m_IniFileInfo.szStoreBoardID)==0 )
	{
		return false;
	}
	/*collect test data*/
	sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%s,", TestDetaillog , m_IniFileInfo.szStoreBoardID);
	/*End*/

	return true;
}
bool CRunInfo::DutCheckMAC(void)
{
	/*
	true	:pass
	false	:false
	*/
	DisplayRunTimeInfo("---Start check MAC---");

	if(!SendDutCommand("burnethermac","#",5000)) return false;

	char    Seps[]     = "-\n";
	char    *token,*DelSpaceToken;
	char    szMACFromDUT[MINBUFSIZE] = "";
	char    *Context ;
	token = strtok_s(m_strStoreEthernetData.GetBuffer(m_strStoreEthernetData.GetLength()),Seps,&Context);
	while(token!=NULL)
	{
		token = _tcstok_s(NULL,Seps,&Context);
		if(strcmp(token,"WAN mac address ") == 0)
		{
			token = strtok_s(NULL, Seps, &Context);
			char subSeps[] = " ";
			DelSpaceToken = strtok_s(token,subSeps,&Context);
			strncpy_s(szMACFromDUT, MINBUFSIZE, DelSpaceToken, 12);
			break;
		}
	}

	ULONG  lgGetWMACFromDUT = CovertMacToNumber(szMACFromDUT, (int)strlen(szMACFromDUT));
	if(lgGetWMACFromDUT != m_nWanMAC)
	{
		return false;
	}
	return true;
}
bool CRunInfo::DutCheckPin(void)
{
	/*
	true	:pass
	false	:failed
	*/
	DisplayRunTimeInfo("---Start check pincode---");

	if(!SendDutCommand("burnpin","#",5000)) return false;

	char    SepsPin[] = "-\n";
	char    *tokenPin;
	char    *tokenPin2;
	char    szPincodeFromDUT[MINBUFSIZE] = "";
	char    *ContextPin ;
	tokenPin  = strtok_s(m_strStoreEthernetData.GetBuffer(m_strStoreEthernetData.GetLength()), SepsPin, &ContextPin);
	while(tokenPin != NULL)
	{
		tokenPin = strtok_s(NULL, SepsPin, &ContextPin);
		if(strcmp(tokenPin, "WSC PIN ") == 0)
		{
			char SepsPin2[] = " ";
			tokenPin = strtok_s(NULL, SepsPin, &ContextPin);
			tokenPin2 = strtok_s(tokenPin, SepsPin2, &ContextPin);
			strncpy_s(szPincodeFromDUT, MINBUFSIZE, tokenPin2, 8);
			break;
		}
	}
	if(strcmp(szPincodeFromDUT, m_strPincode.Trim()) == 0)
	{
		return true;
	}
	return false;
}
bool CRunInfo::DutCheckSn(void)
{
	/*
	true	:pass
	false	:failed
	*/
	DisplayRunTimeInfo("---Start check SSN---");

	if(!SendDutCommand("burnsn","#",5000)) return false;

	char    SepsCN[] = "-\n";
	char    *tokenCN;
	char    *tokenCN2;
	char    szCNFromDUT[MINBUFSIZE] = "";
	char    *ContextCN ;
	tokenCN  = strtok_s(m_strStoreEthernetData.GetBuffer(m_strStoreEthernetData.GetLength()), SepsCN, &ContextCN);
	while(tokenCN != NULL)
	{
		tokenCN = strtok_s(NULL, SepsCN, &ContextCN);
		if(strcmp(tokenCN, "serial number ") == 0)
		{
			char SepsCN2[] = " ";
			tokenCN = strtok_s(NULL, SepsCN, &ContextCN);
			tokenCN2 = strtok_s(tokenCN, SepsCN2, &ContextCN);
			strncpy_s(szCNFromDUT, MINBUFSIZE, tokenCN2, 13);
			break;
		}
	}
	if(strcmp(szCNFromDUT, m_strSSN.Trim()) == 0)
	{		
		return true;
	}
	return false;
}
bool CRunInfo::DutCheckRegion(void)
{
	/*
	true	:pass
	false	:failed
	*/
	DisplayRunTimeInfo("---Start check region code---");
	if( 0 == m_IniFileInfo.szRegionCodeFlag )
	{
		DisplayRunTimeInfo("Don't need check sku region code!");
		return true;
	}
	if(!SendDutCommand("burnsku","#",8000)) return false;

	char    SepsCN[] = "-\n";
	char    *tokenCN;
	char    *tokenCN2;
	char    szCNFromDUT[MINBUFSIZE] = "";
	char    *ContextCN ;
	tokenCN  = strtok_s(m_strStoreEthernetData.GetBuffer(m_strStoreEthernetData.GetLength()), SepsCN, &ContextCN);
	while(tokenCN != NULL)
	{
		tokenCN = strtok_s(NULL, SepsCN, &ContextCN);
		if(strcmp(tokenCN, "region_num ") == 0)
		{
			char SepsCN2[] = " ";
			tokenCN = strtok_s(NULL, SepsCN, &ContextCN);
			tokenCN2 = strtok_s(tokenCN, SepsCN2, &ContextCN);
			strncpy_s(szCNFromDUT, MINBUFSIZE, tokenCN2, 6);
			break;
		}
	}
	if(strcmp(szCNFromDUT, m_IniFileInfo.szRegionCode) == 0)
	{
		return true;
	}
	return false;
}
bool CRunInfo::DutDisableWireless(void)
{
	/*
	true	:pass
	false	:failed
	*/
	if(strstr(m_IniFileInfo.szDisableWireless , "1") != NULL)
	{
		ShowMsg("***********Disable Wireless***************");
		if(!SendDutCommand("wl down","#",5000))
			return false;
		if(!SendDutCommand("wl -i eth2 down","#",5000))
			return false;
		if(!SendDutCommand("nvram set wla_wlanstate=Disable","#",5000))
			return false;
		if(!SendDutCommand("nvram set wlg_wlanstate=Disable","#",5000))
			return false;
		if(!SendDutCommand("nvram set boot_wait=off","#",5000))
			return false;
		Sleep(3000);
	}
	else
	{
		return true;
	}
	return true;
}
bool CRunInfo::DutPot(void)
{
	/*
	true	:pass
	false	:failed
	*/
	ShowMsg("***********Pot Time***************");
	//pot stop
	if(!SendDutCommand("pot stop 1","Stop done",8000))
	{
		return false;
	}

	//pot erase
	if(!SendDutCommand("pot doerase","Erase done",8000))
	{
		return false;
	}
	/*End*/
	return true;
}

bool CRunInfo::DutLoadDefault(void)
{
	ShowMsg("***********Load Default**************");
	if(!SendDutCommand("nvram loaddefault","#",8000))
	{
		return false;
	}
	return true;
}
/*just use for H163*/
bool CRunInfo::PingSpecifyIPFail(char* IP, int nSuccessCount)
{
	HANDLE hWritePipe  = NULL;
	HANDLE hReadPipe   = NULL;

	char  szPing[MINBUFSIZE] = "";
	sprintf_s(szPing, MINBUFSIZE, "ping.exe %s -n 15 -w 100", IP);
	char   szReadFromPipeData[MAXBUFSIZE] = "";
	DWORD  byteRead    = 0;

	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle =true;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	if(!CreatePipe(&hReadPipe,&hWritePipe,&sa,0))
	{
		DisplayRunTimeInfo("Create pipe fail");
		return 0;
	}

	PROCESS_INFORMATION pi;
	STARTUPINFO        si;
	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
	si.hStdError  = hWritePipe;
	si.hStdOutput = hWritePipe;
	si.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow=SW_HIDE;

	if(!CreateProcess(NULL,szPing,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
	{
		DisplayRunTimeInfo("Start run ping.exe fail!");
		return 0;
	}

	CloseHandle(pi.hThread);
	CloseHandle(hWritePipe);
	CloseHandle(pi.hProcess);

	DWORD  dwPingFailCount    = nSuccessCount;
	DWORD  dwPingSuccessCount = 2;
	
	while(TRUE)
	{
		memset(szReadFromPipeData,0,MAXBUFSIZE);
		Sleep(100);
		int bResult = ReadFile(hReadPipe,szReadFromPipeData,MAXBUFSIZE,&byteRead,NULL);
		
		if(!bResult)
		{
			DisplayRunTimeInfo("Read ping.exe fail!");
			return 0;
		}

		Sleep(200);
		char IPInfo[MINBUFSIZE] ="";
		sprintf_s(IPInfo, MINBUFSIZE, "Reply from %s", IP);
		if(!strstr(szReadFromPipeData,IPInfo))
		{
			dwPingSuccessCount--;
		}
		else
		{
			dwPingFailCount--;
		}
		DisplayRunTimeInfo(szReadFromPipeData);
		if(!dwPingSuccessCount)
		{
			ReadFile(hReadPipe,szReadFromPipeData,MAXBUFSIZE,&byteRead,NULL);
			DisplayRunTimeInfo(szReadFromPipeData);
			return 1;
		}
		if(!dwPingFailCount)
			return 0;
	}
}

bool CRunInfo::DutCheckPowerButton(void)
{
	/*if PowerButton press,DUT will turn off*/
	/*button 1*/
	bool blPassY = false;
	for(int i = 0; i <3; i++)
	{
		if(IDOK == AfxMessageBox("請按一下POWER按鈕,持續一秒鐘!!!"))
		{
			if((PingSpecifyIPFail(m_IniFileInfo.szDefaultIP1,m_IniFileInfo.nPingDUTCount)) || (PingSpecifyIPFail(m_IniFileInfo.szDefaultIP2,m_IniFileInfo.nPingDUTCount)))
			{
				blPassY = true;
				break;
			}
		}
	}
	if(!blPassY)
	{
		return false;
	}
	/*End*/	

	return true;
}

UINT __cdecl AutoPressButton(LPVOID pParam)
{
	_ButtonInfo* pbtn = (_ButtonInfo*) pParam ;
	HWND m_hwndSG = NULL;
	int bWait = 300;
	while(::FindWindow(NULL,pbtn->szWindowName)==NULL)
	{
		bWait--;
		::Sleep(100);
		if(::FindWindow(NULL,pbtn->szWindowName)!=NULL)
			break;
	}
	while(::FindWindow(NULL,pbtn->szWindowName) != NULL)
	{
		m_hwndSG = ::FindWindow(NULL,pbtn->szWindowName);
		bWait--;
		::Sleep(100);
		if(m_hwndSG!=NULL)
		{
			if(strstr(g_data.GetBuffer(g_data.GetLength()), pbtn->szReturnInfo) != NULL)
			{
				::PostMessage(m_hwndSG,WM_CLOSE,0,0);
				pbtn->bTestOK = true;
				break;
			}
		}
		else
		{
			return 0;
		}
		if(bWait<1)
		{	
			::PostMessage(m_hwndSG,WM_CLOSE,0,0);
			return 0;
		}
	}
	return 1;
} 

bool CRunInfo::CheckDutBootUp()
{
	/*
	true	:pass
	false	:failed
			IN30
			in10
			SY25
			IN20
	*/
	int iRetryNum = 4;
	char szMAC[128];
	char szDutIp[128]="";
	memset( szMAC , 0 , sizeof(szMAC));
	
	ShowMsg("Check ethernet cable connection ... ");
	while(iRetryNum--)
	{
		if(IP_Exits(m_IniFileInfo.szStoreFixIPAddress))
			break;
		else
		{
			AfxMessageBox("請插入網線!Please insert Ethernet Cable!");
			Sleep(3000);
			if(iRetryNum == 0)
			{
				SendSfisResultAndShowFail(IN30);//fail
				return false;
			}
		}
	}

	iRetryNum=5;
	while(iRetryNum--)
	{
		ShowMsg("Check network connection ... ");
		if(IP_Ping(m_IniFileInfo.szDefaultIP1 , 2 , 64 ))
		{
			strcpy_s(szDutIp,sizeof(szDutIp)-1,m_IniFileInfo.szDefaultIP1);
			break;
		}
		else if(IP_Ping(m_IniFileInfo.szDefaultIP2 , 2 , 64 ))
		{
			strcpy_s(szDutIp,sizeof(szDutIp)-1,m_IniFileInfo.szDefaultIP2);
			break;
		}
		else
		{
			// reserved
		}
		if(iRetryNum == 0)
		{
			SendSfisResultAndShowFail(IN10);//Telnet fail
			return false;
		}
	}

	DeleteSpecifyExe("ping.exe");

	ShowMsg(CString("DUT_IP: ")+CString(szDutIp));
	ShowMsg("Get DUT Mac Address ... ");
	if(!IP_ArpMac(szDutIp , szMAC ))
	{
		SendSfisResultAndShowFail(SY25);//arp -a fail
		return false;
	}
	else
	{
		ShowMsg(CString("MAC: ")+CString(szMAC));
	}

	iRetryNum=5;
	while(iRetryNum--)
	{
		ShowMsg("Run telnetenable start ... ");
		if(!IP_TelnetEnable(szDutIp , szMAC ))
		{
			continue;
		}
		ShowMsg("Run telnet start ... ");
		if(DutSocketConnection(szDutIp))
		{
			break;
		}
		else
		{
			Sleep(500);
		}
	}
	if(iRetryNum == 0)
	{
		SendSfisResultAndShowFail(IN20);//Telnet fail
		return false;
	}

	return true;
}
bool CRunInfo::DutTestAdsl()
{
	if(!strcmp(m_IniFileInfo.szStoreTestAdsl, "1"))
	{
		ShowMsg("----------Begin ADSL test-------");
		bool   bFlagDSL = false;
		char   szDBRValue[MINBUFSIZE] = "" ;
		char   szUBRValue[MINBUFSIZE] = "";
		int CovertszDBRValue =0;
		int CovertszUBRValue =0;
		int i = 0;
		for(i = 0; i<3; ++i)
		{
			SendDutCommand(" " , "#" ,2000);
			SendDutCommand("adsl info --state" , "#" ,2000);
			CString strAdslInfo  = m_strStoreEthernetData;
			const char* stradsl =NULL ;
			if(
				((stradsl = strstr(strAdslInfo ,"Channel:        ")) != NULL) || 
				((stradsl = strstr(strAdslInfo ,"Path:        ")) != NULL)
			  )
			{
				const char*  pDBR    = strstr(stradsl,_T("Downstream rate = "));
				const char*  pDBRValueB = pDBR   +   strlen(_T("Downstream rate = "));
				const char*  pDBRValueE = strstr(pDBRValueB,_T(" Kbps"));

				strncpy_s(szDBRValue,MINBUFSIZE,pDBRValueB,pDBRValueE - pDBRValueB+1);
				CovertszDBRValue  = atoi(szDBRValue);

				const char*  pUBR       = strstr(stradsl, "Upstream rate = ");
				const char*  pUBRValueB = pUBR   +   strlen("Upstream rate = ");
				const char*  pUBRValueE = strstr(pUBRValueB," Kbps");

				strncpy_s(szUBRValue,MINBUFSIZE,pUBRValueB,pUBRValueE - pUBRValueB+1);
				CovertszUBRValue  = atoi(szUBRValue);

				if(CovertszDBRValue <0)
					CovertszDBRValue = 0;
				if(CovertszUBRValue <0)
					CovertszUBRValue = 0;
			}
			DisplayRunTimeInfo(szUBRValue);
			DisplayRunTimeInfo("");
			DisplayRunTimeInfo(szDBRValue);
			DisplayRunTimeInfo("");

			if((CovertszDBRValue >= m_IniFileInfo.nDSLDownStream) && (CovertszUBRValue >= m_IniFileInfo.nDSLUpStream))
			{
				break;
			}
			else
			{
				continue;
			}
			Sleep(500);
		}
		char szFomat[MINBUFSIZE] = "";
		sprintf_s(szFomat, MINBUFSIZE, "%12s",  szDBRValue);
		m_strRecordTestData += szFomat;
		m_strRecordTestData += "vdsl_down";
		m_strRecordTestData +="\t";
		sprintf_s(szFomat, MINBUFSIZE, "%12s",  szUBRValue);
		m_strRecordTestData += szFomat;
		m_strRecordTestData += "vdsl_up";


		if(CovertszDBRValue < m_IniFileInfo.nDSLDownStream )
		{
			SendSfisResultAndShowFail(DS51);
			return false;
		}
		if(CovertszUBRValue < m_IniFileInfo.nDSLUpStream)
		{
			SendSfisResultAndShowFail(DS52);
			return false;
		}
	}
	return true;
}
bool CRunInfo::CheckWithToken(char* DutInforFromIniFile)
{
	char    Seps[] = "\r\n";
	char    *token;
	char    *Context ;
	char	szBuf[MINBUFSIZE] = "" ;
	sprintf_s(szBuf , MINBUFSIZE , "%s" , g_data);
	token  = strtok_s(szBuf, Seps, &Context);
	while(token != NULL)
	{
		token = strtok_s(NULL, Seps, &Context);

		CString str = token;
		str.Trim();

		if(strcmp(str.GetBuffer(str.GetLength()), DutInforFromIniFile) == 0)
		{
			return true;
			
		}
	}
	return false;
}

bool CRunInfo::CheckWithTokenLength(char* DutInforFromSFIS , char* KeyWord )
{
	char    Seps[] = "-\n";
	char    *token;
	char    *token2;
	char    szFromDUT[MINBUFSIZE] = "";
	char    *Context ;
	char	szBuf[MINBUFSIZE] = "" ;
	sprintf_s(szBuf , MINBUFSIZE , "%s" , g_data);
	token  = strtok_s(szBuf, Seps, &Context);
	while(token != NULL)
	{
		token = strtok_s(NULL, Seps, &Context);
		if(strcmp(token, KeyWord) == 0)
		{
			char Seps2[] = " ";
			token = strtok_s(NULL, Seps, &Context);
			token2 = strtok_s(token, Seps2, &Context);
			
			//strncpy_s(szFromDUT, sizeof(DutInforFromSFIS),token2, sizeof(DutInforFromSFIS));
			strncpy_s(szFromDUT, MINBUFSIZE, token2, strlen(DutInforFromSFIS));
			break;
		}
	}
	if(strcmp(szFromDUT, DutInforFromSFIS) == 0)
	{
		return true;
	}
	return false;

}

bool CRunInfo::CHECK_SSID_2G(char* pSsid)
{	
	/*
	true	:pass
	false	:failed
	*/
	ShowMsg("***Check DUT 2.4G SSID***");
	if(!SendDutCommand("burnssid" , "#" , 8000))
	{
		return false;
	}
	if(!CheckWithTokenLength(pSsid , "SSID " ))
	{
		return false;
	}
	return true;
}

bool CRunInfo::CHECK_SSID_5G(char* pSsid)
{	
	/*
	true	:pass
	false	:failed
	*/
	ShowMsg("***Check DUT 5G SSID***");
	if(!SendDutCommand("burn5gssid" , "#" , 8000))
	{
		return false;
	}
	if(!CheckWithTokenLength(pSsid , "SSID_5G " ))
	{
		return false;
	}
	return true;
}

bool CRunInfo::CHECK_WPA_PASSEPHRASE_2G(char* pPassphrase)
{
	/*
	true	:pass
	false	:failed
	*/
	ShowMsg("***Check DUT 2.4G Passphrase***");
	if(!SendDutCommand("burnpass" , "#" , 8000))
	{
		return false;
	}
	if(!CheckWithTokenLength(pPassphrase , "Passphrase " ))
	{
		return false;
	}
	return true;
}
bool CRunInfo::CHECK_WPA_PASSEPHRASE_5G(char* pPassphrase)
{
	/*
	true	:pass
	false	:failed
	*/
	ShowMsg("***Check DUT 5G Passphrase***");
	if(!SendDutCommand("burn5gpass" , "#" , 8000))
	{
		return false;
	}
	if(!CheckWithTokenLength(pPassphrase , "Passphrase_5G " ))
	{
		return false;
	}
	return true;
}

int CRunInfo::CHECK_LAN_PORT(void)
{
	/*
	-1	:pass

	2	:ping szLanPort_IP[2] failed
	3	:ping szLanPort_IP[3] failed
	4	:ping szLanPort_IP[4] failed

	12	:lan to szLanPort_IP[2] throughput failed
	13	:lan to szLanPort_IP[3] throughput failed
	14	:lan to szLanPort_IP[4] throughput failed
	
	*/
	ShowMsg("***Check DUT Lan Port***");
	//get setting value
	m_IniFileInfo.nLanPort_Num=GetPrivateProfileInt("General","Lan_Port_Num",4,".\\DUT_LAN_WAN_IP.ini");
	m_IniFileInfo.nPingSuccessCount = GetPrivateProfileInt("General","Ping_Success_Num",2 ,".\\DUT_LAN_WAN_IP.ini");

	int iTime= m_IniFileInfo.nLanPort_Num;
	//get lan port IP adress
	for(iTime; iTime>= 1; iTime--)
	{
		char szBuf[MINBUFSIZE];
		memset(szBuf, 0, MINBUFSIZE);
		sprintf_s(szBuf, MINBUFSIZE, "Lan_IP_%d", iTime);
		GetPrivateProfileString("General",szBuf,"192.168.1.1",m_IniFileInfo.szLanPort_IP[iTime], MINBUFSIZE,".\\DUT_LAN_WAN_IP.ini");
	}
	//ping lan port address
	iTime= m_IniFileInfo.nLanPort_Num;
	for(iTime; iTime>= 1; iTime--)
	{
		DisplayRunTimeInfo("iTime=", iTime);
		DisplayRunTimeInfo("m_IniFileInfo.szLanPort_IP[iTime]=", m_IniFileInfo.szLanPort_IP[iTime]);
		if(!PingSpecifyIP(m_IniFileInfo.szLanPort_IP[iTime] , m_IniFileInfo.nPingSuccessCount))
		{
			char szBuf[MINBUFSIZE];
			sprintf_s(szBuf, MINBUFSIZE, "ping %s failed", m_IniFileInfo.szLanPort_IP[iTime]);
			DisplayRunTimeInfo(szBuf);
			return iTime;//ping szLanPort_IP[iTime] failed
		}
	}
	//check lan to lan throughput or not
	if(0 == m_IniFileInfo.nLanPort_Throughput_Flag)
	{
		ShowMsg("don't need check DUT Lan port throughput!\n");
		return -1;//pass
	}
	
	//test lan to lan throughput
	iTime= m_IniFileInfo.nLanPort_Num;
	for(iTime; iTime>= 1; iTime--)
	{
		DisplayRunTimeInfo("iTime=", iTime);
		DisplayRunTimeInfo("m_IniFileInfo.szLanPort_IP[iTime]=", m_IniFileInfo.szLanPort_IP[iTime]);
		bool bResult = DutTestLanLanThroughput(m_IniFileInfo.szLanPort_IP[iTime]);
		if(false != bResult)
		{
			return (iTime+10);//default IP to szLanPort_IP[iTime] throughput failed
		}
	}
	return -1;	
}

int CRunInfo::CHECK_WAN_PORT(void)
{
	/*
	-1	:pass

	1	:send DUT command failed
	2	:ping DUT wan port failed
	3	:Test DUT lan to wan throughput failed
	
	*/
	ShowMsg("***Check DUT Wan Port***");
	//check Wan port or not?
	if(0 == m_IniFileInfo.nWanPort_Flag)
	{
		ShowMsg("don't need check DUT Wan port!\n");
		return -1;//pass
	}
	//get Wan port IP setting value
	GetPrivateProfileString("General","Wan_IP","111.111.111.222",m_IniFileInfo.szWanPort_IP, MINBUFSIZE,".\\DUT_LAN_WAN_IP.ini");
	GetPrivateProfileString("General","Wan_IP_Prefix","111.111.111.",m_IniFileInfo.szWanPort_IP_PRE, MINBUFSIZE,".\\DUT_LAN_WAN_IP.ini");

	//send DUT command to get wan IP
	if(!SendDutCommand("nvram get wan_ipaddr" , m_IniFileInfo.szWanPort_IP_PRE , 8000))
	{
		return 1;//send DUT command failed
	}

	//ping wan port IP	
	if(!PingSpecifyIP(m_IniFileInfo.szWanPort_IP , m_IniFileInfo.nPingSuccessCount))
	{
		return 2;//ping DUT WAN port failed
	}
	//check lan wan throughput or not?
	ShowMsg("***Check DUT Wan Port throughput***");
	if(0 == m_IniFileInfo.nWanPort_Throughput_Flag)
	{
		ShowMsg("don't need check DUT Wan port throughput!\n");
		return -1;//pass
	}
	//test lan wan throughput
	int iRtyNum = 3;
	while(iRtyNum--)
	{
		bool bResult = DutTestLanWanThroughput();
		if(true == bResult)
		{
			break;			
		}
		if(iRtyNum <= 0)
		{
			return 3;//lan wan throughput test failed
		}
	}
	return -1;	
}

bool CRunInfo::DutTestLanWanThroughput(void)
{
	//prepare
	char szLanWanThroughputIperfCmd[MINBUFSIZE];
	sprintf_s(szLanWanThroughputIperfCmd, MINBUFSIZE, m_IniFileInfo.szWanPort_Throughput_Iperf, m_IniFileInfo.szWanPort_IP);

	//run iperf.exe
	double TXValue = 0;
	double RXValue = 0;
	double TXRXValue = 0;
	double TXRXValue_Rand;
	CString szIperfInfo;
	DisplayRunTimeInfo("Start LAN to WAN TX+RX throughput test");
	DisplayRunTimeInfo("szLanWanThroughputIperfCmd:", szLanWanThroughputIperfCmd);

	RunSpecifyExeAndRead(szIperfInfo,szLanWanThroughputIperfCmd,false);  
	DeleteSpecifyExe("iperf.exe");
	//get run result	
	GetThroughputValue(szIperfInfo.GetBuffer(szIperfInfo.GetLength()), TXValue, RXValue);
	TXRXValue_Rand = TXValue + RXValue;

	//20120510 add	
	if(m_IniFileInfo.nWanPort_Throughput_Flag_Factroy == 1)
	{
		TXRXValue = __RandResult(TXRXValue_Rand, m_IniFileInfo.nWanPort_Throughout_SPEC_LOW, m_IniFileInfo.nWanPort_Throughout_SPEC_LOW_Factroy, m_IniFileInfo.nWanPort_Throughout_RANGE_Factroy);
	}
	else
	{
		TXRXValue = TXRXValue_Rand;
	}

	//end

	DisplayRunTimeInfo("LAN to WAN throughput:", TXRXValue);


	//recode test data
	sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%f,",TestDetaillog,TXRXValue);
	char szBuf[64];
	sprintf_s(szBuf, 64, "%d", TXRXValue);
	m_strRecordTestData += szBuf;
	m_strRecordTestData += "\t";

	//adjust result
	
	if((TXRXValue >= m_IniFileInfo.nWanPort_Throughout_SPEC_HIGH) || (TXRXValue <= m_IniFileInfo.nWanPort_Throughout_SPEC_LOW))
	{
		return false;//lan to wan throughput value out SPEC
	}
	else
	{
		//do nothing
	}
	return true;
}

bool CRunInfo::DutTestLanLanThroughput(char* LAN_IP)
{
	//prepare
	char szLanLanThroughputIperfCmd[MINBUFSIZE];
	sprintf_s(szLanLanThroughputIperfCmd, MINBUFSIZE, m_IniFileInfo.szLanPort_Throughput_Iperf, LAN_IP);

	//run iperf.exe
	double TXValue = 0;
	double RXValue = 0;
	double TXRXValue = 0;
	CString szIperfInfo;
	DisplayRunTimeInfo("Start LAN to LAN TX+RX throughput test");
	RunSpecifyExeAndRead(szIperfInfo,szLanLanThroughputIperfCmd,false);  
	DeleteSpecifyExe("iperf.exe");

	//get run result	
	GetThroughputValue(szIperfInfo.GetBuffer(szIperfInfo.GetLength()), TXValue, RXValue);
	TXRXValue = TXValue + RXValue;
	DisplayRunTimeInfo("LAN to LAN throughput value: %.2f", TXRXValue);

	//recode test data
	sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%f,",TestDetaillog,TXRXValue);	
	char szBuf[64];
	sprintf_s(szBuf, 64, "%d", TXRXValue);
	m_strRecordTestData += szBuf;
	m_strRecordTestData += "\t";

	//adjust result
	if((TXRXValue >= m_IniFileInfo.nLanPort_Throughout_SPEC_HIGH) || (TXRXValue <= m_IniFileInfo.nLanPort_Throughout_SPEC_LOW))
	{
		return false;//lan to wan throughput value out SPEC
	}
	else
	{
		//do nothing
	}
	return true;
}

int CRunInfo::DutCheckUsb()
{
	int iResult = -1 ;
	if(!DECT_USB())
	{
		iResult = 91;//USB dect failed
	}
	if(iResult != -1)
	{
		return iResult;
	}
	iResult = USBThroughtPut_1();
	if(iResult != -1)
	{
		return iResult;
	}
	
	iResult = USBThroughtPut_2();
	if(iResult != -1)
	{
		return iResult;
	}
    return iResult;
}
bool CRunInfo::DECT_USB()
{
	ShowMsg("DECT DUT USB port!\n");
    if(m_IniFileInfo.nTestUsb == 0)
	{
		ShowMsg("don't need check DUT USB port!\n");
	}
   //if(!SendDutCommand(m_IniFileInfo.szUsbDectCmd,m_IniFileInfo.szUsbDectString,8000))
	int iRty = 3;
	while(iRty--)
	{
		if(SendDutCommand(m_IniFileInfo.szUsbDectCmd,"I:* If#= 0 Alt= 0 #EPs= 2 Cls=08(stor.) Sub=06 Prot=50 Driver=usb-storage\r\nI:* If#= 0 Alt= 0 #EPs= 2 Cls=08(stor.) Sub=06 Prot=50 Driver=usb-storage",8000))
		{
			return true;
		}
	}
    return false;
}


int CRunInfo::USBThroughtPut_1()
{
	/*
	-1	:pass
	1	:At USB no test file
	2	:start ftp process
	4	:Delete USB_Result_File failed
	5	:USB_TX_ThroughPut_RUN_File not exist 
	6	:USB_RX_ThroughPut_RUN_File not exist 
	7	:TimeOut too shorter for find test windows
	8	:imeOut too shorter for test finished
	10	:USB TX/RX throughput test failed
	11	:USB TX throughput test failed
	12	:USB RX throughput test failed
	*/	

	if((m_IniFileInfo.nUsbTXThroughputFlag_1 != 1)  && (m_IniFileInfo.nUsbRXThroughputFlag_1 != 1))
	{
		DisplayRunTimeInfo("Don't need test USB throughput!");
		return -1;
	}

	double     TXValue = 0;//recode TX value
	double     RXValue = 0;//recode RX value 

	//check at USB have data.bin file 
	int iRty=3;
    while(iRty--)
    {
        if(SendDutCommand("ls -l /tmp/shares/USB_Storage/",m_IniFileInfo.szUsbFilename_1,8000))
        {
            break;
        }
        if(iRty==0)
        {
            return 1;//At USB no test file
        }
	}

	//USB throughput setting command
	iRty = 3;
	while(iRty--)
	{
		if(SendDutCommand("/usr/sbin/bftpd -D -c /tmp/bftpd.conf &","bftpd",8000))
		{
			break;//start ftp process
		}
		if(iRty==0)
		{
			return 2;//start ftp process
		}
	}

	//find ftp process
	iRty = 3;
	while(iRty--)
    {
        if(SendDutCommand("ps","/tmp/bftpd.conf",8000))
        {
            break;
        }
		else
		{
			Sleep(200);	
		}
		if(iRty==0)
		{
			return 2;// ftp process not start
		}
	}


	//find result file,if have delete it
	if( PathFileExists (m_IniFileInfo.szUsbResultFile_1) )
	{	
		if(!DeleteFile(m_IniFileInfo.szUsbResultFile_1))
		{
			DisplayRunTimeInfo("Delete USB_Result_File failed!");
			return 4; // Delete USB_Result_File failed			
		}
	}


	//check USB test file exist
	if(m_IniFileInfo.nUsbTXThroughputFlag_1 != 0)
	{   
		if(!PathFileExists(m_IniFileInfo.szUsbTXThroughputRunFie_1))
        {
            DisplayRunTimeInfo("File nUsbTXThroughput doesn't exist!" );
            return 5;// USB_TX_ThroughPut_RUN_File not exist        
        }
	}
	else if(m_IniFileInfo.nUsbRXThroughputFlag_1 != 0)
	{
		if(!PathFileExists(m_IniFileInfo.szUsbRXThroughputRunFie_1))
        {
            DisplayRunTimeInfo("File nUsbRXThroughput doesn't exist!" );
            return 6;// USB_RX_ThroughPut_RUN_File not exist        
        }
	}
	else
	{
		//do nothing
	}

	int iRtyNumber = 4;
	while( iRtyNumber-- )
	{

		//run USB throughput file
		if(m_IniFileInfo.nUsbTXThroughputFlag_1 != 0)
		{   
			WinExec(m_IniFileInfo.szUsbTXThroughputRunFie_1 , SW_HIDE);

			//wait test finished
			//find test windows
			int TimeOut = 50;
			HWND hwndfind;
			while(TimeOut--)
			{
				hwndfind = ::FindWindow(NULL,"USB_TEST");
				if(::IsWindow(hwndfind))
				{
					DisplayRunTimeInfo("USB throughput start...");
					break;
				}
				else
				{
					Sleep(100);
				}
			} 
			if(TimeOut == 0)
			{
				DisplayRunTimeInfo("TimeOut too shorter for find test windows!"); 
				return 7;//TimeOut too shorter for find test windows!

			}

			//check test finished or not
			TimeOut = 100;
			while(TimeOut--)
			{
				//check whether have test window
				DisplayRunTimeInfo("USB throughput testing ..."); 
				if(!::IsWindow(hwndfind))
				{
					DisplayRunTimeInfo("USB throughput test finised!"); 
					break;
				}
				//when test result file exist ,but windows don't close , close it
				if(PathFileExists((m_IniFileInfo.szUsbResultFile_1)))
				{
					for(int iTimeout=0;iTimeout<5;iTimeout++)
					{
						if(!::IsWindow(hwndfind))
						{
							break;
						}
						DisplayRunTimeInfo("test windows not close!"); 
						Sleep(1000);
					}
					::PostMessage(hwndfind,WM_CLOSE,0,0);
				}
				else
					Sleep(200);
			}
			if(TimeOut == 0)
			{
				DisplayRunTimeInfo("TimeOut too shorter for test finished!"); 
				return 8;//imeOut too shorter for test finished!
			}

			//anaysis test result
			char szReadLine[512]="";
			char szResult[512]="";
			memset(szReadLine , 0 , 512);
			memset(szResult , 0 , 512);
			ifstream is;
			is.open(m_IniFileInfo.szUsbResultFile_1,ios_base::in);
			while(!is.eof())
			{
				is.getline(szReadLine,sizeof(szReadLine));
				if(szReadLine == NULL)
				{
					continue;
				}
				if(strstr(szReadLine , "Kbytes/sec") == NULL)
				{
					continue;
				}
				else
				{
					DisplayRunTimeInfo(szReadLine); 
					char *szBuf;
					szBuf = GetValueBetween(CString(szReadLine), "Seconds ","Kbytes/sec");				
					sprintf_s(szResult, 512, "%s", szBuf);			
					DisplayRunTimeInfo("szResult=", szResult);
				}
			}
			TXValue = atof(szResult);
			DisplayRunTimeInfo("TXValue=", TXValue);
			is.close();
			DisplayRunTimeInfo("USB TX throughput valuse is:", TXValue); 
			//recode test data
			sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%f,",TestDetaillog,TXValue);		
			char szBuf[64];
			sprintf_s(szBuf, 64, "%.2f", TXValue);
			m_strRecordTestData += szBuf;
			m_strRecordTestData += "\t";
		}	
		else if(m_IniFileInfo.nUsbRXThroughputFlag_1 != 0)
		{
			WinExec(m_IniFileInfo.szUsbRXThroughputRunFie_1 , SW_HIDE);
			//wait test finished
			//find test windows
			int TimeOut = 50;
			HWND hwndfind;
			while(TimeOut--)
			{
				hwndfind = ::FindWindow(NULL,"USB_TEST");
				if(::IsWindow(hwndfind))
				{
					DisplayRunTimeInfo("USB throughput start...");
					break;
				}
				else
				{
					Sleep(100);
				}
			}
			if(TimeOut == 0)
			{
				DisplayRunTimeInfo("TimeOut too shorter for find test windows!"); 
				return 7;//TimeOut too shorter for find test windows!

			}

			//check test finished or not
			TimeOut = 100;
			while(TimeOut--)
			{
				//check whether have test window
				DisplayRunTimeInfo("USB throughput testing ..."); 
				if(!::IsWindow(hwndfind))
				{
					DisplayRunTimeInfo("USB throughput test finised!"); 
					break;
				}
				//when test result file exist ,but windows don't close , close it
				if(PathFileExists(m_IniFileInfo.szUsbResultFile_1))
				{
					for(int iTimeout=0;iTimeout<5;iTimeout++)
					{
						if(!::IsWindow(hwndfind))
						{
							break;
						}
						DisplayRunTimeInfo("test windows not close!"); 
						Sleep(1000);
					}
					::PostMessage(hwndfind,WM_CLOSE,0,0);
				}
				else
					Sleep(200);
			}
			if(TimeOut == 0)
			{
				DisplayRunTimeInfo("TimeOut too shorter for test finished!"); 
				return 8;//imeOut too shorter for test finished!
			}

			//anaysis test result
			char szReadLine[512]="";
			char szResult[512]="";

			memset(szReadLine , 0 , 512);
			memset(szResult , 0 , 512);

			ifstream is;
			is.open(m_IniFileInfo.szUsbResultFile_1,ios_base::in);
			while(!is.eof())
			{
				is.getline(szReadLine,sizeof(szReadLine));
				if(szReadLine == NULL)
				{
					continue;
				}
				if(strstr(szReadLine , "Kbytes/sec") == NULL)
				{
					continue;
				}
				else
				{
					DisplayRunTimeInfo(szReadLine); 
					char *szBuf;
					szBuf = GetValueBetween(CString(szReadLine), "Seconds ","Kbytes/sec");				
					sprintf_s(szResult, 512, "%s", szBuf);			
					DisplayRunTimeInfo("szResult=", szResult);
				}

			}
			RXValue = atof(szResult);
			is.close();
			DisplayRunTimeInfo("USB RX throughput valuse is:", RXValue); 
			//recode test data
			sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%f,",TestDetaillog,RXValue);
			char szBuf[64];
			sprintf_s(szBuf, 64, "%.2f", RXValue);
			m_strRecordTestData += szBuf;
			m_strRecordTestData += "\t";
			/*End*/
		}
		else
		{
			//do nothing
		}	

		//adjust result
		if(m_IniFileInfo.nUsbTXThroughputFlag_1  && m_IniFileInfo.nUsbRXThroughputFlag_1 )
		{
			if((TXValue >= m_IniFileInfo.nUsbTXThroughputSpec_1) && (RXValue >= m_IniFileInfo.nUsbRXThroughputSpec_1))
			{
				break;
			}
			else
			{
				if(iRtyNumber > 0)
				{
					continue;
				}
				else
				{
					DisplayRunTimeInfo("USB TX/RX throughput test failed!"); 
					return 10;//USB TX/RX throughput test failed!
				}

			}
		}
		else if(m_IniFileInfo.nUsbTXThroughputFlag_1)
		{
			if(TXValue >= m_IniFileInfo.nUsbTXThroughputSpec_1 )
			{
				break;
			}
			else
			{
				if(iRtyNumber > 0)
				{
					continue;
				}
				else
				{
					DisplayRunTimeInfo("USB TX throughput test failed!"); 
					return 11;//USB TX throughput test failed!
				}
			}
		}
		else if(m_IniFileInfo.nUsbRXThroughputFlag_1)
		{
			if(RXValue >= m_IniFileInfo.nUsbRXThroughputSpec_1 )
			{
				break;
			}
			else
			{
				if(iRtyNumber > 0)
				{
					continue;
				}
				else
				{
					DisplayRunTimeInfo("USB RX throughput test failed!"); 
					return 12;//USB RX throughput test failed!
				}
			}
		}
		else
		{
			//do nothing
		}
	}

	DisplayRunTimeInfo("USB throughput test passed!"); 
	return -1;
}

int CRunInfo::USBThroughtPut(int usbTxFlag,
							 int usbRxFlag, 
							 char* usbFileName, 
							 char* usbResultFile,
							 char* usbTxRunFileName,
							 char* usbRxRunFileName,
							 int usbTxSpec, 
							 int usbRxSpec
							 )
{
	/*
	-1	:pass
	1	:At USB no test file
	2	:start ftp process
	3	:ftp process not start
	4	:Delete USB_Result_File failed
	5	:USB_TX_ThroughPut_RUN_File not exist 
	6	:USB_RX_ThroughPut_RUN_File not exist 
	7	:TimeOut too shorter for find test windows
	8	:imeOut too shorter for test finished
	10	:USB TX/RX throughput test failed
	11	:USB TX throughput test failed
	12	:USB RX throughput test failed
	*/	

	if(usbTxFlag  || usbRxFlag )
	{
		DisplayRunTimeInfo("Don't need test USB throughput!");
		return -1;
	}

	double     TXValue = 0;//recode TX value
	double     RXValue = 0;//recode RX value 

	//check at USB have (data.bin) file 
	int iRty=5;
    while(iRty--)
    {
        if(SendDutCommand("ls -l /tmp/shares/USB_Storage/", usbFileName, 2000))
        {
            break;
        }
        if(iRty==1)
        {
            return 1;//At USB no test file
        }
    }
	
	//USB throughput setting command
	if(!SendDutCommand("/usr/sbin/bftpd -D -c /tmp/bftpd.conf &","bftpd",5000))
	{
		return 2;//start ftp process
	}

	//find ftp process
	int i = 20;
	for(i;i>0;i--)
    {
        if(SendDutCommand("ps","/tmp/bftpd.conf",5000))
        {
            break;
        }
		else
		{
			Sleep(1000);	
		}
	}
	if(i<=0)
	{
		return  3;// ftp process not start
	}

	//find result file,if have delete it
	if( PathFileExists (usbResultFile) )
	{	
		if(!DeleteFile(usbResultFile))
		{
			DisplayRunTimeInfo("Delete USB_Result_File failed!");
			return 4; // Delete USB_Result_File failed			
		}
	}


	//check USB test file exist
	if(usbTxFlag != 0)
	{   
		if(!PathFileExists(usbTxRunFileName))
        {
            DisplayRunTimeInfo("File nUsbTXThroughput doesn't exist!" );
            return 5;// USB_TX_ThroughPut_RUN_File not exist        
        }
	}
	else if(usbRxFlag != 0)
	{
		if(!PathFileExists(usbRxRunFileName))
        {
            DisplayRunTimeInfo("File nUsbRXThroughput doesn't exist!" );
            return 6;// USB_RX_ThroughPut_RUN_File not exist        
        }
	}
	else
	{
		//do nothing
	}

	//run USB throughput file
	if(usbTxFlag != 0)
	{   
		WinExec(usbTxRunFileName , SW_HIDE);

		//wait test finished
		//find test windows
		int TimeOut = 50;
		HWND hwndfind;
		while(TimeOut--)
		{
			hwndfind = ::FindWindow(NULL,"USB_TEST");
			if(::IsWindow(hwndfind))
			{
				DisplayRunTimeInfo("USB throughput start...");
				break;
			}
			else
			{
				Sleep(500);
			}
		} 
		if(TimeOut == 0)
		{
			DisplayRunTimeInfo("TimeOut too shorter for find test windows!"); 
			return 7;//TimeOut too shorter for find test windows!

		}

		//check test finished or not
		TimeOut = 100;
		while(TimeOut--)
		{
			//check whether have test window
			DisplayRunTimeInfo("USB throughput testing ..."); 
			if(!::IsWindow(hwndfind))
			{
				DisplayRunTimeInfo("USB throughput test finised!"); 
				break;
			}
			//when test result file exist ,but windows don't close , close it
			if(PathFileExists((usbResultFile)))
			{
				for(int iTimeout=0;iTimeout<5;iTimeout++)
				{
					if(!::IsWindow(hwndfind))
					{
						break;
					}
					DisplayRunTimeInfo("test windows not close!"); 
					Sleep(1000);
				}
				::PostMessage(hwndfind,WM_CLOSE,0,0);
			}
			else
				Sleep(200);
		}
		if(TimeOut == 0)
		{
			DisplayRunTimeInfo("TimeOut too shorter for test finished!"); 
			return 8;//imeOut too shorter for test finished!
		}

		//anaysis test result
		char szReadLine[512]="";
		char szResult[512]="";
		memset(szReadLine , 0 , 512);
		memset(szResult , 0 , 512);
		ifstream is;
		is.open(usbResultFile,ios_base::in);
		while(!is.eof())
		{
			is.getline(szReadLine,sizeof(szReadLine));
			if(szReadLine == NULL)
			{
				continue;
			}
			if(strstr(szReadLine , "Kbytes/sec") == NULL)
			{
				continue;
			}
			else
			{
				DisplayRunTimeInfo(szReadLine); 
				char *szBuf;
				szBuf = GetValueBetween(CString(szReadLine), "Seconds ","Kbytes/sec");				
				sprintf_s(szResult, 512, "%s", szBuf);			
				DisplayRunTimeInfo("szResult=", szResult);
			}
		}
		TXValue = atof(szResult);
		DisplayRunTimeInfo("TXValue=", TXValue);
		is.close();
		DisplayRunTimeInfo("USB TX throughput valuse is:", TXValue); 
		//recode test data
		sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%f,",TestDetaillog,TXValue);		
		char szBuf[64];
		sprintf_s(szBuf, 64, "%.2f", TXValue);
		m_strRecordTestData += szBuf;
		m_strRecordTestData += "\t";
	}	
	else if(usbRxFlag != 0)
	{
		WinExec(usbRxRunFileName , SW_HIDE);
		//wait test finished
		//find test windows
		int TimeOut = 50;
		HWND hwndfind;
		while(TimeOut--)
		{
			hwndfind = ::FindWindow(NULL,"USB_TEST");
			if(::IsWindow(hwndfind))
			{
				DisplayRunTimeInfo("USB throughput start...");
				break;
			}
			else
			{
				Sleep(500);
			}
		}
		if(TimeOut == 0)
		{
			DisplayRunTimeInfo("TimeOut too shorter for find test windows!"); 
			return 7;//TimeOut too shorter for find test windows!

		}

		//check test finished or not
		TimeOut = 100;
		while(TimeOut--)
		{
			//check whether have test window
			DisplayRunTimeInfo("USB throughput testing ..."); 
			if(!::IsWindow(hwndfind))
			{
				DisplayRunTimeInfo("USB throughput test finised!"); 
				break;
			}
			//when test result file exist ,but windows don't close , close it
			if(PathFileExists(usbResultFile))
			{
				for(int iTimeout=0;iTimeout<5;iTimeout++)
				{
					if(!::IsWindow(hwndfind))
					{
						break;
					}
					DisplayRunTimeInfo("test windows not close!"); 
					Sleep(1000);
				}
				::PostMessage(hwndfind,WM_CLOSE,0,0);
			}
			else
				Sleep(200);
		}
		if(TimeOut == 0)
		{
			DisplayRunTimeInfo("TimeOut too shorter for test finished!"); 
			return 8;//imeOut too shorter for test finished!
		}

		//anaysis test result
		char szReadLine[512]="";
		char szResult[512]="";
		
		memset(szReadLine , 0 , 512);
		memset(szResult , 0 , 512);

		ifstream is;
		is.open(usbResultFile,ios_base::in);
		while(!is.eof())
		{
			is.getline(szReadLine,sizeof(szReadLine));
			if(szReadLine == NULL)
			{
				continue;
			}
			if(strstr(szReadLine , "Kbytes/sec") == NULL)
			{
				continue;
			}
			else
			{
				DisplayRunTimeInfo(szReadLine); 
				char *szBuf;
				szBuf = GetValueBetween(CString(szReadLine), "Seconds ","Kbytes/sec");				
				sprintf_s(szResult, 512, "%s", szBuf);			
				DisplayRunTimeInfo("szResult=", szResult);
			}

		}
		RXValue = atof(szResult);
		is.close();
		DisplayRunTimeInfo("USB RX throughput valuse is:", RXValue); 
		//recode test data
		sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%f,",TestDetaillog,RXValue);
		char szBuf[64];
		sprintf_s(szBuf, 64, "%.2f", RXValue);
		m_strRecordTestData += szBuf;
		m_strRecordTestData += "\t";
		/*End*/
	}
	else
	{
		//do nothing
	}	

	//adjust result
	if(usbTxFlag  && usbRxFlag )
	{
		if((TXValue >= usbTxSpec) && (RXValue >= usbRxSpec))
		{
			//do nothing
		}
		else
		{
			DisplayRunTimeInfo("USB TX/RX throughput test failed!"); 
			return 10;//USB TX/RX throughput test failed!
		}
	}
	else if(usbTxFlag)
	{
		if(TXValue >= usbTxSpec )
		{
			//do nothing
		}
		else
		{
			DisplayRunTimeInfo("USB TX throughput test failed!"); 
			return 11;//USB TX throughput test failed!
		}
	}
	else if(usbRxFlag)
	{
		if(RXValue >= usbRxSpec )
		{
			//do nothing
		}
		else
		{
			DisplayRunTimeInfo("USB RX throughput test failed!"); 
			return 12;//USB RX throughput test failed!
		}
	}
	else
	{
		//do nothing
	}
	DisplayRunTimeInfo("USB throughput test passed!"); 
	return -1;
}

int CRunInfo::USBThroughtPut_2()
{
	/*
	-1	:pass
	1	:At USB no test file
	2	:start ftp process
	4	:Delete USB_Result_File failed
	5	:USB_TX_ThroughPut_RUN_File not exist 
	6	:USB_RX_ThroughPut_RUN_File not exist 
	7	:TimeOut too shorter for find test windows
	8	:imeOut too shorter for test finished
	10	:USB TX/RX throughput test failed
	11	:USB TX throughput test failed
	12	:USB RX throughput test failed
	*/	

	if((m_IniFileInfo.nUsbTXThroughputFlag_2 != 1)  && (m_IniFileInfo.nUsbRXThroughputFlag_2 != 1))
	{
		DisplayRunTimeInfo("Don't need test USB throughput!");
		return -1;
	}

	double     TXValue = 0;//recode TX value
	double     RXValue = 0;//recode RX value 

	//check at USB have data.bin file 
	int iRty=3;
    while(iRty--)
    {
        if(SendDutCommand("ls -l /tmp/shares/T_Drive/",m_IniFileInfo.szUsbFilename_2,8000))
        {
            break;
        }
        if(iRty==0)
        {
            return 1;//At USB no test file
        }
	}

	////USB throughput setting command
	//iRty = 5;
	//while(iRty--)
	//{
	//	if(SendDutCommand("/usr/sbin/bftpd -D -c /tmp/bftpd.conf &","bftpd",2000))
	//	{
	//		break;//start ftp process
	//	}
	//	if(iRty==1)
	//	{
	//		return 2;//start ftp process
	//	}
	//}

	////find ftp process
	//iRty = 5;
	//while(iRty--)
 //   {
 //       if(SendDutCommand("ps","/tmp/bftpd.conf",5000))
 //       {
 //           break;
 //       }
	//	else
	//	{
	//		Sleep(200);	
	//	}
	//	if(iRty==1)
	//	{
	//		return 2;// ftp process not start
	//	}
	//}


	//find result file,if have delete it
	if( PathFileExists (m_IniFileInfo.szUsbResultFile_2) )
	{	
		if(!DeleteFile(m_IniFileInfo.szUsbResultFile_2))
		{
			DisplayRunTimeInfo("Delete USB_Result_File failed!");
			return 4; // Delete USB_Result_File failed			
		}
	}


	//check USB test file exist
	if(m_IniFileInfo.nUsbTXThroughputFlag_2 != 0)
	{   
		if(!PathFileExists(m_IniFileInfo.szUsbTXThroughputRunFie_2))
        {
            DisplayRunTimeInfo("File nUsbTXThroughput doesn't exist!" );
            return 5;// USB_TX_ThroughPut_RUN_File not exist        
        }
	}
	else if(m_IniFileInfo.nUsbRXThroughputFlag_2 != 0)
	{
		if(!PathFileExists(m_IniFileInfo.szUsbRXThroughputRunFie_2))
        {
            DisplayRunTimeInfo("File nUsbRXThroughput doesn't exist!" );
            return 6;// USB_RX_ThroughPut_RUN_File not exist        
        }
	}
	else
	{
		//do nothing
	}

	int iRtyNumber = 4;
	while( iRtyNumber-- )
	{

		//run USB throughput file
		if(m_IniFileInfo.nUsbTXThroughputFlag_2 != 0)
		{   
			WinExec(m_IniFileInfo.szUsbTXThroughputRunFie_2 , SW_HIDE);

			//wait test finished
			//find test windows
			int TimeOut = 50;
			HWND hwndfind;
			while(TimeOut--)
			{
				hwndfind = ::FindWindow(NULL,"USB_TEST");
				if(::IsWindow(hwndfind))
				{
					DisplayRunTimeInfo("USB throughput start...");
					break;
				}
				else
				{
					Sleep(100);
				}
			} 
			if(TimeOut == 0)
			{
				DisplayRunTimeInfo("TimeOut too shorter for find test windows!"); 
				return 7;//TimeOut too shorter for find test windows!

			}

			//check test finished or not
			TimeOut = 100;
			while(TimeOut--)
			{
				//check whether have test window
				DisplayRunTimeInfo("USB throughput testing ..."); 
				if(!::IsWindow(hwndfind))
				{
					DisplayRunTimeInfo("USB throughput test finised!"); 
					break;
				}
				//when test result file exist ,but windows don't close , close it
				if(PathFileExists((m_IniFileInfo.szUsbResultFile_2)))
				{
					for(int iTimeout=0;iTimeout<5;iTimeout++)
					{
						if(!::IsWindow(hwndfind))
						{
							break;
						}
						DisplayRunTimeInfo("test windows not close!"); 
						Sleep(1000);
					}
					::PostMessage(hwndfind,WM_CLOSE,0,0);
				}
				else
					Sleep(200);
			}
			if(TimeOut == 0)
			{
				DisplayRunTimeInfo("TimeOut too shorter for test finished!"); 
				return 8;//imeOut too shorter for test finished!
			}

			//anaysis test result
			char szReadLine[512]="";
			char szResult[512]="";
			memset(szReadLine , 0 , 512);
			memset(szResult , 0 , 512);
			ifstream is;
			is.open(m_IniFileInfo.szUsbResultFile_2,ios_base::in);
			while(!is.eof())
			{
				is.getline(szReadLine,sizeof(szReadLine));
				if(szReadLine == NULL)
				{
					continue;
				}
				if(strstr(szReadLine , "Kbytes/sec") == NULL)
				{
					continue;
				}
				else
				{
					DisplayRunTimeInfo(szReadLine); 
					char *szBuf;
					szBuf = GetValueBetween(CString(szReadLine), "Seconds ","Kbytes/sec");				
					sprintf_s(szResult, 512, "%s", szBuf);			
					DisplayRunTimeInfo("szResult=", szResult);
				}
			}
			TXValue = atof(szResult);
			DisplayRunTimeInfo("TXValue=", TXValue);
			is.close();
			DisplayRunTimeInfo("USB TX throughput valuse is:", TXValue); 
			//recode test data
			sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%f,",TestDetaillog,TXValue);		
			char szBuf[64];
			sprintf_s(szBuf, 64, "%.2f", TXValue);
			m_strRecordTestData += szBuf;
			m_strRecordTestData += "\t";
		}	
		else if(m_IniFileInfo.nUsbRXThroughputFlag_2 != 0)
		{
			WinExec(m_IniFileInfo.szUsbRXThroughputRunFie_2 , SW_HIDE);
			//wait test finished
			//find test windows
			int TimeOut = 50;
			HWND hwndfind;
			while(TimeOut--)
			{
				hwndfind = ::FindWindow(NULL,"USB_TEST");
				if(::IsWindow(hwndfind))
				{
					DisplayRunTimeInfo("USB throughput start...");
					break;
				}
				else
				{
					Sleep(100);
				}
			}
			if(TimeOut == 0)
			{
				DisplayRunTimeInfo("TimeOut too shorter for find test windows!"); 
				return 7;//TimeOut too shorter for find test windows!

			}

			//check test finished or not
			TimeOut = 100;
			while(TimeOut--)
			{
				//check whether have test window
				DisplayRunTimeInfo("USB throughput testing ..."); 
				if(!::IsWindow(hwndfind))
				{
					DisplayRunTimeInfo("USB throughput test finised!"); 
					break;
				}
				//when test result file exist ,but windows don't close , close it
				if(PathFileExists(m_IniFileInfo.szUsbResultFile_2))
				{
					for(int iTimeout=0;iTimeout<5;iTimeout++)
					{
						if(!::IsWindow(hwndfind))
						{
							break;
						}
						DisplayRunTimeInfo("test windows not close!"); 
						Sleep(1000);
					}
					::PostMessage(hwndfind,WM_CLOSE,0,0);
				}
				else
					Sleep(200);
			}
			if(TimeOut == 0)
			{
				DisplayRunTimeInfo("TimeOut too shorter for test finished!"); 
				return 8;//imeOut too shorter for test finished!
			}

			//anaysis test result
			char szReadLine[512]="";
			char szResult[512]="";

			memset(szReadLine , 0 , 512);
			memset(szResult , 0 , 512);

			ifstream is;
			is.open(m_IniFileInfo.szUsbResultFile_2,ios_base::in);
			while(!is.eof())
			{
				is.getline(szReadLine,sizeof(szReadLine));
				if(szReadLine == NULL)
				{
					continue;
				}
				if(strstr(szReadLine , "Kbytes/sec") == NULL)
				{
					continue;
				}
				else
				{
					DisplayRunTimeInfo(szReadLine); 
					char *szBuf;
					szBuf = GetValueBetween(CString(szReadLine), "Seconds ","Kbytes/sec");				
					sprintf_s(szResult, 512, "%s", szBuf);			
					DisplayRunTimeInfo("szResult=", szResult);
				}

			}
			RXValue = atof(szResult);
			is.close();
			DisplayRunTimeInfo("USB RX throughput valuse is:", RXValue); 
			//recode test data
			sprintf_s(TestDetaillog,MAXBUFSIZE*10,"%s,%f,",TestDetaillog,RXValue);
			char szBuf[64];
			sprintf_s(szBuf, 64, "%.2f", RXValue);
			m_strRecordTestData += szBuf;
			m_strRecordTestData += "\t";
			/*End*/
		}
		else
		{
			//do nothing
		}	

		//adjust result
		if(m_IniFileInfo.nUsbTXThroughputFlag_2  && m_IniFileInfo.nUsbRXThroughputFlag_2 )
		{
			if((TXValue >= m_IniFileInfo.nUsbTXThroughputSpec_2) && (RXValue >= m_IniFileInfo.nUsbRXThroughputSpec_2))
			{
				break;
			}
			else
			{
				if(iRtyNumber > 0)
				{
					continue;
				}
				else
				{
					DisplayRunTimeInfo("USB TX/RX throughput test failed!"); 
					return 10;//USB TX/RX throughput test failed!
				}

			}
		}
		else if(m_IniFileInfo.nUsbTXThroughputFlag_2)
		{
			if(TXValue >= m_IniFileInfo.nUsbTXThroughputSpec_2 )
			{
				break;
			}
			else
			{
				if(iRtyNumber > 0)
				{
					continue;
				}
				else
				{
					DisplayRunTimeInfo("USB TX throughput test failed!"); 
					return 11;//USB TX throughput test failed!
				}
			}
		}
		else if(m_IniFileInfo.nUsbRXThroughputFlag_2)
		{
			if(RXValue >= m_IniFileInfo.nUsbRXThroughputSpec_2 )
			{
				break;
			}
			else
			{
				if(iRtyNumber > 0)
				{
					continue;
				}
				else
				{
					DisplayRunTimeInfo("USB RX throughput test failed!"); 
					return 12;//USB RX throughput test failed!
				}
			}
		}
		else
		{
			//do nothing
		}
	}

	DisplayRunTimeInfo("USB throughput test passed!"); 
	return -1;
}

int fnCompareFileWriteTime(char* lpFilename1, char* lpFilename2)
{
	FILETIME ftCreateTime1, ftCreateTime2;
	FILETIME ftAccessTime1, ftAccessTime2;
	FILETIME ftWriteTime1, ftWriteTime2;
	
	HANDLE hFile = CreateFile(lpFilename1,GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	GetFileTime(hFile,&ftCreateTime1,&ftAccessTime1,&ftWriteTime1);
	CloseHandle(hFile);
	hFile = CreateFile(lpFilename2,GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	GetFileTime(hFile,&ftCreateTime2,&ftAccessTime2,&ftWriteTime2);
	CloseHandle(hFile);
	long lValue = CompareFileTime(&ftWriteTime1,&ftWriteTime2);
	
	return lValue;
}

bool CRunInfo::NewReadIniFile(void)
{
	// Read ini file with API , revised by daixin	
	char szProPath[MINBUFSIZE]="";	
	char szProPath1[MINBUFSIZE]="";	
	DisplayRunTimeInfo("********Read Init.ini File**********");	
	GetCurrentDirectory(sizeof(szProPath),szProPath);
	lstrcat(szProPath,"\\init.ini");	

	GetPrivateProfileString("General","TestInfoFileName","TestInformation.ini",m_IniFileInfo.szTestInfoFileName, MINBUFSIZE,szProPath);
	GetPrivateProfileString("General","ServerFilePath","",m_IniFileInfo.szServerFilePath, MINBUFSIZE,szProPath);
	//change place to funtion LinkToMydas()
//	GetPrivateProfileString("General","MydasVersion","",m_IniFileInfo.szMydasVersion, MINBUFSIZE,szProPath);
	return 1;
}
bool CRunInfo::DutCheckStringTableChecksum(void)
{
	/*
	true	:pass
	false	:failed
	*/
	DisplayRunTimeInfo("---Start check String table checksum---");
	//check if need test or not
	if( 0 == m_IniFileInfo.nCheckSumFlag )
	{
		DisplayRunTimeInfo("Don't need check String table checksum!");
		return true;
	}	

	//send DUT command to get DUT string table checksum
	char szBuf[MINBUFSIZE];
	sprintf_s(szBuf , sizeof(szBuf),"getchksum %d" , m_IniFileInfo.nCheckSumNum);
	if(!SendDutCommand(szBuf,"#",5000)) return false;
	//check DUT's checksum with ini file
	for(int iTime=1 ; iTime <= m_IniFileInfo.nCheckSumNum ; iTime++)
	{
		char Info[MINBUFSIZE] = "";
		char DestBuffer[MINBUFSIZE] = "";
		const char* flag=NULL;
		sprintf_s(Info,"chksum%d=",iTime);
		if((flag=strstr(g_data,Info))!=NULL )
		{
			strncpy_s(DestBuffer,MINBUFSIZE,flag+8,10);
			//AfxMessageBox(DestBuffer);
			if( (strstr(DestBuffer,m_IniFileInfo.szCkecksum[iTime]) == NULL) ||
				(strcmp("",m_IniFileInfo.szCkecksum[iTime]) == 0) ||
				(strlen(m_IniFileInfo.szCkecksum[iTime] ) == 0)
			   )
			{

				return false;
			}

		}
		else
		{
			return false;
		}
	}
	return true;
}

int CRunInfo::SettingGoldenServer( int iChannel )
{
	/*
	-1	:pass
	2	:Get 2G link status failed 
	3	:Get 5G link status failed 
	*/
	CString strSSID;
	GetDlgItemText(IDC_EDIT_SSID, strSSID);

	if(iChannel > 14)//5G
	{
		char szSSIDCommand[MINBUFSIZE] = "";

		if(!SendGoldenCommand("wl band a" , "ok" , 5))
		{		
			return 3; //Get 5G link status failed  
		}
		if(!SendGoldenCommand("wl up" , "ok" , 5))
		{
			return 3; //Get 5G link status failed  
		}
		sprintf_s(szSSIDCommand, MINBUFSIZE, "startconn %s %d", strSSID , (m_IniFileInfo.nTimeOut));
		if(!SendGoldenCommand(szSSIDCommand , "success" , m_IniFileInfo.nTimeOut))
		{
			return 3; //Get 5G link status failed  
		}
	}
	else//2.4G
	{
		char szSSIDCommand[MINBUFSIZE] = "";
		if(!SendGoldenCommand("wl band b" , "ok" , m_IniFileInfo.nTimeOut))
		{		
			return 2; //Get 2G link status failed  
		}
		if(!SendGoldenCommand("wl up" , "ok" , m_IniFileInfo.nTimeOut))
		{
			return 2; //Get 2G link status failed  
		}
		sprintf_s(szSSIDCommand, MINBUFSIZE, "startconn %s %d", strSSID , m_IniFileInfo.nTimeOut);
		if(!SendGoldenCommand(szSSIDCommand , "success" , m_IniFileInfo.nTimeOut))
		{
			return 2; //Get 2G link status failed  
		}
	}
	return -1;
}

int CRunInfo::SettingDUT(int iChannel , int iChipsetIndex)
{
	/*
	-1	:pass
	1	:send dut setting command failed
	*/
	CString strSSID;
	GetDlgItemText(IDC_EDIT_SSID, strSSID);

	if(iChannel > 14)//5G
	{			
		SendDutCommand("wl -i eth2 up","#",2000);
		if(!SendDutCommand("nvram set wlg_wlanstate=Enable","#",5000))		return 1;	
		if(!SendDutCommand("nvram set wla_wlanstate=Disable","#",5000))		return 1;
		char szSSID[512]="";
		sprintf_s(szSSID,sizeof(szSSID),"nvram set wlg_ssid=%s",m_IniFileInfo.sz5GThrotghputGoldenSSID);
		if(!SendDutCommand(szSSID,"#",5000))	return 1;			
		char szChan[64]="";
		sprintf_s(szChan,sizeof(szChan),"nvram set wlg_channel=%d",iChannel);
		if(!SendDutCommand(szChan,"#",5000))	return 1;
		if(!SendDutCommand("wlg_mode=HT80","#",5000))	return 1;
		if(!SendDutCommand("nvram set wlg_secu_type=None","#",5000))	return 1;
		if(!SendDutCommand("rc wlanrestart","#",5000))	return 1;
		//if(!SendDutCommand("killall -SIGUSR1 wlanconfigd","#",5000))	return 1;
	}
	else//2.4G
	{
		SendDutCommand("wl up","#",2000);
		if(!SendDutCommand("nvram set wla_wlanstate=Enable","#",5000))		return 1;
		if(!SendDutCommand("nvram set wlg_wlanstate=Disable","#",5000))		return 1;
		if(!SendDutCommand("nvram set wl0_obss_coex=0","#",5000)) return 1;
		char szSSID[512]="";
		sprintf_s(szSSID,sizeof(szSSID),"nvram set wla_ssid=%s",m_IniFileInfo.sz2GThrotghputGoldenSSID);
		if(!SendDutCommand(szSSID,"#",5000))	return 1;			
		char szChan[64]="";
		sprintf_s(szChan,sizeof(szChan),"nvram set wla_channel=%d",iChannel);
		if(!SendDutCommand(szChan,"#",5000))	return 1;
		if(!SendDutCommand("nvram set wla_mode=300Mbps","#",5000))	return 1;	
		if(!SendDutCommand("nvram set wla_secu_type=None","#",5000))	return 1;
		if(!SendDutCommand("rc wlanrestart","#",5000))	return 1;	
		//if(!SendDutCommand("killall -SIGUSR1 wlanconfigd","#",5000))	return 1;
	}	
	return -1;
}

bool CRunInfo::Check_Passphrase(char * pszPWD)
{
	/*
	true	:pass
	false	:failed
	*/	
    char szPwdVal[256]="";
	strcpy(szPwdVal,pszPWD);
	/*
	Add shorted adjectives to list, the lenght change to 200
	*/
	for(int i=0;i<200;i++)
	{
		int iAdjLen=strlen(Adjectives[i]);
		if(strncmp(szPwdVal, Adjectives[i],iAdjLen) == 0)
		{
			char szTemp[256]="";
			strcpy(szTemp,szPwdVal+iAdjLen);

			/*
			Add shorted Nouns to list, the lenght change to 200
			*/
			for(int j=0;j<200;j++)
			{
				int iNouLen=strlen(Nouns[j]);
				if(strncmp(szTemp,Nouns[j],iNouLen) == 0)
				{
					char szTempNum[256]="";
					strcpy(szTempNum,szTemp+iNouLen);
					if(strlen(szTempNum) == 3)
					{
						if((szTempNum[0]>='0' && szTempNum[0]<='9') &&
							(szTempNum[1]>='0' && szTempNum[1]<='9') &&
							(szTempNum[2]>='0' && szTempNum[2]<='9') )
						{
							return true;
						}
						else
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}
			}
		}
	}
	return false;
}

bool CRunInfo::Check_SSIDFormat(char * pszSSID)
{
	/*
	true	:pass
	false	:failed
	*/
	char szSSIDVal[256]="";
	strcpy(szSSIDVal, pszSSID);
	int iTempLen = strlen("NETGEAR");
	if(strncmp(szSSIDVal, "NETGEAR",iTempLen) == 0)
	{
		char szTempNum[256]="";
		strcpy(szTempNum,szSSIDVal+iTempLen);
		if(strlen(szTempNum) == 2)
		{
			if((szTempNum[0]>='0' && szTempNum[0]<='9') &&				
				(szTempNum[1]>='0' && szTempNum[2]<='9') )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else if(strlen(szTempNum) == 5)
		{
			if((szTempNum[0]>='0' && szTempNum[0]<='9') &&				
			   (szTempNum[1]>='0' && szTempNum[2]<='9') &&
			   (szTempNum[2]=='-')&&
			   (szTempNum[3]=='5')&&
			   (szTempNum[4]=='G'))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return false;
}

char* CRunInfo::GetValueBetween(CString strBuffer,char*szBeginWord, char* szEndWord)
{
    CString strBuf = strBuffer.Trim();
	char szBuf[512];
	sprintf_s(szBuf, 512, "%s", strBuf);

    char  strRtv[512];
	
	int iBeginPos = strBuf.Find(szBeginWord);
    int iEndPos = strBuf.Find(szEndWord);

	strncpy_s(strRtv, sizeof(strRtv), &szBuf[iBeginPos + strlen(szBeginWord)], iEndPos-iBeginPos-strlen(szBeginWord));

    return strRtv;
}

float CRunInfo::__RandResult(float throughput_value , float throughput_spec_customer , float throughput_spec_our , float throughput_rand)
{
    //if throughput_value > throughput_spec_customer,return
    if(throughput_value > throughput_spec_customer)
    {
       return  throughput_value ;
    }
    //if throughput_value < throughput_spec_our,return
    if(throughput_value < throughput_spec_our)
    {
       return  throughput_value ;
    }
	
	//default rand range,if no this,may cause all range data is the same.
	srand( (unsigned)time( NULL ) );

    //if  throughput_value < throughput_spec_customer,return [throughput_spec_customer,throughput_spec_customer + throughput_rand ]
    int RANGE_MIN = 1;
    int RANGE_MAX = throughput_rand;

    float rand100 = (((double) rand() / (double) RAND_MAX) * RANGE_MAX + RANGE_MIN);

	throughput_value = rand100 + throughput_spec_customer;

    return  throughput_value ;
}

int CRunInfo::CheckButtonRelease()
{
	int iReleaseState = 0 ;
	
	int iRty = 5 ;
	while(iRty--)
	{
		SendDutCommand("\r\n","#",5000);
		if((strstr(g_data, RST_BTN_VALUE) == NULL) && (strstr(g_data, WPS_BTN_VALUE) == NULL) && (strstr(g_data, WIFI_BTN_VALUE) == NULL))
		{
			iReleaseState = 0;
			break;
		}
		if(strstr(g_data, RST_BTN_VALUE) != NULL)
		{
			iReleaseState = 1;//Reset button
			AfxMessageBox("請不要一直按壓Reset Button！\r\nPlease release Release Reset Button！");
			Sleep(500);
			continue ;
		}
		
		if(strstr(g_data, WPS_BTN_VALUE) != NULL)
		{
			iReleaseState = 2;//WPS button
			AfxMessageBox("請不要一直按壓WPS Button！\r\nPlease release Release WPS Button！");
			Sleep(500);
			continue ;
		}
		
		if(strstr(g_data, WIFI_BTN_VALUE) != NULL)
		{
			iReleaseState = 3;//WIFI button
			AfxMessageBox("請不要一直按壓WIFI Button！\r\nPlease release Release WIFI Button！");
			Sleep(500);
			continue ;
		}	
	}
	return iReleaseState;
}
bool CRunInfo::DutCheckRouteInfo(void)
{
	/*
	true	:pass
	false	:false
	*/
	DisplayRunTimeInfo("---Start check Dut Routerinfo---");

	char szDutFWVersion[MINBUFSIZE];
	char szDutCFTVersion[MINBUFSIZE];
	char szDutLanMac[MINBUFSIZE];
	char szDutPinCode[MINBUFSIZE];
	char szDutSerialNo[MINBUFSIZE];
	char szDut2gSSID[MINBUFSIZE];
	char szDut2gPASS[MINBUFSIZE];
	char szDut5gSSID[MINBUFSIZE];
	char szDut5gPASS[MINBUFSIZE];
	char szDutBoardId[MINBUFSIZE];
	char szDutRegionCode[MINBUFSIZE];	

	sprintf_s(szDutFWVersion, MINBUFSIZE, "%s", m_IniFileInfo.szStoreFirmware);
	sprintf_s(szDutCFTVersion, MINBUFSIZE, "CFE version : %s", m_IniFileInfo.szStoreBootCode);
	sprintf_s(szDutLanMac, MINBUFSIZE, "LAN mac address - %s", m_strMAC);
	sprintf_s(szDutPinCode, MINBUFSIZE, "WSC PIN - %s", m_strPincode);
	sprintf_s(szDutSerialNo, MINBUFSIZE, "serial number - %s", m_strSSN);
	sprintf_s(szDut2gSSID, MINBUFSIZE, "SSID - %s", m_strSSID_2G);
	sprintf_s(szDut2gPASS, MINBUFSIZE, "Passphrase - %s", m_strPASS_2G);
	sprintf_s(szDut5gSSID, MINBUFSIZE, "SSID_5G - %s", m_strSSID_5G);
	sprintf_s(szDut5gPASS, MINBUFSIZE, "Passphrase_5G - %s", m_strPASS_5G);
	sprintf_s(szDutBoardId, MINBUFSIZE, "Board ID - %s", m_IniFileInfo.szStoreBoardID);
	sprintf_s(szDutRegionCode, MINBUFSIZE, "region_num - %s", m_IniFileInfo.szRegionCode);


	if(!SendDutCommand("routerinfo","#",10000)) 
	{
		return false;
	}

	DisplayRunTimeInfo("Check DUT FW Version");
	if(strstr(g_data, szDutFWVersion) == NULL)
	{
		SendSfisResultAndShowFail(CC70);
		return false;		
	}

	DisplayRunTimeInfo("Check DUT CFT Version");
	if(strstr(g_data, szDutCFTVersion) == NULL)
	{
		SendSfisResultAndShowFail(CC20);
		return false;		
	}

	DisplayRunTimeInfo("Check DUT Lan MAC");
	if(strstr(g_data, szDutLanMac) == NULL)
	{
		SendSfisResultAndShowFail(CC10);
		return false;		
	}

	DisplayRunTimeInfo("Check DUT PinCode");
	if(strstr(g_data, szDutPinCode) == NULL)
	{
		SendSfisResultAndShowFail(CC30);
		return false;		
	}

	DisplayRunTimeInfo("Check DUT serial Number");
	if(strstr(g_data, szDutSerialNo) == NULL)
	{
		SendSfisResultAndShowFail(CC40);
		return false;		
	}

	if(strcmp(m_IniFileInfo.szIsHaveSSIDLable , "1") == 0)
	{
		DisplayRunTimeInfo("Check DUT 2.4G SSID");
		if(strstr(g_data, szDut2gSSID) == NULL)
		{
			SendSfisResultAndShowFail(CC71);
			return false;		
		}

		DisplayRunTimeInfo("Check DUT 2.4G PassPhrase");
		if(strstr(g_data, szDut2gPASS) == NULL)
		{
			SendSfisResultAndShowFail(CC72);
			return false;		
		}

		DisplayRunTimeInfo("Check DUT 5G SSID");
		if(strstr(g_data, szDut5gSSID) == NULL)
		{
			SendSfisResultAndShowFail(CC73);
			return false;		
		}

		DisplayRunTimeInfo("Check DUT 5G PassPhrase");
		if(strstr(g_data, szDut5gPASS) == NULL)
		{
			SendSfisResultAndShowFail(CC74);
			return false;		
		}
	}

	DisplayRunTimeInfo("Check DUT BoardID");
	if(strstr(g_data, szDutBoardId) == NULL)
	{
		SendSfisResultAndShowFail(CC60);
		return false;		
	}

	if(strstr(m_IniFileInfo.szRegionCodeFlag, "1") != NULL)
	{
		DisplayRunTimeInfo("Check DUT Region Code");
		if(strstr(g_data, szDutRegionCode) == NULL)
		{
			SendSfisResultAndShowFail(CC50);
			return false;		
		}
	}

	return true;
}