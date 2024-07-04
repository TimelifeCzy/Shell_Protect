#include "stdafx.h"
#include "studData.h"
#include "CombatShell/CombatShell.h"
#include "puPEinfoData.h"
#include "CompressionData.h"
#include <io.h>

#define NEWSECITONNAME ".VMP"

extern _Stud*	g_stu;
extern char		g_CombatShellDataLocalFile[MAX_PATH];

studData::studData()
{
}

studData::~studData()
{
} 

BOOL studData::InitStuData() {
	try
	{
		if (!SinglePuPEInfo::instance()->puOpenFileLoadEx(m_MasterFilePath))
			return false;

		m_lpBase = SinglePuPEInfo::instance()->puGetImageBase();
#ifdef _WIN64
		m_dwNewSectionAddress64 = (DWORD64)SinglePuPEInfo::instance()->puGetSectionAddress((char*)m_lpBase, (BYTE*)NEWSECITONNAME);
#else
		m_dwNewSectionAddress = (DWORD)pPeObj->puGetSectionAddress((char*)m_lpBase, (BYTE*)NEWSECITONNAME);
#endif
		m_Oep = SinglePuPEInfo::instance()->puOldOep();
	}
	catch (const std::exception&)
	{
		return false;
	}
	return true;
}

// Stud热身
BOOL studData::LoadLibraryStud()
{
	std::string sDriectory = "";
	std::string sCombatShellPath = "";
	CodeTool::CGetCurrentDirectory(sDriectory);
	if (!sDriectory.empty()) {
		sCombatShellPath = (sDriectory + "CombatShell.dll").c_str();
	}
	if (sCombatShellPath.empty())
		sCombatShellPath = "CombatShell.dll";
	std::wstring wsCombatShellPath = CodeTool::string2wstring(sCombatShellPath.c_str()).c_str();
	if (_access(sCombatShellPath.c_str(), 0) != 0) {
		AfxMessageBox((L"CombatShell文件缺失. " + wsCombatShellPath).c_str());
		return 0;
	}
#ifdef _WIN64
	m_studBase = LoadLibraryEx(wsCombatShellPath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);
#else
	m_studBase = LoadLibraryEx(wsCombatShellPath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);
#endif
	if (!m_studBase || (nullptr == m_studBase)) {
		AfxMessageBox((L"CombatShell LoadLibraryEx Error. " + wsCombatShellPath).c_str());
		return false;
	}
	// 获取dll的导出函数
#ifdef _WIN64
	dexportAddress = GetProcAddress((HMODULE)m_studBase, "VmEntry");
#else
	dexportAddress = GetProcAddress((HMODULE)m_studBase, "CombatShellEntry");
#endif
	// ImageBase
#ifdef _WIN64
	m_dwStudSectionAddress64 = (DWORD64)SinglePuPEInfo::instance()->puGetSectionAddress((char *)m_studBase, (BYTE *)".text");
	m_dwNewSectionAddress64 = (DWORD64)SinglePuPEInfo::instance()->puGetSectionAddress((char *)m_lpBase, (BYTE *)NEWSECITONNAME);
	m_ImageBase64 = ((PIMAGE_NT_HEADERS)SinglePuPEInfo::instance()->puGetNtHeadre())->OptionalHeader.ImageBase;
#else
	m_dwStudSectionAddress = (DWORD)pPeObj->puGetSectionAddress((char *)m_studBase, (BYTE *)".text");
	m_dwNewSectionAddress = (DWORD)pPeObj->puGetSectionAddress((char *)m_lpBase, (BYTE *)NEWSECITONNAME);
	m_ImageBase = ((PIMAGE_NT_HEADERS)pPeObj->puGetNtHeadre())->OptionalHeader.ImageBase;
#endif // _WIN64

	return TRUE;
}

