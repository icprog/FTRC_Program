#define SY00		_T("SY00 Test runtime fail... ")
#define SY05		_T("SY05 Connect with server&copy file failed... 	")
#define SY15    	_T("SY15 Compare product name failed...	")
#define SY25    	_T("SY25 Arp DUT virtual mac failed... 	")
#define SY35    	_T("SY35 Run Telnet  failed...      	")
#define SY45    	_T("SY45 Read ini or tab file failed...	")
#define SY65    	_T("SY65 Communicate with instrument failed...")
#define SY75        _T("SY75 Setting/Get wan port(IP) failed...")
#define SR00    	_T("SR00 SFC response error... ")
#define BR00    	_T("BR00 Barcode error...      ")
#define IN10     	_T("IN10 Ping DUT failed...    ")

#define IN50     	_T("IN50 Ping DUT wan port failed...    ")

#define IN12     	_T("IN12 Ping DUT lan port(Lan_IP_2) failed...    ")
#define IN13     	_T("IN13 Ping DUT lan port(Lan_IP_3) failed...    ")
#define IN14     	_T("IN14 Ping DUT lan port(Lan_IP_4) failed...    ")

#define IN20    	_T("IN20 Telnet login failed...")
#define IN30     	_T("IN30 Ethernet cable not insert...    ")
#define WC10        _T("WC10 Write MAC ID failed...")
#define WC90   	    _T("WC90 Write SSID failed...  ")
#define WC30        _T("WC30 Write Pin code failed...")
#define WC40        _T("WC40 Write CSN failed...")
#define WC50        _T("WC50 Write Region Code failed...")
#define WC70        _T("WC70 Brun RF parameters failed..." )

#define WC71        _T("WC71 Write 2.4G SSID failed...")
#define WC72        _T("WC72 Write 2.4G PASS failed...")
#define WC73        _T("WC73 Write 5G SSID failed...")
#define WC74        _T("WC74 Wirte 5G PASS failed..." )


#define CC10    	_T("CC10 Check MAC ID failed...")
#define CC20    	_T("CC20 Check CFT Version failed...")
#define CC30    	_T("CC30 Check Pin code failed...")
#define CC40    	_T("CC40 Check Serial NO. failed...")
#define CC50    	_T("CC50 Check Region Code failed...")
#define CC60    	_T("CC60 Check BoardID failed...")
#define CC70    	_T("CC70 Check Version failed...")

#define CC71    	_T("CC71 Check 2.4G SSID failed...")
#define CC72    	_T("CC72 Check 2.4G PASS failed...")
#define CC73    	_T("CC73 Check 5G SSID failed...")
#define CC74    	_T("CC74 Check 5G PASS failed...")

#define CC75    	_T("CC75 2G SSID format error...")
#define CC76    	_T("CC76 2G PASS format error...")
#define CC77    	_T("CC77 5G SSID format error...")
#define CC78    	_T("CC78 5G PASS format error...")


#define CC80		_T("CC80 Check string table checksum faile...")

#define CC90    	_T("CC90 Check IMEI failed...")
#define DS10    	_T("DS10 Disable Wireless failed...")

#define SY55    	_T("SY55 Send dut setting command failed...")

//throughput test
#define EH50    	_T("EH50 Get 5G link status failed...")
#define EH20    	_T("EH20 Get 2G link status failed...")

#define EH25    	_T("EH25 Ping 5G golden failed...")
#define EH22    	_T("EH22 Ping 2G golden failed...")

#define TP51    	_T("TP51 5G TX+RX LOWER THAN SPEC...")
#define TP21    	_T("TP21 2G TX +RX LOWER THAN SPEC...")

#define TP56    	_T("TP56 5G TX+RX HIGTER THAN SPEC...")
#define TP26    	_T("TP26 2G TX+RX HIGTER THAN SPEC...")

#define TP52    	_T("TP52 5G TX LOWER THAN SPEC...")
#define TP22    	_T("TP22 2G TX LOWER THAN SPEC...")

