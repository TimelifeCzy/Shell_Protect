// SectionInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "SectionInfo.h"
#include "afxdialogex.h"
#include "puPEinfoData.h"

const TCHAR SectionInfo::strName[6][20] = { L"Name", L"V.Address", L"V.Size", L"Offset", L"R.Size", L"Flag" };

// SectionInfo 对话框

IMPLEMENT_DYNAMIC(SectionInfo, CDialogEx)

SectionInfo::SectionInfo(CWnd* pParent /*=NULL*/)
	: CDialogEx(SectionInfo::IDD, pParent)
{

}

SectionInfo::~SectionInfo()
{
}

void SectionInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_SectionList);
}


BEGIN_MESSAGE_MAP(SectionInfo, CDialogEx)
END_MESSAGE_MAP()



BOOL SectionInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect rc;

	DWORD dwSize = m_SectionList.GetExtendedStyle();

	m_SectionList.SetExtendedStyle(dwSize | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	m_SectionList.GetClientRect(rc);

	int nWidth = rc.Width();

	for (int i = 0; i < 6; ++i){ m_SectionList.InsertColumn(i, strName[i], LVCFMT_CENTER, nWidth / 6); }

	PuPEInfo obj_peinfo;

	pSectionHeadre = { 0 };	pNtHeadre = { 0 };

	pSectionHeadre = (PIMAGE_SECTION_HEADER)obj_peinfo.puGetSection();

	pNtHeadre = (PIMAGE_NT_HEADERS)obj_peinfo.puGetNtHeadre();

	SectionCount = pNtHeadre->FileHeader.NumberOfSections;

	ShowSectionInfo();

	return TRUE;
}

void SectionInfo::ShowSectionInfo()
{ 
	PIMAGE_SECTION_HEADER tempSectionHeadre = pSectionHeadre;

	DWORD dwTemp = 0;	CString str;

	for (DWORD i = 0; i < SectionCount; ++i)
	{

		m_SectionList.InsertItem(i, NULL);
		str = tempSectionHeadre->Name;
		m_SectionList.SetItemText(i, 0, str);

		dwTemp = tempSectionHeadre->VirtualAddress;
		str.Format(L"%08X", dwTemp);
		m_SectionList.SetItemText(i, 1, str);

		dwTemp = tempSectionHeadre->SizeOfRawData;
		str.Format(L"%08X", dwTemp);
		m_SectionList.SetItemText(i, 2, str);

		dwTemp = tempSectionHeadre->PointerToRawData;
		str.Format(L"%08X", dwTemp);
		m_SectionList.SetItemText(i, 3, str);
		
		dwTemp = tempSectionHeadre->Misc.VirtualSize;
		str.Format(L"%08X", dwTemp);
		m_SectionList.SetItemText(i, 4, str);

		dwTemp = tempSectionHeadre->Characteristics;
		str.Format(L"%08X", dwTemp);
		m_SectionList.SetItemText(i, 5, str);

		++tempSectionHeadre;
	}
}