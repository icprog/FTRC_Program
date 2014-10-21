
// GetLastWriteTime - Retrieves the last-write time and converts
//                    the time to a string
//
// Return value  - TRUE if successful, FALSE otherwise
// FileName      - File name
// stLocal       - Buffer used receive local time
BOOL GetLastWriteTime(char* FileName, SYSTEMTIME* stLocal);

// CompareFileTime - Compare two file LastWirteTime
// Return value  - TRUE if time is same, FALSE otherwise
// pFile1        - File name1
// pFile2        - File name2
// pProgramState - Program recode log file
BOOL CompareFileTime(char *pFile1,char* pFile2 , char* pProgramState);

// AutoDownLoadProgram - Compare last wirte time and file checksum and download file from server file to local  
//
// Return value    - TRUE if checksum same, FALSE otherwise
// pLocalFilePath  - Local file path
// pServerFilePath - server file path
// pProgramState   - Program recode log file
BOOL AutoDownLoadProgram(char *pLocalFilePath , char *pServerFilePath , char *ProgramState);

// CompareFileCheckSum - Compare two file's checksum 
//
// Return value  - TRUE if checksum same, FALSE otherwise
// pFile1		 - File1 name
// pFile2		 - File2 name
// pProgramState - Program recode log file

BOOL CompareFileCheckSum(char *pFile1,char* pFile2 , char* pProgramState);

// CompareFileCheckSum - Compare two file's checksum 
//
// Return value  - TRUE if checksum same, FALSE otherwise
// pFile1		 - File1 name
// pFile2		 - File2 name
BOOL GetFileCheckSum(char* FileName, char* lpszString);
UINT __cdecl  FunctionTest(LPVOID parameter);