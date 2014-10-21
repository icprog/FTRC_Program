// MainFrm.h : interface of the CMainFrame class
//
/*
V1.0.0	2011/2/19
		1.Change program from H153
V1.0.1	2011/4/1
		1.modfily recode log bug
		2.modfily SFC bug
V1.0.2	2011/4/2
		1.add golden server
V1.0.3	2011/4/6
		1.Add throught higt SPEC
V1.0.4	2011/4/14
		1.change WPS LED test way
		2.Change program support two different IP Address
		3.Use GetAdapterInfo.dll to get DUT connection
V1.0.5	2011/4/15
		1.Auto press button
V1.0.6	2011/4/18
		1.Use GoldenSocket to get golden connect falg
V1.0.6	2011/5/4
		1.Use new .dll(GetAdapterInfo.dll) which add "arp -d"
V1.0.6	2011/5/19			--->U12H163
		1.FT add burn region code,RC add check
		2.FT add throught test cammand
V1.0.7	2011/10/29
		1.change U12H197 FT&RC program from U12H163
		2.change UI program(Program support different channel set from UI)
		3.change "DutTestWiFiThroughput()" to "DutTestWiFiThroughput_1" and "DutTestWiFiThroughput_2" used to test different chipset
		4.at RunThroughputTest add chipset marker
		5.at "init.ini" marker program use setting file name and file path at erver
		6.at "DUT_LAN_IP.ini" marker DUT lan port AP's IP address
V1.0.8	2011/11/28
		1.change WPS LED test funtion
		2.loaddefault funtion add check return value(20111125 SW support)
V1.0.1.0	2011/11/28
		1.use new program version rule:change"V1.0.8	2011/11/28" to "V1.0.1.0	2011/11/28"
		2.program UI version title add show built time(Use funtion "_T(__DATE__" and "_T(__TIME__)" )
		3.Add flag:szRegionCodeFlag marker burn region code yes or not
V1.1.1.0	2011/11/30
		1.Add funtion:DutCheckStringTableChecksum() to check string table checksum
		2.funtion:RunThroughputTest(int iChannel , int iChipsetIndex) aport to different funtion
		  (Golden setting funtion/DUT setting funtion)
		3.Change chipset 1 and chipset 2 have different SPEC
		4.change funtion:PingSpecifyIP() add parameter:PacketCount/TotalCount/SuccessCount 	
V1.2.1.0	2011/12/08
		1.a command to be changed. ("rc wlanrestart" --> "read_bd; rc restart")	--->20111208 SW change
V1.2.1.0	2011/12/12
		1.bug:at funtion PingSpecifyIP() parameter:TotalCount not work,cannel this parameter
		2.add command "nvram set wla_obss_coex=0" before 5358U throughput test
V1.3.1.0	2012/2/13
		1.add "CompareProgram.h" and "CompareProgram.cpp" for check program lastwirtetime and 
		  checksum then copy server file if they are different!
		  please look detail infarmation at function:AutoDownLoadProgram()
V1.0.1.0	2012/4/7
		1.change from H197 to R6300
V1.0.1.0		2012/5/10
		1.wifi_throughput test result offset program
		2.lan_wan throughput test result offset program
		3.bug:when burn 5G ssid,parameter is SFC's 5G SSID,but used wrong 5G passphrase
V1.0.1.0		2012/7/25
		1.add "obss" setting commaond
V1.0.1.0		2012/9/05 (not online)
		1.change FT throughput test command
		  ( rc wlanrestart --> killall -SIGUSR1 wlanconfigd)
		2.change FT throughput test flow -->debug throughput link issue
		  (2G->5G --> 5G->2G)
V1.0.1.0		2012/11/15 
		1.add function:CheckButtonRelease() to check button release state
V1.0.2.0		2012/12/08
		1.change for R6300 to R6300v2
		2.change check NETGEAR logo LED station
V1.0.2.0		2013/07/28
		1.bug:at function:bool CRunInfo::DutCheckRegion(void) which used check region code,the test flag use wrong one.
		right: szRegionCodeFlag		  wrong:nCheckSumFlag
V1.0.2.0		2013/08/12
		1.add check bootcode version function 
		  please check function : bool CRunInfo::CheckFirmwareVersion(void)
V1.0.3.0		2013/10/14
		1.FT add nvram loaddefault function
		VN OOBA¡@found:nvram parameters and board data parameters not same.
		VN TE co-work with SW and found:if add nvram loaddefault at FT station and after setting wan port, can found this issue.
		This action useful all only NAND falsh product.¡]AC1450/R6300v2/R6250/R6200v2¡^
V1.0.3.0		2013/10/16(not online)
		1.add Retry in RC LanToWan Throughput function
		2.add Check Code use "routerinfo"(check most code in one function)
		3.add check SSID format function
V1.0.3.0		2013/10/23
		1.add timeout in function:RunSpecifyExeAndRead().
*/

#pragma once

#define DIALOGUE_VERSION		"V1.0.3.0"
#define PRODUCT_NAME			"U12H240"
#define PROGRAM_TIME			"2013/10/14"

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
		DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
};


