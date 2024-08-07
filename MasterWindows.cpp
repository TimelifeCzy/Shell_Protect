﻿#include "stdafx.h"
#include "MasterWindows.h"
#include "afxdialogex.h"
#include "puPEinfoData.h"
#include "SectionInfo.h"
#include "AddSection.h"
#include "studData.h"
#include "CompressionData.h"
#include "UnShell.h"
#include <io.h>

// 脱壳本地文件.
CString UnShllerProcPath;
char g_CombatShellDataLocalFile[MAX_PATH] = { 0, };

// MasterWindows
IMPLEMENT_DYNAMIC(MasterWindows, CDialogEx)
#define NEWSECITONNAME ".VMP"

MasterWindows::MasterWindows(CWnd* pParent /*=NULL*/)
	: CDialogEx(MasterWindows::IDD, pParent)
	, m_MasterStaticTextStr(_T(""))
{
}

MasterWindows::~MasterWindows()
{
}

void MasterWindows::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC1, m_MasterStaticText);
	DDX_Text(pDX, IDC_STATIC1, m_MasterStaticTextStr);
	DDX_Control(pDX, IDC_BITMAP_LOG, m_bitmapZionloab);
}

BEGIN_MESSAGE_MAP(MasterWindows, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &MasterWindows::OnBnClickedButton1)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON4, &MasterWindows::OnBnClickedButton4)
	// ON_BN_CLICKED(IDC_BUTTON3, &MasterWindows::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON9, &MasterWindows::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON3, &MasterWindows::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, &MasterWindows::OnBnClickedButton2)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL MasterWindows::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
	SetIcon(LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_ICON1)), TRUE);

	CBitmap* d = new CBitmap();
	d->LoadBitmapW(IDB_BITMAP2);
	m_bitmapZionloab.SetBitmap((HBITMAP)d->m_hObject);
	// m_Logbmp.SetBitmap((HBITMAP)d->m_hObject);

	//加载位图
	CBitmap bmp;
	bmp.LoadBitmapW(MAKEINTRESOURCE(IDB_BITMAP1));
	//添加位图
	m_bmp.Attach(bmp);
	//创建兼容DC
	CDC* pDc = GetDC();
	m_dc.CreateCompatibleDC(pDc);
	//用完之后释放
	ReleaseDC(pDc);
	//把位图对象选入DC中
	m_dc.SelectObject(&m_bmp);
	//使窗口无效,这样OnPaint函数就会被触发,使之被画出来
	Invalidate(FALSE);
	return TRUE;
}

