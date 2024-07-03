// MasterWindows.cpp 
#include "stdafx.h"
#include "MasterWindows.h"
#include "afxdialogex.h"
#include "puPEinfoData.h"
#include "SectionInfo.h"
#include "AddSection.h"
#include "studData.h"
#include "CompressionData.h"
#include "UnShell.h"

CString UnShllerProcPath;

char g_filenameonly[MAX_PATH] = { 0, };

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

// 一键加壳
void MasterWindows::OnBnClickedButton1()
{
	UpdateData(TRUE);
	if (m_MasterStaticTextStr.IsEmpty())
	{
		AfxMessageBox(L"请先拖入文件");
		return;
	}
	// ☆ 先增加新区~段后压缩
	CString nStr, tmep;
	PuPEInfo obj_Peinfo;

	// 1. 新增区段
	if (!NewSection())
		AfxMessageBox(L"添加新区段失败");

	CloseHandle(obj_Peinfo.puFileHandle()); UpdateData(TRUE);

	// 保存Path，拷贝使用
	nStr = m_MasterStaticTextStr;

	// 获取文件名
	tmep = m_MasterStaticTextStr;
	int n = tmep.ReverseFind('\\') + 1;
	int m = tmep.GetLength() - n;
	tmep = tmep.Right(m);

	DWORD dwNum = 0;
	RtlSecureZeroMemory(g_filenameonly, 0);
	dwNum = WideCharToMultiByte(CP_OEMCP, NULL, tmep, -1, NULL, NULL, 0, NULL);
	WideCharToMultiByte(CP_OEMCP, NULL, tmep, -1, g_filenameonly, dwNum, 0, NULL);

	if (strlen(g_filenameonly) > 0)
		strcat(g_filenameonly, "_FileData.txt");
	else
	{
		strcpy(g_filenameonly, "FileData.txt");
		AfxMessageBox(L"转换文件名有问题");
	}
	obj_Peinfo.puOpenFileLoad(m_MasterStaticTextStr);

	// 2. 压缩全部区段 压缩的时候不清空数据目录表以及区段大小（不压缩新增区段）
	CompressionData obj_ComperData;

	CloseHandle(obj_Peinfo.puFileHandle()); UpdateData(TRUE);

	obj_Peinfo.puOpenFileLoad(m_MasterStaticTextStr);


	if (!obj_ComperData.puCompressSection()) {
		AfxMessageBox(L"CompressSection failuer!");
		return;
	}


	CloseHandle(obj_Peinfo.puFileHandle());

	m_MasterStaticTextStr = "C:\\Users\\CompressionMask.exe";

	obj_Peinfo.puOpenFileLoad(m_MasterStaticTextStr);

	// 3. Stud壳数据拷贝/操作
	studData obj_stuData;

	obj_stuData.puLoadLibraryStud();

	obj_stuData.puRepairReloCationStud();

	if (obj_stuData.puCopyStud())
	{
		CloseHandle(obj_Peinfo.puFileHandle());
		UpdateData(TRUE);
		// 这些操作只为了最后保留一个有效加壳文件，其实垃圾exe清理
		int nstatus = CopyFile(L"C:\\Users\\CompressionMask.exe", nStr, FALSE);
		if (nstatus)
		{
			m_MasterStaticTextStr = nStr;
			DeleteFile(L"C:\\Users\\CompressionMask.exe");
		}
		obj_Peinfo.puOpenFileLoad(m_MasterStaticTextStr);
		AfxMessageBox(m_MasterStaticTextStr + L"   Success!");
	}
	else
		AfxMessageBox(L"StudWrite failure!");

	// 4、收尾工作
	CloseHandle(obj_Peinfo.puFileHandle());
}

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
	ShowPEInfoData(m_MasterStaticTextStr);
	DragFinish(hDropInfo);
	CDialogEx::OnDropFiles(hDropInfo);
}

