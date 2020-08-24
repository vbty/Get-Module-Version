#include "windows.h"
#include "stdio.h"

DWORD gVersion = 0;

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
		}
	}
	return;
}

VOID TraverDict(PCHAR StartFilePath)
{
	CHAR NewPath[MAX_PATH] = "";
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	sprintf(NewPath, "%s\\*", StartFilePath);
	hFind = FindFirstFileA(NewPath, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("Dict Not Find: %s\n", StartFilePath);
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

		if (!strstr(FindFileData.cFileName,".mui")&&
			(strstr(FindFileData.cFileName, ".exe") || strstr(FindFileData.cFileName, ".dll")))
		{
			ZeroMemory(NewPath, MAX_PATH);
			sprintf(NewPath, "%s\\%s", StartFilePath, FindFileData.cFileName);
			GetModuleVersion(NewPath);
		}
	}
	FindClose(hFind);
}

int main(int argc,char *argv[])
{
	CHAR StartPath[MAX_PATH] = "";

	if (argc != 3)
	{
		puts("path_name version");
		return 0;
	}

	gVersion = atoi(argv[2]);
	TraverDict(argv[1]);
};

