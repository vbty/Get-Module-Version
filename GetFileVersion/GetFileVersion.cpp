#include "windows.h"
#include "stdio.h"

UINT64 WINAPI GetModuleVersion(WCHAR* pModulePath)
{
	DWORD				dwHandle;
	DWORD				VersionInfoSize;
	VS_FIXEDFILEINFO* pFixedFileInfo;
	LARGE_INTEGER		ModuleVersion;
	UINT				Size;
	VOID* pVersionData = NULL;

	ModuleVersion.QuadPart = 0;
	VersionInfoSize = GetFileVersionInfoSizeW(pModulePath, &dwHandle);
	pVersionData = malloc(VersionInfoSize);
	if (pVersionData == NULL) goto End;

	if (!GetFileVersionInfoW(pModulePath, 0, VersionInfoSize, pVersionData)) goto End;

	if (!VerQueryValueW(pVersionData, L"\\", (VOID**)&pFixedFileInfo, &Size)) goto End;
	ModuleVersion.HighPart = pFixedFileInfo->dwProductVersionMS;
	ModuleVersion.LowPart = pFixedFileInfo->dwProductVersionLS;
End:
	if (pVersionData != NULL) free(pVersionData);
	printf("%llx", ModuleVersion.QuadPart);
	return (ModuleVersion.QuadPart);
}

int main()
{
	WCHAR FilePath[MAX_PATH] = L"C:\\windows\\system32\\kernel32.dll";

	GetModuleVersion(FilePath);
};