void MasterWindows::ShowPEInfoData(const CString & FileName)
{
	PuPEInfo obj_puPe; CString Tempstr;	DWORD TempdwCode = 0;

	if (!obj_puPe.puOpenFileLoad(FileName))
		return;

	PIMAGE_NT_HEADERS pNtHeadre = (PIMAGE_NT_HEADERS)obj_puPe.puGetNtHeadre();

	PIMAGE_FILE_HEADER pFileHeadre = (PIMAGE_FILE_HEADER)&pNtHeadre->FileHeader;

	PIMAGE_OPTIONAL_HEADER pOption = (PIMAGE_OPTIONAL_HEADER)&pNtHeadre->OptionalHeader;

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

}

// PE_View
void MasterWindows::OnBnClickedButton4()
{
	UpdateData(TRUE);
	if (m_MasterStaticTextStr.IsEmpty())
	{
		AfxMessageBox(L"请先拖入文件");
		return;
	}
	SectionInfo obj_section;
	obj_section.DoModal();
	return;
}

// NewSection
void MasterWindows::OnBnClickedButton9()
{
	UpdateData(TRUE);
	if (m_MasterStaticTextStr.IsEmpty())
	{
		AfxMessageBox(L"请先拖入文件");
		return;
	}
	if (NewSection())
		AfxMessageBox(L"Success");
	else
		AfxMessageBox(L"Fauilter");
}

BOOL MasterWindows::NewSection()
{
	BOOL nRet = TRUE;
	BYTE Name[] = ".VMP";
	AddSection obj_addsection; 

	// 添加区段提前申请Stub大小
	DWORD SectionSize = 0;
	std::string sDriectory = "";
	CodeTool::CGetCurrentDirectory(sDriectory);
	if (!sDriectory.empty()) {
		const std::string sStuFile = (sDriectory + "Stud.dll").c_str();
		const HANDLE hFile = CreateFileA(sStuFile.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile) {
			SectionSize = GetFileSize(hFile, &SectionSize);
			CloseHandle(hFile);
		}
		if (SectionSize <= 0)
			return false;
	}
	else
		return false;
	obj_addsection.puModifySectioNumber();
	nRet = obj_addsection.puModifySectionInfo(Name, SectionSize);
	obj_addsection.puModifyProgramEntryPoint();
	obj_addsection.puModifySizeofImage();
	nRet = obj_addsection.puAddNewSectionByData(SectionSize);
	return nRet;
}

// Only_ComperData
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
	
	if (!obj_ComperData.puCompressSection())
		AfxMessageBox(L"CompressSection failuer!");
	else
		AfxMessageBox(L"CompressSection Seucess!");
}

// 一键脱壳
void MasterWindows::OnBnClickedButton2()
{
	UpdateData(TRUE);
	if (m_MasterStaticTextStr.IsEmpty())
	{
		AfxMessageBox(L"请先拖入文件");
		return;
	}

	// 判断是否我们的壳，否则不给脱壳
	UnShllerProcPath = m_MasterStaticTextStr;

	UnShell obj_Unshell;
	if (!obj_Unshell.puRepCompressionData()) {
		AfxMessageBox(L"puRepCompressionData error.");
		return;
	}
	if(!obj_Unshell.puDeleteSectionInfo()) {
		AfxMessageBox(L"puDeleteSectionInfo error.");
		return;
	}

	if (obj_Unshell.puSaveUnShell())
	{
		// 只保留脱壳后原始文件，命名一致性
		int nRet = CopyFile(L"C:\\Users\\Administrator\\Desktop\\UnShellNewPro.exe", m_MasterStaticTextStr, FALSE);
		if (nRet)
		{
			DeleteFile(L"C:\\Users\\Administrator\\Desktop\\UnShellNewPro.exe");
		}
		AfxMessageBox(L"puSaveUnShell_Success");
	}
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
	CRect rect = { 0 };
	GetClientRect(&rect);
	BITMAP bm;
	m_bmp.GetBitmap(&bm);
	dc.StretchBlt(0, 0, rect.Width(), rect.Height(),
		&m_dc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

	CDialogEx::OnPaint();
}
