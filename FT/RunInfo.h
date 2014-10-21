#pragma once


// CRunInfo dialog
#include "IsStartSfis.h"
#include "Winsock2.h"
#include "Com.h"
#include <fstream>
#include "afxwin.h"
#include "Fluke.h"
#include "PTSC.h"
#include "srcDll.h"
#include <io.h>
#include "GetAdapterInfo.h"

using namespace std;
#define STANDBY           0
#define RUN                     1
#define PASS                    2
#define FAIL                    3
#ifndef _RUNINFO_H_
#define _RUNINFO_H_

#define MAXBUFSIZE                      1024

struct _ButtonInfo
{
	bool bTestOK;
	char* szWindowName;
	char* szReturnInfo;
};
struct _GoldenInfo
{
	bool		bConnectOK;
	char*		szSendCom;
	char*		szReturnInfo;
	int			nDelay;
	SOCKET		GoldenSocket;
	CString		szGoldenData;
	bool		bResponeOK;

};
static	 UINT __cdecl AutoPressButton(LPVOID pParam);
static	 UINT __cdecl OnRecvGoldenData(LPVOID pParam);

class CRunInfo : public CDialog
{
	DECLARE_DYNAMIC(CRunInfo)

public:
	CRunInfo(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRunInfo();

	// Dialog Data
	enum { IDD = IDD_RUNINFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	typedef struct _Iniinfo
	{
		char        szStoreFixIPAddress[MINBUFSIZE];

		char        szStoreFirmware[MINBUFSIZE];
		char        szStoreBootCode[MINBUFSIZE];
		char        szStoreBoardID[MINBUFSIZE];

		char        szStoreProductName[MINBUFSIZE];
		char		szModuleInfor[MINBUFSIZE];//ADD
		char		szModuleVersion[MINBUFSIZE];//ADD
		char        szStoreSrvPath[MINBUFSIZE];
		char        szStoreIperfPara[MINBUFSIZE];
		char        szStoreIperfParaFAI[MINBUFSIZE];

		char        szStoreIperfPara_D[MINBUFSIZE];
		char        szStoreIperfPara_R[MINBUFSIZE];

		char		szTestInfoFileName[MINBUFSIZE];	/* 2011/10/31*/
		char		szServerFilePath[MINBUFSIZE];	/* 2011/10/31*/

		char		szStoreTestThroughput[MINBUFSIZE];
		char		szStoreTestThroughputSingle[MINBUFSIZE]; // for bi-direction throughput test 
		char        szDutImei[MINBUFSIZE];//add
		char        szLockCode[MINBUFSIZE]; //add
        char        szStoreTestRssi[MINBUFSIZE]; //add
		char        szBurnMacWithoutSFIS[MINBUFSIZE];	
		char        szStoreTestAdsl[MINBUFSIZE];
		char        szAllowBurn[MINBUFSIZE];
		char		szRework[MINBUFSIZE];
		char        szSetChannel[MINBUFSIZE];
		char        szSetSSID[MINBUFSIZE];
		char        szCheckVersion[MINBUFSIZE];
		char		szUpLoadVersion[MINBUFSIZE];//flag used mark upload FW to SFIS or not
		char		szIsHaveSSIDLable[MINBUFSIZE];//flag used mark is have SSID lable or not
		char        szDefaultIP1[MINBUFSIZE];
		char        szDefaultIP2[MINBUFSIZE];
		char        szCurrentTest[MINBUFSIZE];
		char        szPostFlag[MINBUFSIZE];
		char		szDisableWireless[MINBUFSIZE];
		char		szRegionCodeFlag[MINBUFSIZE];
		char		szRegionCode[MINBUFSIZE];

		/*MAC prefix*/
		char		szMac1[MINBUFSIZE];/*MAC1*/
		char		szMac2[MINBUFSIZE];/*MAC2*/
		char		szMac3[MINBUFSIZE];/*MAC3*/
		char		szTestStatuse[MINBUFSIZE];/*test statuse*/
		/*End*/

		int			nPingTolalCount;
		int			nPingPacketCount;
		int			nPingSuccessCount;
		
		/*ThroughPut*/

		int			  nThroughputGoldenFlag;

		int			  nRetryNum;
		int           nTXThroughputSpec;
		int           nRXThroughputSpec;
		int           nTXRXThroughputSpec;
		int           nTXThroughputSpec5G;
		int           nRXThroughputSpec5G;
		int           nTXRXThroughputSpec5G;

		int           nTXThroughputSpecHigt;
		int           nRXThroughputSpecHigt;
		int           nTXRXThroughputSpecHigt;
		int           nTXThroughputSpecHigt5G;
		int           nRXThroughputSpecHigt5G;
		int           nTXRXThroughputSpecHigt5G;

		int           nTXThroughputSpec_2;
		int           nRXThroughputSpec_2;
		int           nTXRXThroughputSpec_2;
		int           nTXThroughputSpec5G_2;
		int           nRXThroughputSpec5G_2;
		int           nTXRXThroughputSpec5G_2;

		int           nTXThroughputSpecHigt_2;
		int           nRXThroughputSpecHigt_2;
		int           nTXRXThroughputSpecHigt_2;
		int           nTXThroughputSpecHigt5G_2;
		int           nRXThroughputSpecHigt5G_2;
		int           nTXRXThroughputSpecHigt5G_2;

		
		int           nChannelNum_2G_1;
		int           nChannelNum_5G_1;
		int           nChannelNum_2G_2;
		int           nChannelNum_5G_2;

		char		  sz2GThrotghputGoldenSSID[MINBUFSIZE];
		char		  sz5GThrotghputGoldenSSID[MINBUFSIZE];
		char		  sz2GThrotghputGoldenIP[MINBUFSIZE];
		char		  sz5GThrotghputGoldenIP[MINBUFSIZE];

		int			nTXThroughputSpec_Factroy;
		int			nRXThroughputSpec_Factroy;
		int			nTXRXThroughputSpec_Factroy;

		int			nTXThroughputSpec5G_Factroy;
		int			nRXThroughputSpec5G_Factroy;
		int			nTXRXThroughputSpec5G_Factroy;

		int			nThroughputFalg_Factroy;
		int			nThroughputFalg5G_Factroy;

		int			nThroughputRange_Factroy;
		int			nThroughputRange5G_Factroy;
		/*End*/
		int           MURssiSpec;  //2010.10.6
		int           MDRssiSpec;
		int           NURssiSpec;
		int           NDRssiSpec;
		int           DeltaUPSpec;
		int           DeltaDownSpec;
		int           nDSLDownStream;
		int           nDSLUpStream;
		int           nDSLCount;
		int           nDSLCountIterval;
		int           nPingDUTCount;
		int           nPingGoldenCount;
		int           nTimeOut;
		int           nSleepOrNo;

		/*Lan Port*/
		int			  nLanPort_Num;
		char          szLanPort_IP[10][MINBUFSIZE];//DUT Lan port IP
		
		int			  nLanPort_Throughput_Flag;		
		int			  nLanPort_Throughout_SPEC_LOW;
		int			  nLanPort_Throughout_SPEC_HIGH;
		char		  szLanPort_Throughput_Iperf[MINBUFSIZE];
		/*End*/

		/*Wan Port*/
		int			  nWanPort_Flag;		
		char		  szWanPort_IP[MINBUFSIZE];
		char		  szWanPort_IP_PRE[MINBUFSIZE];

		int			  nWanPort_Throughput_Flag;		
		int			  nWanPort_Throughout_SPEC_LOW;
		int			  nWanPort_Throughout_SPEC_HIGH;
		char		  szWanPort_Throughput_Iperf[MINBUFSIZE];

		int			nWanPort_Throughput_Flag_Factroy;
		int			nWanPort_Throughout_SPEC_LOW_Factroy;
		int			nWanPort_Throughout_RANGE_Factroy;
		/*End*/

		/*USB*/	
		char          szUsbDectCmd[MINBUFSIZE];//Dect USB command 
		char          szUsbDectString[MINBUFSIZE];//Dect USB need find string in return value 
		int           nTestUsb;//Test USB port or not

		//USB1
		char          szUsbResultFile_1[MINBUFSIZE];//USB throughput test result file 	
		char          szUsbTXThroughputRunFie_1[MINBUFSIZE];//USB TX throughput test run file
		char          szUsbRXThroughputRunFie_1[MINBUFSIZE];//USB RX throughput test run file
		int           nUsbTXThroughputFlag_1;//Test USB TX throughput or not
		int           nUsbRXThroughputFlag_1;//Test USB RX throughput or not
		int           nUsbTXThroughputSpec_1;//USB throughput TX spec
		int			  nUsbRXThroughputSpec_1;//USB throughput RX spec
		char		  szUsbFilename_1[MINBUFSIZE];//at USB throughput test,must have this file in USB(usually is name:data.bin) 

		//USB2
		char          szUsbResultFile_2[MINBUFSIZE];//USB throughput test result file 		

		char          szUsbTXThroughputRunFie_2[MINBUFSIZE];//USB TX throughput test run file
		char          szUsbRXThroughputRunFie_2[MINBUFSIZE];//USB RX throughput test run file
		int           nUsbTXThroughputFlag_2;//Test USB TX throughput or not
		int           nUsbRXThroughputFlag_2;//Test USB RX throughput or not
		int           nUsbTXThroughputSpec_2;//USB throughput TX spec
		int			  nUsbRXThroughputSpec_2;//USB throughput RX spec
		char		  szUsbFilename_2[MINBUFSIZE];//at USB throughput test,must have this file in USB(usually is name:data.bin) 

		/*End*/

		/*string table checksum*/
		int			  nCheckSumFlag;//if need check or not
		int			  nCheckSumNum; //DUT checksum number

		char		szCkecksum[10][MINBUFSIZE]; //Recode ini file checksum(just support 10 string table)

	/*End*/

	}Iniinfo,*pIniinfo;

typedef int (*pFunction)(char*, char*);
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void    OnOK();
public:
	afx_msg void   OnTimer(UINT_PTR nIDEvent);
	afx_msg int    OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void   OnBnClickedBtnStart();
	afx_msg void   OnBnClickedBtnQuit();
	afx_msg void   OnCbnSelchangeCom();
private:
	afx_msg LRESULT  OnRecvComData(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT  OnRecvEthernetData(WPARAM wParam,LPARAM lParam);
	//afx_msg LRESULT  OnRecvGoldenData(WPARAM wParam,LPARAM lParam);
	afx_msg HBRUSH   OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

private:
	
	CFont			m_SupMinft, m_Minft, m_Midft, m_Maxft, m_Supft;
	CBrush			m_Redbh, m_Greenbh, m_Yellowbh, m_Grancybh, m_Blackbh;
	CIsStartSfis	m_IsStartSfis;
	CCom			m_ComSfis;
	bool			m_blIsOpenSfis;
	bool            m_blIsSfis;	
	CString         m_strIniProductName;
private:
	CString        m_strCom;
	CString        m_strSfisStatus;
	CString        m_strTestResult;
	CButton        m_BtnStart;
	CString		   m_strLockCode;
	CString        m_strRssi;
    CString        m_strMainrssi;
	CString        m_strwizoutrssi;  
	CString        m_strauxrssi;
	CString        m_strinsidrssi;
    CString        m_strtxrxvalue;
	CString        m_strimei;
	CString        m_strTestTime;
	CString        m_strErroCode;
	CButton        m_BtnQuit;
	pFunction      m_pfn;
public:
	CString     m_strRecordTestData;
	CString     m_strStoreEthernetData;
	CString		m_strStoreGoldenData;
	CString     m_strPcName;
	CString     m_strProductName;
	CString     m_strMAC;
	CString     m_strPincode;
	CString     m_strSN;

	CString     m_strSSID_2G;
	CString     m_strPASS_2G;
	CString     m_strSSID_5G;
	CString     m_strPASS_5G;

	CString     szFWforSfis;/*Save FW for SFIS*/
	CTime		m_TestTimeCount;
	Iniinfo     m_IniFileInfo;
	HANDLE      m_hEvt;
	fstream     fsPass;
	fstream     fsFail;
	ULONG       m_nLanMAC;
	ULONG       m_nWanMAC;	
	HINSTANCE   hInst;	
	bool        m_flag;	
	SOCKET		m_GoldenSocket;
	SOCKET		m_Socket;
	CString	m_strSSN;
	int		m_FlukeAddr;
	CFluke  m_fluke;
	CString m_Station;
	int     ibutton;
public:
	bool    ReadIniFile();/*Flow product name read ini file*/
public:
	bool    ReadIniFileForGeneric();/*Read ini file */
public:
	void    IsDisplayErrorCode(bool Flag);/*Display ErrorCode*/
public:
	void    CountTestResult(bool IsPass);/*Count Test Result*/
public:
	void    TestPass();/*Test pass funtion*/
public:
	void    TestFail(char* ErroCode);/*Test fail funtion*/
public:
	void	GetLockCode(char* pIMEI, char* pLockCode); 
public:
	bool    RunTelnetExe(char* GetMacValue);
public:
	bool    PingSpecifyIP(char* IP,HANDLE &handle,HANDLE &hReadPipe,HANDLE &hWritePipe);
public:
	bool    PingSpecifyIP(char* IP, int nSuccessCount);
public:
	int     RunSpecifyExeAndRead(CString& strArpOutInfo,char* RunFileName,bool bIsShow = false);
public:
	void    GetMac(IN char* source,OUT char* target);
public:
	bool    InitSocket(char* DefaultIP);
public:
	bool    InitEthernetSocket();
public:
	void    GetThroughputValue(IN char* SourceValue,OUT double& TXValue, OUT double& RXValu);
public:
	ULONG   CovertMacToNumber(TCHAR* Mac,int lenth);
public:
	void    DisplayRunTimeInfo(char* pRunInfo);
	void	DisplayRunTimeInfo(char* pRunInfo, double iData);
	void	DisplayRunTimeInfo(char* pRunInfo, char* pData);
public:
	void    SendEthernetCmd(char* SendData);
public:
	bool    CheckMac(CString strMac);
public:
    bool    sendDatatoMydas(char*iDas);
public:
	bool	SendDutCommand(char *pCmd, char *pRtn, int iDelay=1000);
public:
	char	ErrorDefine[MINBUFSIZE*2];
public:
	char	Detaillog[MAXBUFSIZE];
public:
	char	TestDetaillog[MAXBUFSIZE*10];/*recode test data*/
public:
	char	MainData[MINBUFSIZE];
public:
	char    FailValue[MINBUFSIZE];
public:
	void    SendSfisResultAndShowFail(char* ErroCode);
public:
	void    SendSfisResultAndShowPass();
public:
	void	SendWLCommand(CString& strTempSSID);
public:
	bool	CheckFirmwareVersion(void);
public:
	bool	GetSSIS(char*  szEthernetInfo,CString& str);
public:
	bool	GetSecType(char*  szEthernetInfo,CString& str);
public:
	bool	GetIsSfis();
public:
	void	DeleteSpecifyExe(char* exeName);
public:
	static  UINT __cdecl  ReadComForSfis(LPVOID param);/*SFC Thread*/
public:
	static  UINT __cdecl  FT_FunctionTestRun(LPVOID parameter);/*FT Thread*/	//Add 2011/4/18
public:
	static  UINT __cdecl  RC_FunctionTestRun(LPVOID parameter);/*RC Thread*/	//Add 2011/4/18
public:
	static	UINT __cdecl  PT2_FunctionTestRun(LPVOID parameter);/*PT2 Thread*/	//Add 2012/4/7 for R6300
public:
	static  UINT __cdecl  Debug_FunctionTestRun(LPVOID parameter);/*Debug Thread*/	//Add 2011/4/18
public:
	void RunArpDelelte(char* pszIpAddr);/*Run "arp -d"*/
public:
	bool PingDUT(char* pszIpAddr, int iCount);/*Run ping.exe*/
public:
	bool RunTelnetEnable(char* pszIpAddr);/*Run telnetEnable.exe*/
public:
	bool DutSocketConnection(char* DefaultIP);/*telnet to dut*/
public:
	bool DutCheckVer(void);/*change "/" at version to "_" for SFC*/
public:
	bool DutBurnSn(void);/*Burn Sn*/
public:
	bool DutBurnMac(void);/*BurnMac*/
public:
	bool DutBurnPin(void);/*BurnPin*/
public:
	bool DutBurnRegion(void);/*BurnRegionCode*/
public:
	bool DutCheckLedAll(void);
public:
	bool DutFixWanSetting(void);/*Fix WAN Port IP*/
public:
	bool DutTestLockCode(void);
public:
	bool DutGetIMEI(void);
public:
	bool DutClearSSID(void);/*clear SSID*/
public:
	bool DutCheckSimCard(void);
public:
	bool DutCheckSimRW(void);
public:
	bool DutCheck3gModelType(void);
public:
	bool DutCheck3gModuleVer(void);
public:
	bool DutDetect3gModule(void);
public:
	int DutTestRssi(void);
public:
	int DutTestWiFiThroughput_1(void);
	int DutTestWiFiThroughput_2(void);

/*LAN/WAN port*/
public:
	int CHECK_WAN_PORT(void);/*Fa-Hong add 120416 for check WAN port*/
	int CHECK_LAN_PORT(void);/*Fa-Hong add 120416 for check LAN port*/
	bool DutTestLanWanThroughput(void);/*Fa-Hong add 120416 for check DutTestLanWanThroughput*/
	bool DutTestLanLanThroughput(char* LAN_IP);/*Fa-Hong add 120416 for check DutTestLanWanThroughput*/
/*END*/
public:
	bool CRunInfo::DutCheckLed(void);/*Fa-Hong add 110107 for check LED*/
	bool CRunInfo::DutCheckLed_PT2(void);/* add 110107 for check LED in PT2 don't test WAN LED*/
public:
	bool CRunInfo::DutCheckButton(void);/*Fa-Hong add 110107 for check Button*/
public:
	bool CRunInfo::DutCheckButton_Auto(void);/*Fa-Hong add 110415 for check Button Auto*/
public:
	int RunThroughputTest(int iChannel , int iChipsetIndex = 1);/*Fa-Hong change 111107 for test dual band chipset(U12H197)*/
public:
	typedef struct _TestRecordinfo
	{
		CString		szRecordTime;
		CString		szRecordSN;
		CString		szRecordMac;
		CString		szRecordPincode;
		CString		szRecordSSN;
		CString		szRecordProductName;
		CString		szRecordPcName;
		CString		szRecordFirmware;
		CString		szRecordTestTime;
		CString		szRecordResult;
	}TestRecordinfo,*pTestRecordinfo;

public:
	TestRecordinfo m_TestRecordinfo;
public:
	void	InitTestRecordData(void);/*Init Test Record Data*/
public:
	void	CollectServerLogData();/*Collect Server Log Data*/
public:
	void	RecordTestLog();
public:
	CString		CustomerFirmwareVersion(char*  DutFirmwareVersion); /*change "/" to "_" for FW to SFIS*/
public:
	bool DutCheckBoardID(void);/*check boardid*/
public:
	bool DutCheckMAC(void);/*check MAC*/
public:
	bool DutCheckPin(void);/*check Pin*/
public:
	bool DutCheckSn(void);/*check SSN*/
public:
	bool DutCheckRegion(void);/*check RegionCode*/
public:
	bool DutDisableWireless(void);/*Disable wireless*/
public:
	bool PingSpecifyIPFail(char* IP, int nSuccessCount);/*just use for H163*/
public:
	bool DutCheckPowerButton(void);/*check power button*/
public:
	bool CheckDutBootUp(void);/*Check Dut BootUp*/
public:
	bool DutPot(void);/*Dut Pot time*/
public:
	bool DutLoadDefault(void);/*Dut loadDefault*/
public:
	bool DutTestAdsl();/*Dut test ADSL*/
public:
	bool LinkDataBase(void);/*Program link Mydas Database*/
public:
	bool SendDatatoMYDAS(char* DetailLog,char* ErrDef,char* SendDataBuf);/*Program send data to Mydas*/
public:
	bool InitGoldenSocket();
public:
	void SendCmdToGolden(char* pCmd);
public:
	bool SendGoldenCommand(char *pCmd, char *pRtn, int iDelay);
public:
    bool SureUnlock(LPVOID parameter);
public:
	void ShowMsg(CString strMsg);
public:
	bool Write_Code( char* BurnCmd  , char* BurnWhat , char* ReturnValue);
public:
	bool CheckWithToken(char* DutInforFromIniFile);
	bool CheckWithTokenLength(char* DutInforFromSFIS , char* KeyWord );
	bool CHECK_SSID_2G(char* pSsid);
	bool CHECK_SSID_5G(char* pSsid);
	bool CHECK_WPA_PASSEPHRASE_2G(char* pPassphrase);
	bool CHECK_WPA_PASSEPHRASE_5G(char* pPassphrase);
/*USB*/
public:
	bool DECT_USB();
	int  DutCheckUsb();
	int  USBThroughtPut_1();
	int  USBThroughtPut_2();
	int  USBThroughtPut(int usbTxFlag,
							 int usbRxFlag, 
							 char* usbFileName, 
							 char* usbResultFile,
							 char* usbTxRunFileName,
							 char* usbRxRunFileName,
							 int usbTxSpec, 
							 int usbRxSpec
							 );
/*END*/
public:
	bool NewReadIniFile();
	bool DutCheckStringTableChecksum(void);
	int  SettingGoldenServer(int iChannel);
	int  SettingDUT(int iChannel , int iChipsetIndex);
	bool PingSpecifyIP_2(char* IP, int nSuccessCount);
public:
	bool Check_Passphrase(char * pszPWD);
	bool Check_SSIDFormat(char * pszSSID);
	char* GetValueBetween(CString strBuffer,char*szBeginWord, char* szEndWord);
	float __RandResult(float throughput_value , float throughput_spec_customer , float throughput_spec_our , float throughput_rand);
	int CheckButtonRelease();
	
	bool DutCheckRouteInfo(void);
};

int fnCompareFileWriteTime(char* lpFilename1, char* lpFilename2);

#define C_MAX_KEY_STR_SIZE                  32

/*
Add 100 shorted Adjective 
*/
static char Adjectives[][C_MAX_KEY_STR_SIZE] = 
{
    "ancient",      "aquatic",      "basic",    "black",        "blue",
    "brave",        "breezy",       "bright",   "calm",         "cheerful",
    "classy",       "clever",       "cloudy",   "crispy",       "curly",
    "daily",        "delightful",   "dizzy",    "dynamic",      "elated",
    "elegant",      "excited",      "exotic",   "fancy",        "fearless",
    "festive",      "fluffy",       "fresh",    "friendly",     "fuzzy",
    "gentle",       "gigantic",     "graceful", "grand",        "great",
    "green",        "happy",        "heavy",    "helpful",      "husky",
    "icy",          "imaginary",    "jagged",   "jolly",        "joyous",
    "kind",         "large",        "little",   "lively",       "lucky",
    "magical",      "melodic",      "mighty",   "misty",        "modern",
    "narrow",       "new",          "nifty",    "noisy",        "odd",
    "orange",       "pastel",       "perfect",  "phobic",       "pink",
    "precious",     "purple",       "quaint",   "quick",        "quiet",
    "rapid",        "red",          "rocky",    "round",        "royal",
    "rustic",       "shiny",        "silent",   "silky",        "silly",
    "smiling",      "slow",         "smooth",   "strong",       "sunny",
    "sweet",        "thirsty",      "tiny",     "thoughtful",   "uneven",
    "unusual",      "vanilla",      "vast",     "watery",       "wide",
    "witty",        "wonderful",    "yellow",   "young",        "zany",
	"ancient",      "aquatic",      "basic",    "black",        "blue",
    "brave",        "breezy",       "bright",   "calm",         "cool",
    "classy",       "clever",       "cloudy",   "crispy",       "curly",
    "daily",        "free",			"dizzy",    "dynamic",      "elated",
    "elegant",      "excited",      "exotic",   "fancy",        "bold",
    "festive",      "fluffy",       "fresh",    "chummy",     "fuzzy",
    "gentle",       "hungry",		"gifted",	"grand",        "great",
    "green",        "happy",        "heavy",    "helpful",      "husky",
    "icy",          "manic",		"jagged",   "jolly",        "joyous",
    "kind",         "large",        "little",   "lively",       "lucky",
    "magical",      "melodic",      "mighty",   "misty",        "modern",
    "narrow",       "new",          "nifty",    "noisy",        "odd",
    "orange",       "pastel",       "perfect",  "phobic",       "pink",
    "polite",		"purple",       "quaint",   "quick",        "quiet",
    "rapid",        "red",          "rocky",    "round",        "royal",
    "rustic",       "shiny",        "silent",   "silky",        "silly",
    "smiling",      "slow",         "smooth",   "strong",       "sunny",
    "sweet",        "thirsty",      "tiny",     "deep",			"uneven",
    "unusual",      "vanilla",      "vast",     "watery",       "wide",
    "witty",        "windy",		"yellow",   "young",        "zany",
};

/*
Add 100 shorted Nouns 
*/
static char Nouns[][C_MAX_KEY_STR_SIZE] = 
{
    "airplane",     "apple",        "balloon",  "banana",       "breeze",
    "bird",         "boat",         "box",      "bug",          "butter",
    "cartoon",      "canoe",        "carrot",   "cello",        "chair",
    "cheese",       "coconut",      "comet",    "curtain",      "cream",
    "daisy",        "diamond",      "earth",    "elephant",     "fire",
    "flamingo",     "flower",       "flute",    "giant",        "grasshopper",
    "hat",          "hill",         "iris",     "ink",          "jade",
    "jungle",       "kangaroo",     "kayak",    "lake",         "lightning",
    "lotus",        "mango",        "mint",     "moon",         "mountain",
    "nest",         "ocean",        "onion",    "octopus",      "orchestra",
    "owl",          "lotus",        "mango",    "phoenix",      "piano",
    "pineapple",    "planet",       "pond",     "potato",       "prairie",
    "quail",        "rabbit",       "raccoon",  "raven",        "river",
    "road",         "rosebud",      "sea",      "ship",         "shoe",
    "shrub",        "skates",       "sky",      "socks",        "sparrow",
    "spider",       "squash",       "squirrel", "star",         "street",
    "sun",          "table",        "teapot",   "trail",        "train",
    "tree",         "tomato",       "trumpet",  "tuba",         "tulip",
    "umbrella",     "unicorn",      "valley",   "vase",         "violet",
    "violin",       "water",        "window",     "wind",       "zoo",
	"jet",			"apple",        "balloon",  "banana",       "breeze",
    "bird",         "boat",         "box",      "bug",          "butter",
    "cartoon",      "canoe",        "carrot",   "cello",        "chair",
    "cheese",       "coconut",      "comet",    "curtain",      "cream",
    "daisy",        "diamond",      "earth",    "panda",		"fire",
    "finch",		"flower",       "flute",    "giant",        "gadfly",
    "hat",          "hill",         "iris",     "ink",          "jade",
    "jungle",       "hippo",		"kayak",    "lake",         "bolt",
    "lotus",        "mango",        "mint",     "moon",         "mesa",
    "nest",         "ocean",        "onion",    "octopus",      "oboe",
    "owl",          "lotus",        "mango",    "phoenix",      "piano",
    "plum",			"planet",       "pond",     "potato",       "prairie",
    "quail",        "rabbit",       "raccoon",  "raven",        "river",
    "road",         "rosebud",      "sea",      "ship",         "shoe",
    "shrub",        "skates",       "sky",      "socks",        "sparrow",
    "spider",       "squash",       "sheep",	"star",         "street",
    "sun",          "table",        "teapot",   "trail",        "train",
    "tree",         "tomato",       "trumpet",  "tuba",         "tulip",
    "unit",			"unicorn",      "valley",   "vase",         "violet",
    "violin",       "water",        "window",     "wind",       "zoo",
};


#endif
