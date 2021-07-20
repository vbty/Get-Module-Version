#include "windows.h"
#include "stdio.h"
#include "string.h"
#include "shlwapi.h"

DWORD gVersion = 0;
BOOLEAN bBackup = FALSE;
CHAR BackupDict[MAX_PATH] = "";
BOOLEAN DEBUG = FALSE;

CONST CHAR* DictBlackList[] = {
	"SysWOW64",
	"WinSxS"
};

CONST CHAR* ExtNameWhiteList[] = {
	"sys",
	"exe",
	"dll"
};

VOID WINAPI GetModuleVersion(CHAR* pModulePath)
{
	DWORD				dwHandle;
	DWORD				VersionInfoSize;
	VS_FIXEDFILEINFO*	pFixedFileInfo = NULL;
	LARGE_INTEGER		ModuleVersion;
	UINT				Size;
	VOID* pVersionData = NULL;

	ModuleVersion.QuadPart = 0;
	VersionInfoSize = GetFileVersionInfoSizeA(pModulePath, &dwHandle);
	pVersionData = malloc(VersionInfoSize);

	if (pVersionData == NULL)
		return;

	if (!GetFileVersionInfoA(pModulePath, 0, VersionInfoSize, pVersionData))
		return;

	if (!VerQueryValueW(pVersionData, L"\\", (VOID**)&pFixedFileInfo, &Size))
		return;
	 
	if (pFixedFileInfo->dwSignature == 0xfeef04bd)
	{
		if (gVersion == ((pFixedFileInfo->dwFileVersionLS >> 0) & 0xffff))
		{
			printf("Version: %d.%d   %s\n",
				(pFixedFileInfo->dwFileVersionLS >> 16) & 0xffff,
				(pFixedFileInfo->dwFileVersionLS >> 0) & 0xffff,
				pModulePath
			);

			if (bBackup)
			{
				CHAR NewFileName[MAX_PATH] = "";
				CHAR PartFileName[MAX_PATH] = "";
				PCHAR LastBackslash = NULL;
				PCHAR LastDot = NULL;

				LastBackslash = StrRStrIA(
					pModulePath,
					NULL,
					"\\");

				if (!LastBackslash)
				{
					puts("LastBackslash error");
				}
				
				lstrcpyA(PartFileName, LastBackslash + 1);

				LastDot = StrRStrIA(
					PartFileName,
					NULL,
					".");

				if (!LastDot)
				{
					puts("LastDot error");
				}

				LastDot[0] = '\x00';

				sprintf_s(NewFileName, MAX_PATH, "%s\\%s.%d.%s",
					BackupDict,
					PartFileName,
					(pFixedFileInfo->dwFileVersionLS >> 0) & 0xffff,
					LastDot+1);

				if (!CopyFileA(pModulePath, NewFileName, TRUE))
				{
					DWORD LastError = GetLastError();
					if (LastError == 80)
						return;
					printf("[ERROR] CopyFile %d\n", LastError);
				}
			}
		}
	}
	return;
}

VOID TraverDict(PCHAR StartFilePath)
{
	CHAR NewPath[MAX_PATH*2] = "";
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	for (size_t i = 0; i < sizeof(DictBlackList)/sizeof(PCHAR); i++)
	{
		if (strstr(StartFilePath, DictBlackList[i]))
		{
			return;
		}
	}

	sprintf(NewPath, "%s\\*", StartFilePath);
	hFind = FindFirstFileA(NewPath, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		if (DEBUG)
		{
			printf("Dict Not Find: %s\n", StartFilePath);
		}
	
		return;
	}

	while (FindNextFileA(hFind, &FindFileData) != 0)                         
	{
		if ( lstrcmpA(FindFileData.cFileName, ".") == 0 || 
			 lstrcmpA(FindFileData.cFileName, "..") == 0 )       
		{
			continue;
		}

		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)  
		{
			ZeroMemory(NewPath, MAX_PATH);
			sprintf(NewPath, "%s\\%s", StartFilePath, FindFileData.cFileName);
			TraverDict(NewPath);                               
		}

		for (size_t i = 0; i < sizeof(ExtNameWhiteList)/sizeof(PCHAR); i++)
		{
			if (strstr(FindFileData.cFileName, ".mui"))
			{
				continue;
			}

			if (strstr(FindFileData.cFileName, ExtNameWhiteList[i]))
			{
				ZeroMemory(NewPath, MAX_PATH);
				sprintf(NewPath, "%s\\%s", StartFilePath, FindFileData.cFileName);
				GetModuleVersion(NewPath);
				break;
			}	
		}
	}
	FindClose(hFind);
}

VOID ShowUsage(VOID)
{
	puts("Usage:"
	"start_path ver_num [-b backup_path]");
}

int main(int argc,char *argv[])
{
	CHAR StartPath[MAX_PATH] = "";

	if (argc < 3)
	{
		ShowUsage();
		return 0;
	}

	if (argc == 5)
	{
		if (argv[3][0] == '-' &&
			argv[3][1] == 'b')
		{
			bBackup = TRUE;
			lstrcpyA(BackupDict, argv[4]);
		}
		else
		{
			ShowUsage();
			return 0;
		}
	}

	gVersion = atoi(argv[2]);
	TraverDict(argv[1]);
};

