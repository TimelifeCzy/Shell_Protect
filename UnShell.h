#pragma once
#ifndef UNSHELL_H_
#define UNSHELL_H_
#include "stdafx.h"

class UnShell
{
public:
	UnShell();
	~UnShell();

public:
	BOOL puRepCompressionData(){ return this->RepCompressionData(); }
	BOOL puDeleteSectionInfo(){ return this->DeleteSectionInfo(); }
	BOOL puSaveUnShell(){ return this->SaveUnShell(); }

private:
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
	DWORD TotaldwSize = 0;
	char* Sectionbuf = nullptr;
	FILE *fpFile = nullptr;
};

#endif