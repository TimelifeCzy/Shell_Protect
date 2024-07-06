#pragma once
#include "stdafx.h"

class UnShell
{
public:
	UnShell();
	~UnShell();

public:
	BOOL puUnShell() { return this->UnShellEx(); }
	BOOL puRepCompressionData(){ return this->RepCompressionData(); }
	BOOL puDeleteSectionInfo(){ return this->DeleteSectionInfo(); }
	BOOL puSaveUnShell(){ return this->SaveUnShell(); }
	const std::string puGetUnShellPath() { return m_sUnShellPath.c_str(); }
	void puClose();
private:
	BOOL UnShellEx();
	BOOL RepCompressionData();
	BOOL DeleteSectionInfo();
	BOOL SaveUnShell();

private:
	void* m_Base = nullptr;
	void* m_NtAddress = nullptr;
	HANDLE hFile = nullptr;
	void* m_studBase = nullptr;
	PIMAGE_DOS_HEADER pDosHander;
	PIMAGE_NT_HEADERS pHeadres;
	PIMAGE_SECTION_HEADER pSection;
	char* UnShellNewFile = nullptr;
	DWORD m_dwTotaldwSize = 0;
	char* m_pSectionData = nullptr;
	FILE *fpFile = nullptr;

	std::string m_sUnShellPath = "";
};

using SingleUnShell = ustdex::Singleton<UnShell>;