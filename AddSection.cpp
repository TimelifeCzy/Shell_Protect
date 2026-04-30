#include "stdafx.h"
#include "AddSection.h"
#include "puPEinfoData.h"

// x64 asm
#ifdef _WIN64
extern "C" void __stdcall AsmCountTemp(PVOID dwdata);
extern "C" void __stdcall AsmCountTemp1(PVOID dwdata);
#else
#endif

namespace {
DWORD AlignValue(DWORD value, DWORD alignment)
{
	if (alignment == 0)
		return value;
	return ((value + alignment - 1) / alignment) * alignment;
}
}

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

	if (!SinglePuPEInfo::instance()->puOpenFileLoadEx(m_FilePath))
		return false;
	pFileBaseData = SinglePuPEInfo::instance()->puGetImageBase();
	pNtHeadre = SinglePuPEInfo::instance()->puGetNtHeadre();
	pSectionHeadre = SinglePuPEInfo::instance()->puGetSection();
	FileSize = SinglePuPEInfo::instance()->puFileSize();
	FileHandle = SinglePuPEInfo::instance()->puFileHandle();
	OldOep = SinglePuPEInfo::instance()->puGetOEP();
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
	NewpSection = nullptr;
	SectionSizeof = 0;
	FileSize = 0;
	OldOep = 0;
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
	if (!Name || !pSectionHeadre || !pNtHeadre || SectionSizeof == 0)
		return FALSE;

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
	memset(NewpSection, 0, sizeof(IMAGE_SECTION_HEADER));
	const size_t nameLen = strlen((const char*)Name);
	memcpy(NewpSection->Name, Name, min(nameLen, (size_t)IMAGE_SIZEOF_SHORT_NAME));

	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)pNtHeadre;
	if (!pNt)
		return FALSE;

	PIMAGE_SECTION_HEADER pFirstSection = IMAGE_FIRST_SECTION(pNt);
	DWORD firstRawPointer = 0;
	for (WORD i = 0; i < pNt->FileHeader.NumberOfSections - 1; ++i)
	{
		if (pFirstSection[i].PointerToRawData != 0 &&
			(firstRawPointer == 0 || pFirstSection[i].PointerToRawData < firstRawPointer))
		{
			firstRawPointer = pFirstSection[i].PointerToRawData;
		}
	}
	if (firstRawPointer != 0)
	{
		const DWORD newSectionHeaderEnd = (DWORD)((BYTE*)(NewpSection + 1) - (BYTE*)pFileBaseData);
		if (newSectionHeaderEnd > firstRawPointer) {
			AfxMessageBox(L"区段表空间不足，无法安全添加新区段");
			return FALSE;
		}
	}

	const DWORD sectionAlignment = pNt->OptionalHeader.SectionAlignment;
	const DWORD fileAlignment = pNt->OptionalHeader.FileAlignment;
	const DWORD prevVirtualSize = max(PtrpSection->Misc.VirtualSize, PtrpSection->SizeOfRawData);
	DWORD dwtemps = PtrpSection->VirtualAddress + prevVirtualSize;
	if (!dwtemps)
		return false;

	dwtemps = AlignValue(dwtemps, sectionAlignment);
	NewpSection->VirtualAddress = dwtemps;
	DWORD Temp = PtrpSection->SizeOfRawData + PtrpSection->PointerToRawData;
	Temp = AlignValue(Temp, fileAlignment);
	if (!dwtemps || !Temp)
		return FALSE;

	NewpSection->PointerToRawData = Temp;
	NewpSection->SizeOfRawData = AlignValue(size, fileAlignment);
	NewpSection->Misc.VirtualSize = size;
	NewpSection->Characteristics = 0xE00000E0;
	return TRUE;
}

BOOL AddSection::ModifyProgramEntryPoint()
{
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)pNtHeadre;
	if (pNt && NewpSection) {
		pNt->OptionalHeader.AddressOfEntryPoint = NewpSection->VirtualAddress;
		return TRUE;
	}
	return false;
}

BOOL AddSection::ModifySizeofImage()
{
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)pNtHeadre;
	if (pNt && NewpSection) {
		const DWORD sectionAlignment = pNt->OptionalHeader.SectionAlignment;
		const DWORD imageEnd = NewpSection->VirtualAddress + max(NewpSection->Misc.VirtualSize, NewpSection->SizeOfRawData);
		pNt->OptionalHeader.SizeOfImage = AlignValue(imageEnd, sectionAlignment);
		pNt->OptionalHeader.DllCharacteristics = 0x8000;
		return TRUE;
	}
	return FALSE;
}

BOOL AddSection::AddNewSectionByteData(const DWORD & size)
{
	if (!FileHandle || !pFileBaseData || !NewpSection)
		return FALSE;

	const DWORD sectionEnd = NewpSection->PointerToRawData + NewpSection->SizeOfRawData;
	if (sectionEnd < NewpSection->PointerToRawData)
		return FALSE;
	const DWORD newFileSize = max(FileSize, sectionEnd);
	m_newlpBase = (char *)malloc(newFileSize);
	if (!m_newlpBase || (nullptr == m_newlpBase))
		return false;
	memset(m_newlpBase, 0, newFileSize);

	if (pFileBaseData) {
		memcpy(m_newlpBase, pFileBaseData, FileSize);
	}
	else
		return false;

	SetFilePointer(FileHandle, 0, NULL, FILE_BEGIN);
	DWORD dWriteSize = 0; OVERLAPPED OverLapped = { 0 };
	int nRetCode = WriteFile(FileHandle, m_newlpBase, newFileSize, &dWriteSize, &OverLapped);
	if (m_newlpBase) {
		free(m_newlpBase);
		m_newlpBase = nullptr;
	}
	if (!nRetCode || dWriteSize != newFileSize) {
		AfxMessageBox(L"CreateSection WriteFIle faliuer");
		return FALSE;
	}
	return TRUE;
}
