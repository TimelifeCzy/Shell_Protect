#include "stdafx.h"
#include "AddSection.h"
#include "puPEinfoData.h"

// x64 asm 
#ifdef _WIN64
extern "C" void __stdcall AsmCountTemp(PVOID dwdata);
extern "C" void __stdcall AsmCountTemp1(PVOID dwdata);
#else
#endif

AddSection::AddSection()
{
}

AddSection::~AddSection()
{
	if (pFileBaseData) {
		free(pFileBaseData);
		pFileBaseData = nullptr;
	}
	if (FileHandle) {
		CloseHandle(FileHandle);
		FileHandle = nullptr;
	}
}

BOOL AddSection::Init() {
	Free();

	SinglePuPEInfo::instance()->puOpenFileLoadEx(m_FilePath);
	pFileBaseData = SinglePuPEInfo::instance()->puGetImageBase();
	pNtHeadre = SinglePuPEInfo::instance()->puGetNtHeadre();
	pSectionHeadre = SinglePuPEInfo::instance()->puGetSection();
	FileSize = SinglePuPEInfo::instance()->puFileSize();
	FileHandle = SinglePuPEInfo::instance()->puFileHandle();
	OldOep = SinglePuPEInfo::instance()->puOldOep();
	return true;
}

BOOL AddSection::Free() {
	SinglePuPEInfo::instance()->puClearPeData();
	if (pFileBaseData) {
		pFileBaseData = nullptr;
	}
	if (FileHandle) {
		FileHandle = nullptr;
	}
	if (m_newlpBase) {
		free(m_newlpBase);
		m_newlpBase = nullptr;
	}
	pNtHeadre = nullptr;
	pSectionHeadre = nullptr;
	SectionSizeof = 0;
	FileSize = 0;
	return true;
}

BOOL AddSection::ModifySectionNumber()
{
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)this->pNtHeadre;
	if (pNtHeaders) {
		DWORD temp = pNtHeaders->FileHeader.NumberOfSections;
		SectionSizeof = temp * 0x28;
		pNtHeaders->FileHeader.NumberOfSections += 0x1;
		return TRUE;
	}
	return false;
}

BOOL AddSection::ModifySectionInfo(const BYTE* Name, const DWORD & size)
{
#ifdef _WIN64
	DWORD64 pSectionAddress = (DWORD64)pSectionHeadre;
#else
	DWORD pSectionAddress = (DWORD)pSectionHeadre;
#endif
	pSectionAddress = pSectionAddress + SectionSizeof - 0x28;
	PIMAGE_SECTION_HEADER PtrpSection = (PIMAGE_SECTION_HEADER)pSectionAddress;
	if (!PtrpSection)
		return false;

	pSectionAddress += 0x28;
	NewpSection = (PIMAGE_SECTION_HEADER)pSectionAddress;
	memcpy(NewpSection->Name, Name, sizeof(Name));
	DWORD dwtemps = PtrpSection->VirtualAddress + PtrpSection->SizeOfRawData;
	if (!dwtemps)
		return false;

	DWORD Temp = 0;
#ifdef _WIN64
	// x64下使用，不涉及__int64类型，汇编使用同一套即可
	AsmCountTemp(&dwtemps);
	NewpSection->VirtualAddress = dwtemps;
	Temp = PtrpSection->SizeOfRawData + PtrpSection->PointerToRawData;
	AsmCountTemp1(&Temp);
	// check arg
	if (!dwtemps || !Temp)
		return 0;

#else
	__asm{
		pushad;
		mov		esi, dwtemps;
		mov		eax, dwtemps;
		mov		edx, 0x1;
		mov		cx, 0x1000;
		div		cx;
		test	dx, dx;
		jz		MemSucess
		shr		dx, 12;
		inc		dx;
		shl		dx, 12;
		add		esi, edx;
		shr		esi, 12;
		shl		esi, 12;
		mov		dwtemps, esi;
	MemSucess:
		popad
	}
	NewpSection->VirtualAddress = dwtemps;

	Temp = PtrpSection->SizeOfRawData + PtrpSection->PointerToRawData;

	__asm{
		pushad;
		mov		esi, Temp;
		mov		edx, 0x1;
		mov		eax, Temp;
		mov		ecx, 0x200;
		div		cx;
		test	dx, dx;
		jz		FileSucess
		xor		eax, eax
		mov		ax, 0x200;
		sub		ax, dx;
		add		esi, eax;
		mov		Temp, esi;
	FileSucess:
		popad
	}

#endif // _WIN64
	NewpSection->PointerToRawData = Temp;
	NewpSection->SizeOfRawData = size;
	NewpSection->Misc.VirtualSize = NewpSection->SizeOfRawData;
	NewpSection->Characteristics = 0xE00000E0;
	return TRUE;
}

BOOL AddSection::ModifyProgramEntryPoint()
{
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)pNtHeadre;
	if (pNt) {
		pNt->OptionalHeader.AddressOfEntryPoint = NewpSection->VirtualAddress;
		return TRUE;
	}
	return false;
}

BOOL AddSection::ModifySizeofImage()
{
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)pNtHeadre;
	if (pNt) {
		pNt->OptionalHeader.SizeOfImage = NewpSection->VirtualAddress + NewpSection->SizeOfRawData;
		pNt->OptionalHeader.DllCharacteristics = 0x8000;
		return TRUE;
	}
	return FALSE;
}

BOOL AddSection::AddNewSectionByteData(const DWORD & size)
{
	const int newFileSize = FileSize + size;
	m_newlpBase = (char *)malloc(newFileSize);
	if (!m_newlpBase || (nullptr == m_newlpBase))
		return false;
	memset(m_newlpBase, 0, newFileSize);

	if (pFileBaseData) {
		memcpy(m_newlpBase, pFileBaseData, FileSize);
	}
	else
		return false;

	DWORD dWriteSize = 0; OVERLAPPED OverLapped = { 0 };
	int nRetCode = WriteFile(FileHandle, m_newlpBase, (FileSize + size), &dWriteSize, &OverLapped);
	if (m_newlpBase) {
		free(m_newlpBase);
		m_newlpBase = nullptr;
	}
	if (dWriteSize == 0){ 
		AfxMessageBox(L"CreateSection WriteFIle faliuer"); 
		return FALSE; 
	}
	return TRUE;
}

