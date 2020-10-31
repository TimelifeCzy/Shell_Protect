#pragma once
#include "stdafx.h"
#include "resource.h"
#include "afxwin.h"


// MasterWindows

class MasterWindows : public CDialogEx
{
	DECLARE_DYNAMIC(MasterWindows)

public:
	MasterWindows(CWnd* pParent = NULL);			
	virtual ~MasterWindows();

	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CStatic m_MasterStaticText;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	CString m_MasterStaticTextStr;

public:
	afx_msg void OnBnClickedButton4();
//	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton9();

	CBitmap m_bmp;   //位图
	CBrush m_brush;  //画刷
	CDC m_dc;        //DC对象

private:

	void ShowPEInfoData(const CString & FileName);

	BOOL NewSection();
public:
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton2();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	CStatic m_bitmapZionloab;
};
