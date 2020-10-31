#include "AddSection.h"
#include "puPEinfoData.h"

// x64 asm 
#ifdef _WIN64
extern "C" void __stdcall AsmCountTemp(PVOID dwdata);
extern "C" void __stdcall AsmCountTemp1(PVOID dwdata);
#else

#endif


char* AddSection::newlpBase = nullptr;

AddSection::AddSection()
{
	PuPEInfo obj_PuPE;

	pFileBaseData = obj_PuPE.puGetImageBase();

	pNtHeadre = obj_PuPE.puGetNtHeadre();

	pSectionHeadre = obj_PuPE.puGetSection();

	FileSize = obj_PuPE.puFileSize();

	FileHandle = obj_PuPE.puFileHandle();

	OldOep = obj_PuPE.puOldOep();
}

AddSection::~AddSection()
{

}

BOOL AddSection::ModifySectionNumber()
{
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)this->pNtHeadre;

	DWORD temp = pNtHeaders->FileHeader.NumberOfSections;

	SectionSizeof = temp * 0x28;

	pNtHeaders->FileHeader.NumberOfSections += 0x1;

	return TRUE;
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

	pSectionAddress += 0x28;
	NewpSection = (PIMAGE_SECTION_HEADER)pSectionAddress;
	
	memcpy(NewpSection->Name, Name, sizeof(Name));
	DWORD dwtemps = PtrpSection->VirtualAddress + PtrpSection->SizeOfRawData;

	DWORD Temp = 0;

#ifdef _WIN64
	// x64下使用，因为不涉及__int64类型，所以汇编使用同一套即可
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

	pNt->OptionalHeader.AddressOfEntryPoint = NewpSection->VirtualAddress;

	return TRUE;
}

BOOL AddSection::ModifySizeofImage()
{
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)pNtHeadre;

	pNt->OptionalHeader.SizeOfImage = NewpSection->VirtualAddress + NewpSection->SizeOfRawData;
	
	pNt->OptionalHeader.DllCharacteristics = 0x8000;
	
	return TRUE;
}

BOOL AddSection::AddNewSectionByteData(const DWORD & size)
{
	newlpBase = (char *)malloc(FileSize + size);
	memset(newlpBase, 0, (FileSize + size));
	memcpy(newlpBase, pFileBaseData, FileSize);
	free(pFileBaseData);

	DWORD dWriteSize = 0; OVERLAPPED OverLapped = { 0 };

	int nRetCode = WriteFile(FileHandle, newlpBase, (FileSize + size), &dWriteSize, &OverLapped);

	free(newlpBase);

	if (dWriteSize == 0){ AfxMessageBox(L"CreateSection WriteFIle faliuer"); return FALSE; }

	return TRUE;
}

