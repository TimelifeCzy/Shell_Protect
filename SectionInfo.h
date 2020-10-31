#pragma once
#include "stdafx.h"
#include "resource.h"
#include "afxcmn.h"

// SectionInfo 对话框

class SectionInfo : public CDialogEx
{
	DECLARE_DYNAMIC(SectionInfo)

public:
	SectionInfo(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~SectionInfo();

// 对话框数据
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_SectionList;
	virtual BOOL OnInitDialog();

	/*自定义数据*/
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
};