// 修复重定位
BOOL studData::RepairReloCationStud()
{
	PIMAGE_DOS_HEADER pStuDos = (PIMAGE_DOS_HEADER)m_studBase;
#ifdef _WIN64
	PIMAGE_NT_HEADERS pStuNt = (PIMAGE_NT_HEADERS)(pStuDos->e_lfanew + (DWORD64)m_studBase);
	PIMAGE_BASE_RELOCATION pStuRelocation = (PIMAGE_BASE_RELOCATION)(pStuNt->OptionalHeader.DataDirectory[5].VirtualAddress + (DWORD64)m_studBase);
#else
	PIMAGE_NT_HEADERS pStuNt = (PIMAGE_NT_HEADERS)(pStuDos->e_lfanew + (DWORD)m_studBase);
	PIMAGE_BASE_RELOCATION pStuRelocation = (PIMAGE_BASE_RELOCATION)(pStuNt->OptionalHeader.DataDirectory[5].VirtualAddress + (DWORD)m_studBase);
#endif // _WIN64

	typedef struct _Node
	{
		WORD offset : 12;
		WORD type : 4;
	}Node, *PNode;

#ifdef _WIN64
	LONGLONG dwDelta = (__int64)m_studBase - m_ImageBase64;
#endif
	DWORD OldAttribute = 0;
	while (pStuRelocation->SizeOfBlock)
	{
		DWORD nStuRelocationBlockCount = (pStuRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;

		_Node* RelType = (PNode)(pStuRelocation + 1);

		for (DWORD i = 0; i < nStuRelocationBlockCount; ++i)
		{
			if (RelType[i].type == 3) {
				DWORD* pRel = (DWORD *)(pStuRelocation->VirtualAddress + RelType[i].offset + (DWORD)m_studBase);

				VirtualProtect(pRel, 8, PAGE_READWRITE, &OldAttribute);

				*pRel = *pRel - (DWORD)m_studBase - ((PIMAGE_SECTION_HEADER)m_dwStudSectionAddress)->VirtualAddress + m_ImageBase + ((PIMAGE_SECTION_HEADER)m_dwNewSectionAddress)->VirtualAddress;

				VirtualProtect(pRel, 8, OldAttribute, &OldAttribute);
			}
#ifdef _WIN64
			if (RelType->type == 10) {
				PULONGLONG pAddress = (PULONGLONG)((DWORD64)m_studBase + pStuRelocation->VirtualAddress + RelType[i].offset);
				VirtualProtect(pAddress, 8, PAGE_READWRITE, &OldAttribute);
				*pAddress += dwDelta;
				//*pAddress = *pAddress - (DWORD64)m_studBase - ((PIMAGE_SECTION_HEADER)m_dwStudSectionAddress64)->VirtualAddress + ((PIMAGE_SECTION_HEADER)m_dwNewSectionAddress64)->VirtualAddress + m_ImageBase64;
				VirtualProtect(pAddress, 8, OldAttribute, &OldAttribute);
			}

#endif // _WIN64
		}
#ifdef _WIN64
		pStuRelocation = (PIMAGE_BASE_RELOCATION)((DWORD64)pStuRelocation + pStuRelocation->SizeOfBlock);
#else
		pStuRelocation = (PIMAGE_BASE_RELOCATION)((DWORD)pStuRelocation + pStuRelocation->SizeOfBlock);
#endif
	}
	return TRUE;
}

// 拷贝stud数据到新增区段
BOOL studData::CopyStud()
{
	g_stu->s_dwOepBase = m_Oep;
	FILE* fpFile = nullptr;
	if ((fpFile = fopen(g_CombatShellDataLocalFile, "ab+")) == NULL) {
	
		AfxMessageBox(L"文件打开失败");
		return false;
	}

	fwrite(&m_Oep, sizeof(DWORD), 1, fpFile);
	fclose(fpFile);
	PIMAGE_SECTION_HEADER studSection = SinglePuPEInfo::instance()->puGetSectionAddress((char *)m_studBase, (BYTE *)".text");
	PIMAGE_SECTION_HEADER SurceBase = SinglePuPEInfo::instance()->puGetSectionAddress((char *)m_lpBase, (BYTE *)NEWSECITONNAME);
	if (!studSection || (!SurceBase))
		return false;
#ifdef _WIN64
	memcpy(
		(void *)(SurceBase->PointerToRawData + (DWORD64)m_lpBase),
		(void *)(studSection->VirtualAddress + (DWORD64)m_studBase),
		studSection->Misc.VirtualSize
	);
#else
	memcpy(
		(void *)(SurceBase->PointerToRawData + (DWORD)m_lpBase),
		(void *)(studSection->VirtualAddress + (DWORD)m_studBase),
		studSection->Misc.VirtualSize
	);
#endif

	DWORD dwRiteFile = 0;	OVERLAPPED overLapped = { 0 };
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)SinglePuPEInfo::instance()->puGetNtHeadre();
	if (!pNt)
		return false;

	// OEP
#ifdef _WIN64
	pNt->OptionalHeader.AddressOfEntryPoint = (DWORD64)dexportAddress - (DWORD64)m_studBase - studSection->VirtualAddress + SurceBase->VirtualAddress;
#else
	pNt->OptionalHeader.AddressOfEntryPoint = (DWORD)dexportAddress - (DWORD)m_studBase - studSection->VirtualAddress + SurceBase->VirtualAddress;
#endif

	int nRet = WriteFile(SinglePuPEInfo::instance()->puFileHandle(), SinglePuPEInfo::instance()->puGetImageBase(), SinglePuPEInfo::instance()->puFileSize(), &dwRiteFile, &overLapped);
	if (!nRet)
		return FALSE;
	return TRUE;
}

// Clear
void studData::puClearStuData()
{
	SinglePuPEInfo::instance()->puClearPeData();
	if (m_lpBase)
		m_lpBase = nullptr;
	if (m_studBase)
		m_studBase = nullptr;
	m_Oep = 0;
	m_ImageBase = 0;
}