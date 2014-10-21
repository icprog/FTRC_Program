#include "stdafx.h"
#include "CompareProgram.h"
#include <windows.h>
#include <math.h>
#include "Shlwapi.h"
#include <direct.h>


// GetLastWriteTime - Retrieves the last-write time and converts
//                    the time to a string
//
// Return value  - TRUE if successful, FALSE otherwise
// FileName      - File name
// stLocal       - Buffer used receive local time

BOOL GetLastWriteTime(char* FileName, SYSTEMTIME* stLocal)
{
	HANDLE hFile;
	hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

    FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME stUTC;

    // Retrieve the file times for the file.
    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
        return FALSE;

    // Convert the last-write time to local time.
    FileTimeToSystemTime(&ftWrite, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, stLocal);

	CloseHandle(hFile);
    return TRUE;
}

// CompareFileTime - Compare two file LastWirteTime
// Return value  - TRUE if time is same, FALSE otherwise
// pFile1        - File name1
// pFile2        - File name2
// pProgramState - Program recode log file

BOOL CompareFileTime(char *pFile1,char* pFile2 , char* pProgramState)
{
	if((PathFileExists(pFile1) == NULL ) || (PathFileExists(pFile2) == NULL ))
	{
		return FALSE;
	}
	SYSTEMTIME stLocal1,stLocal2;
	GetLastWriteTime(pFile1,&stLocal1);
	GetLastWriteTime(pFile2,&stLocal2);
	long time1,time2;
	time1=stLocal1.wSecond+stLocal1.wMinute*60+stLocal1.wHour*60*60+stLocal1.wDay*24*60*60;
	sprintf(pProgramState , "%s%s LastWirteTime is:%d\r\n" , pProgramState , pFile1 , time1);
	time2=stLocal2.wSecond+stLocal2.wMinute*60+stLocal2.wHour*60*60+stLocal2.wDay*24*60*60;
	sprintf(pProgramState , "%s%s LastWirteTime is:%d\r\n" , pProgramState , pFile2 , time2);
	if(abs(time1-time2)>5)
		return FALSE;
	else
		return TRUE;
}

// GetFileCheckSum - Get file's checksum and return a receive buffer
//
// Return value  - TRUE if successful, FALSE otherwise
// FileName      - File name
// lpszString    - Pointer to buffer to receive string

BOOL GetFileCheckSum(char* FileName, char* lpszString)
{
	//Use md5sum.exe get filename's checksum and to a file	
	char szFileNameBuffer[512];
	char szFileBuffer[512];
	memset(szFileNameBuffer , 0 , 512);
	memset(szFileBuffer , 0 , 512);
	GetCurrentDirectory(512 , szFileNameBuffer);
	sprintf_s(szFileBuffer , 512 , " md5sum.exe \"%s\" >Result.txt\n" , FileName);

	if(PathFileExists("Result.txt"))
	{
		DeleteFile("Result.txt");
	}
	if(PathFileExists("getchecksum.bat"))
	{
		DeleteFile("getchecksum.bat");
	}
	
	FILE * fp;
	fopen_s(&fp,"getchecksum.bat","a");
	//fprintf(fp,"title getchecksum\n");
	fprintf(fp,"cd\\\n");
	fprintf(fp,"%c:\n",_getdrive() + 'A'-1);
	//fprintf(fp,"D:\n");
	fprintf(fp,"cd \"");
	fprintf(fp,szFileNameBuffer);
	fprintf(fp,"\"");
	fprintf(fp,"\n");
	fprintf(fp,szFileBuffer);
	fprintf(fp,"\n");
	//fprintf(fp,"exit\n");
	fclose(fp);

	WinExec("getchecksum.bat" , SW_HIDE);

	//open file get filename's checksum
	CFile cFile;	
	if(cFile.Open("Result.txt" , CFile::modeRead , NULL))
	{
		cFile.Read(lpszString , 33);
		cFile.Close();
		return TRUE;
		
	}
	return FALSE;
 }

// CompareFileCheckSum - Compare two file's checksum 
//
// Return value  - TRUE if checksum same, FALSE otherwise
// pFile1		 - File1 name
// pFile2		 - File2 name
// pProgramState - Program recode log file

BOOL CompareFileCheckSum(char *pFile1,char* pFile2 , char* pProgramState)
{
	char File1CheckSum[64];
	char File2CheckSum[64];
	memset(File1CheckSum , 0 ,64);
	memset(File2CheckSum , 0 ,64);

	if((PathFileExists(pFile1) == NULL ) || (PathFileExists(pFile2) == NULL ))
	{
		return FALSE;
	}
	//Get file1 checksum
	if(!GetFileCheckSum(pFile1 , File1CheckSum))
	{
		return FALSE;
	}
	//Get file2 checksum
	if(!GetFileCheckSum(pFile2 , File2CheckSum))
	{
		return FALSE;
	}
	//campare file1 and file2 checksum
	sprintf(pProgramState , "%s%s checksum is:%s\r\n" , pProgramState , pFile1 , File1CheckSum);
	sprintf(pProgramState , "%s%s checksum is:%s\r\n" , pProgramState , pFile2 , File2CheckSum);
	if(strncmp(File1CheckSum , File2CheckSum , 33) != 0)
	{
		return FALSE;
	}
	return TRUE;
}

