// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PTSC_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PTSC_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifndef srcDll_H
#define srcDll_H
#ifdef PTSC_EXPORTS
#define PTSC_API  extern "C" __declspec(dllexport)
#else
#define PTSC_API  extern "C" __declspec(dllimport)
#endif

PTSC_API int __stdcall telkomGenDefaultsSSID(char *sn, char *ssid);
PTSC_API int __stdcall telkomGenDefaultsKey(char *lan_mac, char *WPAkey);

#endif
