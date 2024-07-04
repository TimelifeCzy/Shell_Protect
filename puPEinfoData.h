#pragma once
#ifndef PUPEINFODATA_H_
#define PUPEINFODATA_H_

/*
	类名称：PuPeInfo
	类用途：公共接口类，提供可执行文件的PE信息
	时间：	2018-11-29
*/
class PuPEInfo
{
public:
	PuPEInfo();
	~PuPEInfo();
	
	/*公开接口*/
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

	// 设置文件偏移以及文件大小
	BOOL puSetFileoffsetAndFileSize(const void* Base, const DWORD & offset, const DWORD size, const BYTE* Name)
	{
		return this->SetFileoffsetAndFileSize(Base, offset, size, Name);
	}


private:
	// 文件读取
	BOOL prOpenFile(const CString & PathName);

	// 刷新文件数据
	BOOL prOpenFileEx(const CString& PathName);

	// PE文件判断
	BOOL IsPEFile();

	// RVAofFOA
	DWORD RVAofFOA(const DWORD Rva);

	// 根据区段名称获取区段首地址
	PIMAGE_SECTION_HEADER GetSectionAddress(const char* Base, const BYTE* SectionName);

	// 设置文件偏移以及文件大小
	BOOL SetFileoffsetAndFileSize(const void* Base, const DWORD & offset, const DWORD size, const BYTE* Name);

public:
	// 保存ImageBase
	void* m_pFileBase = nullptr;

	// 保存PE头
	void* m_pNtHeader = nullptr;

	// 保存Section
	void* m_SectionHeader = nullptr;

	// 保存文件大小
	DWORD m_FileSize = 0;

	// 保存文件路径
	CString m_strNamePath;

	// 保存文件句柄
	HANDLE m_hFileHandle = nullptr;

	// 保存原始OEP
	DWORD m_OldOEP = 0;

	// 保存区段个数
	int	m_SectionCount = 0;

	// 标记原始OEP
	BOOL OepFlag = FALSE;

	bool m_bLoadFileSuc = false;
};

#endif

using SinglePuPEInfo = ustdex::Singleton<PuPEInfo>;