// 新增节区
BOOL MasterWindows::NewSection()
{
	BOOL nRet = TRUE;
	BYTE Name[] = NEWSECITONNAME;

	// 添加区段提前申请Stub大小
	DWORD SectionSize = 0;
	std::string sDriectory = "";
	CodeTool::CGetCurrentDirectory(sDriectory);
	if (!sDriectory.empty()) {
		const std::string sStuFile = (sDriectory + "CombatShell.dll").c_str();
		const HANDLE hFile = CreateFileA(sStuFile.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if ((hFile != NULL) && hFile) {
			SectionSize = GetFileSize(hFile, &SectionSize);
			CloseHandle(hFile);
		}
		if (SectionSize <= 0)
			return false;
	}
	else
		return false;

	SingleAddSection::instance()->puInti(m_MasterStaticTextStr);
	m_dwOldOEP = SinglePuPEInfo::instance()->puGetOEP();
	SingleAddSection::instance()->puModifySectioNumber();
	nRet = SingleAddSection::instance()->puModifySectionInfo(Name, SectionSize);
	if (!nRet) {
		SingleAddSection::instance()->puFree();
		return false;
	}
	SingleAddSection::instance()->puModifyProgramEntryPoint();
	SingleAddSection::instance()->puModifySizeofImage();
	nRet = SingleAddSection::instance()->puAddNewSectionByData(SectionSize);
	SingleAddSection::instance()->puFree();
	return nRet;
}

// 节区显示
void MasterWindows::ShowPEInfoData(const CString& FileName)
{
	CString Tempstr;
	DWORD TempdwCode = 0;
	// open
	if (!SinglePuPEInfo::instance()->puOpenFileLoadEx(FileName))
		return;
	PIMAGE_NT_HEADERS pNtHeadre = (PIMAGE_NT_HEADERS)SinglePuPEInfo::instance()->puGetNtHeadre();
	PIMAGE_FILE_HEADER pFileHeadre = (PIMAGE_FILE_HEADER)&pNtHeadre->FileHeader;
	PIMAGE_OPTIONAL_HEADER pOption = (PIMAGE_OPTIONAL_HEADER)&pNtHeadre->OptionalHeader;
	if (!pNtHeadre || (!pFileHeadre) || (!pOption))
		return;
	try
	{
		TempdwCode = pFileHeadre->NumberOfSections;
		Tempstr.Format(L"%d", TempdwCode);
		SetDlgItemText(IDC_EDIT9, Tempstr);

		// OEP
		TempdwCode = pOption->AddressOfEntryPoint;
		Tempstr.Format(L"%08X", TempdwCode);
		SetDlgItemText(IDC_EDIT1, Tempstr);

		TempdwCode = pOption->ImageBase;
		Tempstr.Format(L"%08X", TempdwCode);
		SetDlgItemText(IDC_EDIT3, Tempstr);

		TempdwCode = pOption->Magic;
		Tempstr.Format(L"%04X", TempdwCode);
		SetDlgItemText(IDC_EDIT2, Tempstr);

		TempdwCode = pOption->NumberOfRvaAndSizes;
		Tempstr.Format(L"%08X", TempdwCode);
		SetDlgItemText(IDC_EDIT7, Tempstr);

		TempdwCode = pOption->BaseOfCode;
		Tempstr.Format(L"%08X", TempdwCode);
		SetDlgItemText(IDC_EDIT4, Tempstr);

#ifdef _WIN64

#else
		TempdwCode = pOption->BaseOfData;
		Tempstr.Format(L"%08X", TempdwCode);
		SetDlgItemText(IDC_EDIT5, Tempstr);
#endif

		TempdwCode = pOption->SectionAlignment;
		Tempstr.Format(L"%08X", TempdwCode);
		SetDlgItemText(IDC_EDIT6, Tempstr);

		TempdwCode = pOption->FileAlignment;
		Tempstr.Format(L"%08X", TempdwCode);
		SetDlgItemText(IDC_EDIT8, Tempstr);

		// clear
		SinglePuPEInfo::instance()->puClearPeData();
	}
	catch (const std::exception&)
	{
		// clear
		SinglePuPEInfo::instance()->puClearPeData();
		return;
	}
}

// 拖拽文件
void MasterWindows::OnDropFiles(HDROP hDropInfo)
{
	int DropCount = DragQueryFile(hDropInfo, -1, NULL, 0);
	char wcStr[MAX_PATH] = {};
	for (int i = 0; i < DropCount; ++i)
	{
		wcStr[0] = 0;
		DragQueryFileA(hDropInfo, i, wcStr, MAX_PATH);
		m_MasterStaticTextStr = wcStr;
	}

	UpdateData(FALSE);

	// Show PE Info
	ShowPEInfoData(m_MasterStaticTextStr);

	// 脱壳本地数据存储
	CString nStr, sFileName, sDirectory;
	nStr = m_MasterStaticTextStr;
	{
		sFileName = m_MasterStaticTextStr;
		int n = sFileName.ReverseFind('\\') + 1;
		int m = sFileName.GetLength() - n;
		const CString csTargetDirectory = sFileName.Left(n);
		sDirectory = sFileName.Left(n);
		sFileName = sFileName.Right(m);
		
		int nFileName = sFileName.FindOneOf(L".exe");
		sFileName = sFileName.Left(nFileName);
		sFileName = (sDirectory + sFileName).GetString();

		DWORD dwNum = 0;
		RtlSecureZeroMemory(g_CombatShellDataLocalFile, 0);
		dwNum = WideCharToMultiByte(CP_OEMCP, NULL, sFileName, -1, NULL, NULL, 0, NULL);
		WideCharToMultiByte(CP_OEMCP, NULL, sFileName, -1, g_CombatShellDataLocalFile, dwNum, 0, NULL);
		if (strlen(g_CombatShellDataLocalFile) > 0) {
			strcat(g_CombatShellDataLocalFile, "_CombatShellData.dat");
		}
		else
		{
			strcat(g_CombatShellDataLocalFile, "CombatShellData.dat");
			AfxMessageBox((L"本地Combat存储文件名获取失败,  默认 " + CodeTool::string2wstring(g_CombatShellDataLocalFile)).c_str());
		}
	}

	DragFinish(hDropInfo);
	CDialogEx::OnDropFiles(hDropInfo);
}

// 一键加壳
void MasterWindows::OnBnClickedButton1()
{
	UpdateData(TRUE);
	if (m_MasterStaticTextStr.IsEmpty())
	{
		AfxMessageBox(L"请先拖入文件");
		return;
	}

	// 保存Path，拷贝使用
	CString nStr, tmep;
	nStr = m_MasterStaticTextStr;

	// 获取文件名
	tmep = m_MasterStaticTextStr;
	int n = tmep.ReverseFind('\\') + 1;
	int m = tmep.GetLength() - n;
	const CString csTargetDirectory = tmep.Left(n);
	tmep = tmep.Right(m);

	// 保存旧文件
	CopyFile(m_MasterStaticTextStr, (csTargetDirectory + "old_" + tmep).GetString(), FALSE);

	// 1. 新增区段
	if (!NewSection()) {
		AfxMessageBox(L"添加新区段失败");
		return;
	}

	// 2. 压缩全部区段 压缩的时候不清空数据目录表以及区段大小（不压缩新增区段）
	{
		CompressionData obj_ComperData;
		obj_ComperData.puInit(m_MasterStaticTextStr);
		if (!obj_ComperData.puCompressSection()) {
			AfxMessageBox(L"CompressSection failuer!");
			return;
		}
	}

	// 3. CombatShell 数据拷贝/操作
	m_MasterStaticTextStr = (csTargetDirectory + "CompressionMask.exe").GetBSTR();
	if (!SingleStudData::instance()->puInit(m_MasterStaticTextStr, m_dwOldOEP)) {
		AfxMessageBox(L"studData failuer!");
		return;
	}
	SingleStudData::instance()->puLoadLibraryStud();
	SingleStudData::instance()->puRepairReloCationStud();
	const bool bSuc = SingleStudData::instance()->puCopyStud();
	SingleStudData::instance()->puClearStuData();
	if (bSuc)
	{
		// clear exe
		DeleteFile(nStr.GetString());
		int nstatus = CopyFile(m_MasterStaticTextStr.GetString(), nStr.GetString(), FALSE);
		if (nstatus) {
			DeleteFile(m_MasterStaticTextStr.GetString());
			m_MasterStaticTextStr = nStr;
		}
		ShowPEInfoData(m_MasterStaticTextStr);
		AfxMessageBox((m_MasterStaticTextStr + L"   Success!").GetString());
	}
	else
		AfxMessageBox((m_MasterStaticTextStr + L"   Failure!").GetString());
}

// PE View
void MasterWindows::OnBnClickedButton4()
{
	UpdateData(TRUE);
	if (m_MasterStaticTextStr.IsEmpty())
	{
		AfxMessageBox(L"请先拖入文件");
		return;
	}

	SectionInfo cSection;
	cSection.SetAnalyzeFilePath(m_MasterStaticTextStr);
	cSection.DoModal();
	return;
}

// 新增区段
void MasterWindows::OnBnClickedButton9()
{
	UpdateData(TRUE);
	if (m_MasterStaticTextStr.IsEmpty())
	{
		AfxMessageBox(L"请先拖入文件");
		return;
	}
	if (NewSection()) {
		ShowPEInfoData(m_MasterStaticTextStr);
		AfxMessageBox(L"Success");
	}
	else {
		AfxMessageBox(L"Fauilter");
	}
}

// 仅压缩
void MasterWindows::OnBnClickedButton3()
{
	UpdateData(TRUE);
	if (m_MasterStaticTextStr.IsEmpty())
	{
		AfxMessageBox(L"请先拖入文件");
		return;
	}
	UpdateData(TRUE);
	if (m_MasterStaticTextStr.IsEmpty())
	{
		AfxMessageBox(L"请先拖入文件");
		return;
	}
	CompressionData obj_ComperData;
	obj_ComperData.puInit(m_MasterStaticTextStr);
	if (!obj_ComperData.puCompressSection())
		AfxMessageBox(L"CompressSection failuer!");
	else
		AfxMessageBox(L"CompressSection Seucess!");
	ShowPEInfoData(m_MasterStaticTextStr);
}

// 一键脱壳
void MasterWindows::OnBnClickedButton2()
{
	UpdateData(TRUE);
	if (m_MasterStaticTextStr.IsEmpty())
	{
		MessageBox(L"请先拖入文件");
		return;
	}

	// 判断是否我们的壳，否则不给脱壳(未检测.)
	UnShllerProcPath = m_MasterStaticTextStr;

	UnShell obj_Unshell;
	if (!obj_Unshell.puUnShell()) {
		MessageBox(L"puUnShell error");
		return;
	}
	if (!obj_Unshell.puRepCompressionData()) {
		MessageBox(L"puRepCompressionData error.");
		return;
	}
	if(!obj_Unshell.puDeleteSectionInfo()) {
		MessageBox(L"puDeleteSectionInfo error.");
		return;
	}

	if (obj_Unshell.puSaveUnShell())
	{
		const std::wstring sUnShellPath = CodeTool::string2wstring(obj_Unshell.puGetUnShellPath().c_str()).c_str();
		// Clear
		obj_Unshell.puClose();
		DeleteFile(m_MasterStaticTextStr);
		int nRet = CopyFile(sUnShellPath.c_str(), m_MasterStaticTextStr, FALSE);
		if (nRet) {
			DeleteFile(sUnShellPath.c_str());
			DeleteFileA(g_CombatShellDataLocalFile);
		}
		MessageBox(L"puSaveUnShell_Success.");
	}
	ShowPEInfoData(m_MasterStaticTextStr);
}

HBRUSH MasterWindows::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_STATIC  :
	case IDC_STATIC_PEINFO :
	case IDC_STATIC_NAME:
	case IDC_BUTTON1 :
	case IDC_BUTTON4 :
	case IDC_BUTTON9 :
	case IDC_BUTTON3 :
	case IDC_BUTTON2 :
	{
		pDC->SetBkColor(RGB(255, 255, 255));
		return (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	}
	break;
	default:
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void MasterWindows::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CDialogEx::OnPaint() for painting messages
	CRect rect = { 0, };
	GetClientRect(&rect);

	BITMAP bm;
	m_bmp.GetBitmap(&bm);
	dc.StretchBlt(0, 0, rect.Width(), rect.Height(),
		&m_dc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	CDialogEx::OnPaint();
}
