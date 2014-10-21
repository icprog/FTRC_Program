#include "stdafx.h"

#include "Com.h"

/////////////////////////////////////////// 
CCom::CCom()
{
	
	m_bOpened = FALSE;
	
	m_dwInBuf = 1024;
	
	m_dwOutBuf = 512;
	
	m_hCom = NULL;

	m_osReader.hEvent = NULL;

	m_osWriter.hEvent = NULL;
	
} 

void CCom::PreOpenSetupQueue(DWORD dwInQueue, DWORD dwOutQueue)
{
	
	m_dwInBuf = dwInQueue;
	
	m_dwOutBuf = dwOutQueue;
	
}

BOOL CCom::Open(char* pPort, int nBaud)
{
	
	ASSERT(nBaud >= 110 || nBaud <= 128000);
	
	if(m_bOpened)
	{
		return TRUE;
	}
		
//	char lpDef[15];
	
	DCB dcb = {0};
	
	dcb.DCBlength = sizeof(dcb);
		
//	wsprintf(lpDef, "%d,n,8,1", nBaud);
	
	m_hCom = CreateFile(pPort, GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, NULL);

	if(m_hCom == INVALID_HANDLE_VALUE ) 
		return FALSE;

	PurgeComm(m_hCom,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR); 
		
	FillMemory(&m_osReader, sizeof(OVERLAPPED), 0);
	
	FillMemory(&m_osWriter, sizeof(OVERLAPPED), 0);
	
	m_osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	m_osWriter.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
//	int byteUsedTime = 14400 / nBaud +1;
	
//	COMMTIMEOUTS timeouts = {20 + byteUsedTime, byteUsedTime, 1000, byteUsedTime, 20};
	
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout        = 500;
	timeouts.ReadTotalTimeoutConstant   = 1000;
	timeouts.ReadTotalTimeoutMultiplier = 1000;
	timeouts.WriteTotalTimeoutConstant  = 1000;
	timeouts.WriteTotalTimeoutMultiplier= 1000;

/*	dcb.fParity = TRUE;
	
	dcb.fErrorChar = TRUE;
	
	dcb.ErrorChar = '~'; */

	GetCommState(m_hCom,&dcb);

	dcb.BaudRate = nBaud;

	dcb.ByteSize = 8;

	dcb.Parity   = NOPARITY;

	dcb.StopBits = ONESTOPBIT;
	
	if( m_osReader.hEvent == NULL || m_osWriter.hEvent == NULL
		
		|| !SetCommTimeouts(m_hCom, &timeouts) 
		
		|| !SetCommState(m_hCom, &dcb) || !SetupComm(m_hCom, m_dwInBuf, m_dwOutBuf))
	{
		
		if( m_osReader.hEvent != NULL )
			
			CloseHandle( m_osReader.hEvent );
		
		if( m_osWriter.hEvent != NULL )
			
			CloseHandle( m_osWriter.hEvent );
		
		CloseHandle( m_hCom );
		
		return FALSE;
		
	}
	
	m_bOpened = TRUE;
	
	Sleep(500);

	return m_bOpened;
	
}

void CCom::SetParentWnd(HWND hWnd)
{
	m_hParentWnd = hWnd;
}

BOOL CCom::SetupQueue(DWORD dwInQueue, DWORD dwOutQueue)
{
	
	if (m_hCom == NULL) return FALSE;
	
	m_dwInBuf = dwInQueue;
	
	m_dwOutBuf = dwOutQueue;
	
	return SetupComm(m_hCom, m_dwInBuf, m_dwOutBuf);
	
} 

BOOL CCom::ResetParity(char Parity)
{
	
	if (m_hCom == NULL) return FALSE;
	
	DCB dcb;
	
	dcb.DCBlength = sizeof( DCB );
	
	if (!GetCommState(m_hCom, &dcb))
		return FALSE;
	
	BYTE cParity;
	
	Parity = tolower(Parity);
	
	switch (Parity) {
		
	case 'o':
		
		cParity = 1;
		
		break;
		
	case 'e':
		
		cParity = 2;
		
		break;
		
	case 'm':
		
		cParity = 3;
		
		break;
		
	case 's':
		
		cParity = 4;
		
		break;
		
	default:
		
		cParity = 0;
		
		break;
		
	}
	
	dcb.Parity = cParity;
	
	return SetCommState(m_hCom, &dcb);
	
}

BOOL CCom::SendData(LPCVOID lpBuf, DWORD dwToWrite)
{
	
	TRACE("SSSSSSSSSSSSS 00\n"); 
	
	if( !m_bOpened || m_hCom == NULL )
		return FALSE;
	
	DWORD dwWritten;
	
	if (WriteFile(m_hCom, lpBuf, dwToWrite, &dwWritten, &m_osWriter))
		return TRUE;
	
	if (GetLastError() != ERROR_IO_PENDING)
		return FALSE;
	
	GetOverlappedResult(m_hCom, &m_osWriter, &dwWritten, TRUE);
	
	TRACE("SSSSSSSSSSSSS 11\n"); 
	
	return (dwToWrite == dwWritten);
	
} 

DWORD CCom::ReadData(LPVOID lpBuf, DWORD dwToRead, int ntimeout)
{ 
	TRACE("RRRRRRRRRRRR 00\n"); 
	
	if( !m_bOpened || m_hCom == NULL ) return 0;
	
	DWORD dwRead;
	
	if (ReadFile(m_hCom, lpBuf, dwToRead, &dwRead, &m_osReader))
		return dwRead; 
	
	if (GetLastError() != ERROR_IO_PENDING)  
		return 0; 
	if (WaitForSingleObject(m_osReader.hEvent, ntimeout) != WAIT_OBJECT_0 ) 
		return 0; 
	if (!GetOverlappedResult(m_hCom, &m_osReader, &dwRead, FALSE))
		return 0;
	
	TRACE("RRRRRRRRRRRR 11\n");  
	
//	Sleep(230);

	return dwRead;

}

void CCom::Close()
{ 
	
	if (m_osReader.hEvent != NULL) 
	{
		CloseHandle( m_osReader.hEvent );
		m_osReader.hEvent = NULL;
	}
	
	if (m_osWriter.hEvent != NULL) 
	{
		CloseHandle( m_osWriter.hEvent );
		m_osWriter.hEvent = NULL;
	}
	
	if (m_hCom != NULL)  
	{
		CloseHandle( m_hCom );
		m_hCom = NULL;
	}
	
	m_bOpened = FALSE;
	
} 

CCom::~CCom()
{
	Close();
} 

// main.cpp char Buf[40];

/*int nArray[3];

CMyComn myCom;

UINT SendDataProc(LPVOID pParam); void OnCommSendReceive() 

{
	
	myCom.PreOpenSetupQueue(12, 12);
	
	if (!myCom.Open(2, 4800)) return;
	
	//如果想改?校?位，在此位置，如：myCom.ResetParity('m');
	
	FillMemory(Buf, 40, 0);
	
	nArray[0] = 0;
	
	nArray[1] = 17;
	
	nArray[2] = 88888;
	
	AfxBeginThread(SendDataProc, (LPVOID)(12));
	
	TRACE("EEEEEEEE\n");
	
	int nRead = myCom.ReadData(Buf, 12);
	
	int* nA = (int*) Buf;
	
	TRACE("AAAAAAAAAAAAAA %d, %d %d %d\n",nRead, nA[0], nA[1], nA[1]); 
	
} UINT SendDataProc(LPVOID pParam)

{
	
	return myCom.SendData((LPVOID)nArray, (DWORD) pParam);
	
}*/