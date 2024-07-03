#pragma once
#include "resource.h"

class SectionInfo : public CDialogEx
{
	DECLARE_DYNAMIC(SectionInfo)

public:
	SectionInfo(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~SectionInfo();
	enum { IDD = IDD_DIALOG2 };
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	void InitSectionData();
	void SetAnalyzeFilePath(CString csPath) { m_csFilePath = csPath; }

private:
	// 显示区段信息
	void ShowSectionInfo();
	// 列表显示
	static const TCHAR strName[6][20];
	// 保存区段信息
	PIMAGE_SECTION_HEADER pSectionHeadre;
	// 保存区段数量
	PIMAGE_NT_HEADERS pNtHeadre;
	DWORD SectionCount = 0;
	CListCtrl m_SectionList;
	// 解析文件路径
	CString m_csFilePath;
};

using SingleSectionInfo = ustdex::Singleton<SectionInfo>;