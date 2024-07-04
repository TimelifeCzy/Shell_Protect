#pragma once
#ifndef PUPEINFODATA_H_
#define PUPEINFODATA_H_

/*
	�����ƣ�PuPeInfo
	����;�������ӿ��࣬�ṩ��ִ���ļ���PE��Ϣ
	ʱ�䣺	2018-11-29
*/
class PuPEInfo
{
public:
	PuPEInfo();
	~PuPEInfo();
	
	/*�����ӿ�*/
public:

	const bool puGetLoadFileSuccess() { return m_bLoadFileSuc; }

	void* puGetImageBase(){ 
		return m_pFileBase;
	}

	void* puGetNtHeadre(){ return m_pNtHeader; }

	void* puGetSection(){ return m_SectionHeader; }

	DWORD puFileSize(){ return m_FileSize; }

	BOOL puOpenFileLoad(const CString & PathName){ return prOpenFile(PathName); }
	
	BOOL puOpenFileLoadEx(const CString& PathName) { return prOpenFileEx(PathName); }

	void puClearPeData();

	BOOL puIsPEFile(){ return IsPEFile(); }

	DWORD puRVAofFOA(const DWORD Rva){ return RVAofFOA(Rva); }

	CString puFilePath(){ return m_strNamePath; }

	HANDLE puFileHandle() {
		return m_hFileHandle; 
	}

	DWORD64 puOldOep(){ return this->m_OldOEP; }

	int puGetSectionCount() { return this->m_SectionCount; }

	PIMAGE_SECTION_HEADER puGetSectionAddress(const char* Base, const BYTE* Name){ return this->GetSectionAddress(Base, Name); }

	// �����ļ�ƫ���Լ��ļ���С
	BOOL puSetFileoffsetAndFileSize(const void* Base, const DWORD & offset, const DWORD size, const BYTE* Name)
	{
		return this->SetFileoffsetAndFileSize(Base, offset, size, Name);
	}


private:
	// �ļ���ȡ
	BOOL prOpenFile(const CString & PathName);

	// ˢ���ļ�����
	BOOL prOpenFileEx(const CString& PathName);

	// PE�ļ��ж�
	BOOL IsPEFile();

	// RVAofFOA
	DWORD RVAofFOA(const DWORD Rva);

	// �����������ƻ�ȡ�����׵�ַ
	PIMAGE_SECTION_HEADER GetSectionAddress(const char* Base, const BYTE* SectionName);

	// �����ļ�ƫ���Լ��ļ���С
	BOOL SetFileoffsetAndFileSize(const void* Base, const DWORD & offset, const DWORD size, const BYTE* Name);

public:
	// ����ImageBase
	void* m_pFileBase = nullptr;

	// ����PEͷ
	void* m_pNtHeader = nullptr;

	// ����Section
	void* m_SectionHeader = nullptr;

	// �����ļ���С
	DWORD m_FileSize = 0;

	// �����ļ�·��
	CString m_strNamePath;

	// �����ļ����
	HANDLE m_hFileHandle = nullptr;

	// ����ԭʼOEP
	DWORD m_OldOEP = 0;

	// �������θ���
	int	m_SectionCount = 0;

	// ���ԭʼOEP
	BOOL OepFlag = FALSE;

	bool m_bLoadFileSuc = false;
};

#endif

using SinglePuPEInfo = ustdex::Singleton<PuPEInfo>;
