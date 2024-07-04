#pragma once
/*
	类名称：studData
	用途：stud数据相关操作
	时间：2018/12/1
	修改日期：2018/12/2
*/

class studData
{
public:
	studData();
	~studData();

public:
	BOOL puInit(CString csFilePath) { 
		m_MasterFilePath = csFilePath.GetString();
		return this->InitStuData();
	}
	void puLoadLibraryStud(){ this->LoadLibraryStud(); }
	void puRepairReloCationStud(){ this->RepairReloCationStud(); }
	BOOL puCopyStud(){ return this->CopyStud(); }
	void puClearStuData();

private:
	BOOL InitStuData();
 	BOOL LoadLibraryStud();
	BOOL RepairReloCationStud();
	BOOL CopyStud();

private:

	void* dexportAddress = 0; // main
	void* WinMain = 0;		  // WinMain

	void* m_studBase = nullptr;

	void* m_lpBase = nullptr;

	DWORD m_dwNewSectionAddress = 0;
	DWORD64 m_dwNewSectionAddress64 = 0;

	DWORD m_dwStudSectionAddress = 0;
	DWORD64 m_dwStudSectionAddress64 = 0;

	DWORD64 m_Oep = 0;

	DWORD m_ImageBase = 0;
	DWORD64 m_ImageBase64 = 0;

	CString m_MasterFilePath;
};

using SingleStudData = ustdex::Singleton<studData>;