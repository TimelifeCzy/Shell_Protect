#include "stdafx.h"
#include "puPEinfoData.h"

PuPEInfo::PuPEInfo()
{
	m_bLoadFileSuc = false;
	m_hFileHandle = NULL;
}

PuPEInfo::~PuPEInfo()
{
	if (m_pFileBase) {
		free(m_pFileBase);
		m_pFileBase = nullptr;
	}
	if (m_hFileHandle)
		CloseHandle(m_hFileHandle);
	m_bLoadFileSuc = false;
}

BOOL PuPEInfo::IsPEFile()
{
	if (IMAGE_DOS_SIGNATURE != ((PIMAGE_DOS_HEADER)PuPEInfo::m_pFileBase)->e_magic) return FALSE;
	
	if (IMAGE_NT_SIGNATURE != ((PIMAGE_NT_HEADERS)PuPEInfo::m_pNtHeader)->Signature) return FALSE;
	
	return TRUE;
}

BOOL PuPEInfo::prOpenFile(const CString & PathName)
{
	m_strNamePath = PathName;
	if (m_strNamePath.IsEmpty())
		return false;

	HANDLE hFile = CreateFile(PathName, GENERIC_READ | GENERIC_WRITE, FALSE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ((int)hFile <= 0){ 
		AfxMessageBox(L"打开文件失败"); 
		return FALSE; 
	}

	m_hFileHandle = hFile;
	DWORD dwSize = GetFileSize(hFile, NULL);
	m_FileSize = dwSize;
	
	m_pFileBase = (void *)malloc(dwSize);
	if (!m_pFileBase)
		return false;
	memset(m_pFileBase, 0, dwSize);
	
	DWORD dwRead = 0;
	OVERLAPPED OverLapped = { 0 };
	int nRetCode = ReadFile(hFile, m_pFileBase, dwSize, &dwRead, &OverLapped);
	
	PIMAGE_DOS_HEADER pDosHander = (PIMAGE_DOS_HEADER)m_pFileBase;
#ifdef _WIN64
	PIMAGE_NT_HEADERS pHeadres = (PIMAGE_NT_HEADERS)(pDosHander->e_lfanew + (DWORD64)m_pFileBase);
#else
	PIMAGE_NT_HEADERS pHeadres = (PIMAGE_NT_HEADERS)(pDosHander->e_lfanew + (LONG)m_pFileBase);
#endif
	m_pNtHeader = (void *)pHeadres;
	if (PuPEInfo::OepFlag == FALSE)
	{
		m_OldOEP = pHeadres->OptionalHeader.AddressOfEntryPoint;
		OepFlag = TRUE;
	}

	m_SectionCount = pHeadres->FileHeader.NumberOfSections;
	if (!IsPEFile()){ 
		free(m_pFileBase); 
		m_pFileBase = nullptr; 
		AfxMessageBox(L"非PE文件"); return FALSE; 
	}

	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION((PIMAGE_NT_HEADERS)m_pNtHeader);
	m_SectionHeader = (void *)pSection;
	m_bLoadFileSuc = true;
	return TRUE;
}

BOOL PuPEInfo::prOpenFileEx(const CString& PathName) {
	// clear
	if (m_pFileBase) {
		free(m_pFileBase);
		m_pFileBase = nullptr;
	}
	if (m_hFileHandle) {
		CloseHandle(m_hFileHandle);
		m_hFileHandle = nullptr;
	}
	m_bLoadFileSuc = false;

	// reload
	return prOpenFile(PathName);
}

// RVAofFOA
DWORD PuPEInfo::RVAofFOA(const DWORD Rva)
{
	DWORD dwSectionCount = (PIMAGE_NT_HEADERS(PuPEInfo::m_pNtHeader))->FileHeader.NumberOfSections;

	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION((PIMAGE_NT_HEADERS)PuPEInfo::m_pNtHeader);

	for (DWORD i = 0; i < dwSectionCount; ++i)
	{
		if ((Rva >= (pSection->VirtualAddress)) && (Rva < ((pSection->VirtualAddress) + (pSection->SizeOfRawData)))) {
			// DWORD offset = Rva - pSection->VirtualAddress;
			// DWORD FOA = pSection->PointerToRawData + offset;
			return (pSection->VirtualAddress + pSection->PointerToRawData);
		}
		++pSection;
	}
	return 0;
}

PIMAGE_SECTION_HEADER PuPEInfo::GetSectionAddress(const char* Base, const BYTE* SectionName)
{
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(((PIMAGE_DOS_HEADER)Base)->e_lfanew + Base);

	PIMAGE_SECTION_HEADER pSect = IMAGE_FIRST_SECTION(pNt);

	for (int i = 0; i < m_SectionCount; ++i) { 
		if (0 == _mbscmp(pSect->Name, SectionName))
			return (PIMAGE_SECTION_HEADER)pSect; 
		++pSect; 
	}
	
	return 0;
}

BOOL PuPEInfo::SetFileoffsetAndFileSize(const void* Base, const DWORD & offset, const DWORD size, const BYTE* Name)
{
	 PIMAGE_SECTION_HEADER Address = GetSectionAddress((char*)Base, Name);

	 Address->PointerToRawData = offset;

	 Address->SizeOfRawData = size;

	 return TRUE;
}