#define TP53    	_T("TP53 5G RX LOWER THAN SPEC...")
#define TP23    	_T("TP23 2G RX LOWER THAN SPEC...")

#define TP55    	_T("TP55 5G TX/RX HIGTER THAN SPEC...")
#define TP25    	_T("TP25 2G TX/RX HIGTER THAN SPEC...")

#define TP50     	_T("TP50 Test DUT lan to wan throughput failed...    ")

#define TP12     	_T("TP12 Test DUT lan to (Lan_IP_2) throughput failed...    ")
#define TP13     	_T("TP13 Test DUT lan to (Lan_IP_3) throughput failed...    ")
#define TP14     	_T("TP14 Test DUT lan to (Lan_IP_4) throughput failed...    ")

#define IP00        _T("IP00 Exception happen in throughput test process...")

//LED 
#define LD00        _T("LD00 LED lighting on failed...")
#define LD10        _T("LD10 WPS LED lighting on failed...")

#define LD20        _T("LD20 DUT LED lighting on Green failed...")
#define LD30        _T("LD30 DUT LED lighting on Amber failed...")
#define LD40        _T("LD40 DUT LED lighting on Red failed...")

#define EH30    	_T("EH30 Get Golden connection state failed...")
#define OS10    	_T("OS10 POT function failed...")
#define OS20    	_T("OS20 Start POT function failed...")
#define OS30    	_T("OS30 Check POT time failed...")
#define OS40    	_T("OS40 Erase POT time failed...")
#define OS50    	_T("OS50 loaddefault failed...")

//button check
#define BC10    	_T("BC10 Check power switch failed...")

#define BC20    	_T("BC20 Check Reset button failed...")
#define BC30    	_T("BC30 Check WPS button failed...")
#define BC40    	_T("BC40 Check WiFi button failed...")

#define BC21    	_T("BC21 Before test button Check Reset button failed...")
#define BC31    	_T("BC31 Before test button Check WPS button failed...")
#define BC41    	_T("BC41 Before test button Check WiFi button failed...")

#define BC22    	_T("BC22 After test button Check Reset button failed...")
#define BC32    	_T("BC32 After test button Check WPS button failed...")
#define BC42    	_T("BC42 After test button Check WiFi button failed...")

#define CR00        _T("CR00 Check RX current failed... ")
#define DS30        _T("DS30 3Kft adsl can not connect terminate")
#define DS31        _T("DS31 3Kft adsl downstream test fail")
#define DS32        _T("DS32 3Kft adsl Upstream test fail")
#define DS50        _T("DS50 15Kft adsl can not connect terminate")
#define DS51        _T("DS51 15Kft adsl downstream test fail")
#define DS52        _T("DS52 15Kft adsl Upstream test fail")
#define SC00		_T("SC00 Test SIM card failed...")
#define C300		_T("C300 Check 3G Module failed...")
#define	MO00		_T("MO00 Check module infor failed...")
#define	BL00		_T("BL00 Burn Lock code fail...")
#define RS01        _T("RS01 MAIN Rssi test fail...")
#define RS02        _T("RS02 With out antenna test fail...")
#define RS03        _T("RS03 Aux Rssi test fail...")
#define WR00        _T("WR00 SIM Read and Write test fail... ")
#define LD00        _T("LD00 LED lighting on failed...")
//USB test 
#define UB00        _T("UB00 USB DECT failed...")
#define UB01        _T("UB01 At USB no test file failed...")
#define UB02        _T("UB02 ftp process not start...")
#define UB03        _T("UB03 Delete USB_Result_File failed...")
#define UB04        _T("UB04 USB_TX_ThroughPut_RUN_File not exist...")
#define UB05        _T("UB05 USB_RX_ThroughPut_RUN_File not exist...")
#define UB06        _T("UB06 TimeOut too shorter for find test windows...")
#define UB07        _T("UB07 TimeOut too shorter for test finished...")
#define UB08        _T("UB08 USB TX/RX throughput test failed...")
#define UB09        _T("UB09 USB TX throughput test failed...")
#define UB10        _T("UB10 USB RX throughput test failed...")
