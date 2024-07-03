#pragma once
#include "resource.h"

class SectionInfo : public CDialogEx
{
	DECLARE_DYNAMIC(SectionInfo)

public:
	SectionInfo(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~SectionInfo();
	enum { IDD = IDD_DIALOG2 };
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	void InitSectionData();
	void SetAnalyzeFilePath(CString csPath) { m_csFilePath = csPath; }

private:
	// ��ʾ������Ϣ
	void ShowSectionInfo();
	// �б���ʾ
	static const TCHAR strName[6][20];
	// ����������Ϣ
	PIMAGE_SECTION_HEADER pSectionHeadre;
	// ������������
	PIMAGE_NT_HEADERS pNtHeadre;
	DWORD SectionCount = 0;
	CListCtrl m_SectionList;
	// �����ļ�·��
	CString m_csFilePath;
};

using SingleSectionInfo = ustdex::Singleton<SectionInfo>;