// AutoDownLoadProgram - Compare last wirte time and file checksum and download file from server file to local  
//
// Return value    - TRUE if checksum same, FALSE otherwise
// pLocalFilePath  - Local file path
// pServerFilePath - server file path
// pProgramState   - Program recode log file
BOOL AutoDownLoadProgram(char *pLocalFilePath , char *pServerFilePath , char *ProgramState)
{
	BOOL bChange;
	bChange = FALSE;
	CFileFind finder;
	char strServerFileList[512];
	strcpy_s(strServerFileList , 512 , pServerFilePath);

	if(PathFileExists(".\\delself.bat"))
	{
		DeleteFile(".\\delself.bat");
	}

	FILE * fp;
	fopen_s(&fp,"delself.bat","a");
	fprintf(fp,"title Upgrade\n");
	fclose(fp);

	strcat_s(strServerFileList,512,"\\*.*");	
	BOOL bWorking = finder.FindFile(strServerFileList);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		char strLocalFileList[512];
		strcpy_s(strLocalFileList,512,pLocalFilePath);
		if(!finder.IsDirectory())
		{
			if( strstr(finder.GetFileName(),".ex") || 
				strstr(finder.GetFileName(),".Ex") || 
				strstr(finder.GetFileName(),".Ex") || 
				strstr(finder.GetFileName(),".EX")
				)
			{
				//F:\lsy\Test\量產FT\FT(U12H163)\U12H63.exx
				char szServerFileList[512];
				strcpy_s(szServerFileList , 512 , pServerFilePath);
				sprintf_s(strServerFileList,512,"%s\\%s",szServerFileList,finder.GetFileName());

				//D:\Project\H163\FT&RC\program function list---20111102\program function list\release\U12H63.exe
				char szLocalFileList[512];
				strcpy_s(szLocalFileList,512,pLocalFilePath);
				sprintf_s(strLocalFileList,512,"%s\\%s.exe",szLocalFileList,finder.GetFileTitle());

				if( !(CompareFileTime(strServerFileList , strLocalFileList , ProgramState) || 
				      CompareFileCheckSum(strServerFileList , strLocalFileList , ProgramState) 
				     ) )
				{
					bChange = TRUE;
					//FILE * fp;
					fopen_s(&fp,"delself.bat","a");
					fprintf(fp,"copy \"%s\" \"%s\" /y\n",strServerFileList,strLocalFileList);

					fclose(fp);
				}
			}			
			if((strstr(finder.GetFileName() , ".dll" )) || (strstr(finder.GetFileName() , ".ini" )))
				
			{
				//F:\lsy\Test\量產FT\FT(U12H163)\U12H63.exx
				char szServerFileList[512];
				strcpy_s(szServerFileList , 512 , pServerFilePath);
				sprintf_s(strServerFileList,512,"%s\\%s",szServerFileList,finder.GetFileName());

				//D:\Project\H163\FT&RC\program function list---20111102\program function list\release
				char szLocalFileList[512];
				strcpy_s(szLocalFileList,512,pLocalFilePath);
				sprintf_s(strLocalFileList,512,"%s\\%s",szLocalFileList,finder.GetFileName());

				if(!(CompareFileTime(strServerFileList , strLocalFileList , ProgramState) || 
				     CompareFileCheckSum(strServerFileList , strLocalFileList , ProgramState)
				    ) )
				{
					bChange = TRUE;
					//FILE * fp;
					fopen_s(&fp,"delself.bat","a");
					fprintf(fp,"copy \"%s\" \"%s\" /y\n",strServerFileList,strLocalFileList);

					fclose(fp);
				}
			}

		}
	}

	fopen_s(&fp,"delself.bat","a");
	fprintf(fp,"%s\n","exit");
	fclose(fp);

	WinExec("delself.bat" , SW_SHOW);	
	//AfxBeginThread(FunctionTest , NULL ,0  , 0 , 0);

	if(bChange)
	{		
		exit(1);

	}

	return TRUE;
}

UINT __cdecl  FunctionTest(LPVOID parameter)
{
	//get program name used for open it again
	char pProgramName[512];
	char szLocalPath[512];
	memset(pProgramName , 0 , 512);
	memset(szLocalPath , 0 , 512);
	GetCurrentDirectory(sizeof(szLocalPath),szLocalPath);
	lstrcat(szLocalPath,"\\TestInformation.ini");
	GetPrivateProfileString("General","ProgramName","U12H163.exe",pProgramName, 512,szLocalPath);

	FILE * fp;
	fopen_s(&fp,"a.bat","w");
	fprintf(fp, pProgramName);
	fprintf(fp,"%s\n","exit");		
	fclose(fp);
	WinExec("delself.bat" , SW_SHOW);
	return 1;
}