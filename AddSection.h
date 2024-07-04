#pragma once
#ifndef ADDSECTION_H_
#define ADDSECTION_H_

/*
	类名称：AddSection
	用途：添加一个区段
	时间：2018/11/30
*/

class AddSection
{
public:
	AddSection();

	~AddSection();

public:
	void puInti(CString sFilePath) {
		m_FilePath = sFilePath;
		this->Init();
	}

	void puFree() {
		m_FilePath.Empty();
		this->Free();
	}

	void puModifySectioNumber(){ this->ModifySectionNumber(); }

	void puModifyProgramEntryPoint(){ this->ModifyProgramEntryPoint(); }

	void puModifySizeofImage(){ this->ModifySizeofImage(); }

	BOOL puModifySectionInfo(BYTE* Name, const DWORD & size){ return this->ModifySectionInfo(Name, size); }

	BOOL puAddNewSectionByData(const DWORD & size){ return this->AddNewSectionByteData(size); }

	void* puGetNewBaseAddress(){ return this->m_newlpBase; }

	DWORD puGetNewBaseSize(){ return this->FileSize + 0x1000; }


private:
	BOOL Init();

	BOOL Free();

	BOOL ModifySectionNumber();

	BOOL ModifySectionInfo(const BYTE* Name, const DWORD & size);

	BOOL ModifyProgramEntryPoint();

	BOOL ModifySizeofImage();

	BOOL AddNewSectionByteData(const DWORD & size);

private:

	void* pFileBaseData = nullptr;

	void* pNtHeadre = nullptr;

	void* pSectionHeadre = nullptr;

	DWORD SectionSizeof = 0;

	DWORD FileSize = 0;

	HANDLE FileHandle = nullptr;

	PIMAGE_SECTION_HEADER NewpSection = { 0 };

	DWORD64 OldOep = 0;

	CString m_FilePath;

	char* m_newlpBase = nullptr;
};
#endif

using SingleAddSection = ustdex::Singleton<AddSection>;