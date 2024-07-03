#include "stdafx.h"
#include "CompressionData.h"
#include "vm.h"
#include "AddSection.h"
#include "puPEinfoData.h"
#include "CombatShell/CombatShell.h"
#include "lz4/include/lz4.h"
#include "quick/quicklz.h"
#include <io.h>

#define NEWSECITONNAME ".VMP"

FILE* fpVmFile = NULL;
// Stub export
_Stud* g_stu = nullptr;
_VmNode* g_Vm = nullptr;
char* g_dataHlpers = nullptr;
DWORD64 g_dataoffset = 0;
extern char g_filenameonly[MAX_PATH];

CompressionData::CompressionData()
{
}

CompressionData::~CompressionData()
{
	if (m_lpBase) {
		free(m_lpBase);
		m_lpBase = nullptr;
	}
	if (m_hFile) {
		CloseHandle(m_hFile);
		m_hFile = nullptr;
	}
}

VOID CompressionData::ReFileInit()
{
	if (m_lpBase) {
		free(m_lpBase);
		m_lpBase = nullptr;
	}
	if (m_hFile) {
		CloseHandle(m_hFile);
		m_hFile = nullptr;
	}
	PuPEInfo obj_peInfo;
	obj_peInfo.puOpenFileLoad(m_MasterStaticTextStr);
	m_lpBase = obj_peInfo.puGetImageBase();
	m_SectionHeadre = obj_peInfo.puGetSection();
	m_SectionCount = ((PIMAGE_NT_HEADERS)(obj_peInfo.puGetNtHeadre()))->FileHeader.NumberOfSections;
	m_hFile = obj_peInfo.puFileHandle();
	m_hFileSize = obj_peInfo.puFileSize();
}

// 压缩区段之前 Vmencode
void CompressionData::VmcodeEntry(char* TargetCode, _Out_ int &CodeLength)
{
	// 这里写入需要加密多少次,或者代码段Asm
	int vm_len = 1;
	// write: 0. 写入一共VM加密多少代码段
	g_Vm->VmCount = vm_len;
	// fwrite(&vm_len, sizeof(int), 1, fpVmFile);

	DWORD64 Offset = 0;
	// 获取VM的起始地址
	DWORD64 Vmencodeaddr = (DWORD64)GetProcAddress((HMODULE)m_studBase, "CombatShellEntry");
	if (!Vmencodeaddr)
		return;
	PuPEInfo obj_pePe;
	PIMAGE_SECTION_HEADER studSection = obj_pePe.puGetSectionAddress((char *)m_studBase, (BYTE *)".text");
	PIMAGE_SECTION_HEADER SurceBase = obj_pePe.puGetSectionAddress((char *)m_lpBase, (BYTE *)NEWSECITONNAME);
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)obj_pePe.puGetNtHeadre();
	if (!studSection || (!SurceBase) || (!pNt))
		return;

#ifdef _WIN64
	Offset = (DWORD64)Vmencodeaddr - (DWORD64)m_studBase - studSection->VirtualAddress + SurceBase->VirtualAddress;
	// 计算保存VM数据偏移
	g_dataoffset = (DWORD64)g_dataHlpers - (DWORD64)m_studBase - studSection->VirtualAddress + SurceBase->VirtualAddress;
#else
	Offset = (DWORD)Vmencodeaddr - (DWORD)m_studBase - studSection->VirtualAddress + SurceBase->VirtualAddress;
#endif
	/*
		线性反汇编来求大小
	*/
	// write: 1. offset -- 汇编指令
	g_Vm->VmAddroffset = Offset;
	// fwrite(&Offset, sizeof(DWORD64), 1, fpVmFile);
	vm_len = 82;		// 固定的需要人工去看反汇编多少行,ida中看一下,不智能
	g_Vm->Vmencodeasmlen = vm_len;
	// fwrite(&vm_len, sizeof(int), 1, fpVmFile);
	// fflush(fpVmFile);
	VM vmobj;
	vmobj.VmEntry((PVOID64)Vmencodeaddr, vm_len);
}

// 添加一个区段给压缩后的数据使用
void CompressionData::AddCompreDataSection(const DWORD & size)
{
	BYTE Name[] = ".UPX";
	DWORD Compresdata = size;

	AddSection obj_addSection;
	obj_addSection.puInti(m_MasterStaticTextStr);
	obj_addSection.puModifySectioNumber();
	obj_addSection.puModifySectionInfo(Name, Compresdata);
	obj_addSection.puModifySizeofImage();
	obj_addSection.puAddNewSectionByData(Compresdata);
}

BOOL CompressionData::EncryptionSectionData(
	char* src,
	int srclen,
	char enkey)
{
	for (int i = 0; i < srclen; ++i)
	{
		*src ^= enkey;
		src++;
	}
	return 1;
}

// 压缩PE区段数据
BOOL CompressionData::CompressSectionData()
{
	CString csTmep;
	std::string sTagetDirectory = "";
	std::wstring wsTagetDirectory = L"";
	{
		csTmep = m_MasterStaticTextStr;
		int n = csTmep.ReverseFind('\\') + 1;
		int m = csTmep.GetLength() - n;
		wsTagetDirectory = csTmep.Left(n);
		csTmep = csTmep.Right(m);
	}
	if (strlen(g_filenameonly) <= 0) {
		RtlSecureZeroMemory(g_filenameonly, 0);
		DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, csTmep, -1, NULL, NULL, 0, NULL);
		WideCharToMultiByte(CP_OEMCP, NULL, csTmep, -1, g_filenameonly, dwNum, 0, NULL);
		if (strlen(g_filenameonly) > 0)
			strcat(g_filenameonly, "_FileData.txt");
		else
		{
			strcpy(g_filenameonly, "FileData.txt");
			AfxMessageBox(L"转换文件名有问题");
		}
	}
	sTagetDirectory = (CodeTool::wstring2string(wsTagetDirectory) + g_filenameonly).c_str();

	std::string sDriectory = "";
	std::string sCombatShellPath = "";
	CodeTool::CGetCurrentDirectory(sDriectory);
	if (!sDriectory.empty()) {
		sCombatShellPath = (sDriectory + "CombatShell.dll").c_str();
	}
	if (sCombatShellPath.empty())
		sCombatShellPath = "CombatShell.dll";
	std::wstring wsCombatShellPath = CodeTool::string2wstring(sCombatShellPath.c_str()).c_str();
	if (_access(sCombatShellPath.c_str(), 0) != 0) {
		AfxMessageBox((L"CombatShell文件缺失. " + wsCombatShellPath).c_str());
		return 0;
	}

#ifdef _WIN64
	m_studBase = LoadLibraryEx(wsCombatShellPath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);
#else
	m_studBase = LoadLibraryEx(wsCombatShellPath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);
#endif
	if (!m_studBase || (nullptr == m_studBase)) {
		AfxMessageBox((L"CombatShell LoadLibraryEx Error. " + wsCombatShellPath).c_str());
		return false;
	}

	g_stu = (_Stud*)GetProcAddress((HMODULE)m_studBase, "g_stud");
	g_Vm = (VmNode*)GetProcAddress((HMODULE)m_studBase, "g_VmNode");
	g_dataHlpers = (char *)GetProcAddress((HMODULE)m_studBase, "g_dataHlper");// g_dataHlper
	if (!g_stu || (!g_Vm) || (!g_dataHlpers)) {
		AfxMessageBox(L"Stud.dll GetProcAddress 失败.");
		return 0;
	}
	g_stu->s_OneSectionSizeofData = FALSE;

#ifdef _WIN64
	int nLen = 0;

	// 压缩前后都可以,因为只对壳代码VM
	VmcodeEntry(NULL, nLen);

	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(((PIMAGE_DOS_HEADER)m_lpBase)->e_lfanew + (DWORD64)m_lpBase);
#else
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(((PIMAGE_DOS_HEADER)m_lpBase)->e_lfanew + (DWORD)m_lpBase);
#endif

	DWORD dSectionCount = pNt->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER psection = (PIMAGE_SECTION_HEADER)m_SectionHeadre;

	PuPEInfo obj_peInfo;
	m_maskAddress = obj_peInfo.puGetSectionAddress((char *)m_lpBase, (BYTE *)NEWSECITONNAME);
	if (!m_maskAddress) {
		AfxMessageBox(L".VMP 区别识别失败!\n");
		return false;
	}

	// 避免如.textbss无数据
	for (DWORD i = 0; i < dSectionCount; ++i)
	{
		if (psection->PointerToRawData != 0)
			break;
		++psection;
	}

	// pe标准大小对齐后（加载基址 + .text->pointertorawdata的数据）= 大小
	DWORD pStandardHeadersize = psection->PointerToRawData;

	char* SaveCompressData = (char*)malloc(m_hFileSize);
	if (!SaveCompressData)
		return false;
	memset(SaveCompressData, 0, m_hFileSize);

	PIMAGE_SECTION_HEADER pSections = (PIMAGE_SECTION_HEADER)m_SectionHeadre;
	DWORD ComressTotalSize = 0;
	// 注意修复-改为程序Name_FileData.txt
	if ((fpFile = fopen(sTagetDirectory.c_str(), "wb+")) == NULL)
	{
		AfxMessageBox(L"CombatShell 打开创建失败.");
		return false;
	}
	// 不压缩新增的区段（加壳区段）
	for (DWORD i = 0; i < dSectionCount - 2; ++i)
	{

		DWORD DataSize = pSections->SizeOfRawData;

		if (pSections->SizeOfRawData == 0)
		{
			fwrite(&pSections->SizeOfRawData, sizeof(DWORD), 1, fpFile);
			fflush(fpFile);
			++pSections;
			g_stu->s_OneSectionSizeofData = TRUE;
			continue;
		}

		void* DataAddress = (void *)(pSections->PointerToRawData + (DWORD64)m_lpBase);

		char* buf = NULL;
#ifdef _WIN64
//------------------------------------------------------------------------------------------

		qlz_state_compress *state_compress = (qlz_state_compress *)malloc(sizeof(qlz_state_compress));

		// 计算安全缓冲区
		//const int blen = LZ4_compressBound(pSections->SizeOfRawData + 1);

		const int blen = pSections->SizeOfRawData + 400;

		// 安全空间申请
		if ((buf = (char*)malloc(sizeof(char) * blen)) == NULL)
		{
			AfxMessageBox(L"no enough memory!\n");
			return -1;
		}

		/* 压缩 */
		// const int dwCompressionSize = LZ4_compress_default((char*)DataAddress, buf, pSections->SizeOfRawData, blen);
		const int dwCompressionSize = qlz_compress((char*)DataAddress, buf, blen, state_compress);

//------------------------------------------------------------------------------------------
#else 
		DWORD blen;

		// 计算安全缓冲区
		blen = LZ4_compressBound(pSections->SizeOfRawData);

		// 安全空间申请
		if ((buf = (char*)malloc(sizeof(char) * blen)) == NULL)
		{
			AfxMessageBox(L"no enough memory!\n");
			return -1;
		}

		DWORD dwCompressionSize = 0;

		/* 压缩 */
		dwCompressionSize = LZ4_compress_default((char*)DataAddress, buf, pSections->SizeOfRawData, blen);

#endif
//------------------------------------------------------------------------------------------
		fwrite(&dwCompressionSize, sizeof(DWORD), 1, fpFile);

		fflush(fpFile);

		// 区段异或加密
		// EncryptionSectionData(buf, dwCompressionSize, 'B');

		// 计算缓区去后大小
		memcpy(&g_stu->s_blen[i], &dwCompressionSize, sizeof(DWORD));

		// 保存压缩后区段数据（拼接每一个压缩区段）
		memcpy(&SaveCompressData[ComressTotalSize], buf, dwCompressionSize);

		// 保存压缩后总大小
		ComressTotalSize += dwCompressionSize;

		free(buf);

		++pSections;
	}
	if (fpFile)
		fclose(fpFile);

	// 0x400 + (压缩后的大小 / 0x200 + ----压缩后的大小 % 0x200 ? 1 : 0) 0x200;
	// 对齐数据
	DWORD Size = 0;
	if (ComressTotalSize % 0x200 == 0)
	{
		Size = pStandardHeadersize + ((ComressTotalSize / 0x200) * 0x200);
		int a = 10;
	}
	else
	{
		Size = pStandardHeadersize + (((ComressTotalSize / 0x200) + 1) * 0x200);
		int a = 10;
	}

	DWORD ModifySize = Size - 0x400;

	// 创建一个新区段
	AddCompreDataSection(ModifySize);

	// 重新加载个中PE数据保证下面数据获取最新
	PuPEInfo obj_Peinfo;
	CloseHandle(obj_Peinfo.puFileHandle());
	FileName = obj_peInfo.puFilePath();
	obj_Peinfo.puOpenFileLoad(FileName);
	CompressionData obj_Compre;
	// 修改新区段的信息数据 文件偏移 0x400  大小 压缩后数据对齐大小
	BYTE Name[] = ".UPX";

	obj_peInfo.puSetFileoffsetAndFileSize(m_lpBase, 0x400, ModifySize, Name);

	BYTE Nmase[] = ".UPX";

	PIMAGE_SECTION_HEADER compSectionAddress = obj_peInfo.puGetSectionAddress((char*)m_lpBase, Nmase);

	// 保存内存地址 用于解压基址
	g_stu->s_CompressionSectionRva = compSectionAddress->VirtualAddress;

	// 拷贝压缩后的数据(对齐) --> 新加的区段
#ifdef  _WIN64
	memcpy((PVOID64)(compSectionAddress->PointerToRawData + (DWORD64)m_lpBase), SaveCompressData, ModifySize);
#else
	memcpy((void*)(compSectionAddress->PointerToRawData + (DWORD)m_lpBase), SaveCompressData, ModifySize);
#endif //  _WIN64
	// 拼接标准PE头 + 压缩数据的区段 + 自己的区段
	char* ComressNewBase = (char*)malloc(Size + m_maskAddress->SizeOfRawData);

	memset(ComressNewBase, 0, (Size + m_maskAddress->SizeOfRawData));

	// 拼接标准PE
	memcpy(ComressNewBase, m_lpBase, pStandardHeadersize);

	
#ifdef _WIN64
	// 拼接压缩后的全部区段(第一个头信息)
	memcpy(&ComressNewBase[pStandardHeadersize], (PVOID64)(compSectionAddress->PointerToRawData + (DWORD64)m_lpBase), ComressTotalSize);
	memcpy(&ComressNewBase[Size], (PVOID64)(m_maskAddress->PointerToRawData + (DWORD64)m_lpBase), m_maskAddress->SizeOfRawData);
#else
	memcpy(&ComressNewBase[pStandardHeadersize], (void*)(compSectionAddress->PointerToRawData + (DWORD)m_lpBase), ComressTotalSize);
	// 拼接加壳区段数据
	memcpy(&ComressNewBase[Size], (void *)(m_maskAddress->PointerToRawData + (DWORD)m_lpBase), m_maskAddress->SizeOfRawData);
#endif // _WIN64

	DWORD dwWrite = 0;	OVERLAPPED OverLapped = { 0 };

	// 清空数据目录表(收尾工作)
	CleanDirectData(ComressNewBase, ComressTotalSize, Size);

	// Create File
	std::wstring wsMaskCompre = (wsTagetDirectory + L"CompressionMask.exe").c_str();
	HANDLE HandComprele = CreateFile(wsMaskCompre.c_str(), GENERIC_READ | GENERIC_WRITE, FALSE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	// Write Exe 完成压缩
	int nRet = WriteFile(HandComprele, ComressNewBase, (Size + m_maskAddress->SizeOfRawData), &dwWrite, &OverLapped);
	CloseHandle(HandComprele);
	if (!nRet)
		AfxMessageBox(L"CompressWriteFile failuer");
	return TRUE;
}

// 判断真正的区段数据大小（未对齐）
DWORD CompressionData::IsSectionSize(DWORD MiscVirtualsize, DWORD sizeOfRawData)
{
	if (MiscVirtualsize > sizeOfRawData)
		return sizeOfRawData;
	if (MiscVirtualsize < sizeOfRawData)
		return MiscVirtualsize;
	if (MiscVirtualsize == sizeOfRawData)
		return sizeOfRawData;
	return 0;
}

// 清空数据目录等数据
BOOL CompressionData::CleanDirectData(const char* NewAddress, const DWORD & CompresSize, const DWORD & Size)
{
	if ((fpFile = fopen(g_filenameonly, "ab+")) == NULL)
	{
		AfxMessageBox(L"文件打开失败");
		return false;
	}
#ifdef _WIN64
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(((PIMAGE_DOS_HEADER)NewAddress)->e_lfanew + (DWORD64)NewAddress);
#else
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(((PIMAGE_DOS_HEADER)NewAddress)->e_lfanew + (DWORD)NewAddress);
#endif

	PIMAGE_DATA_DIRECTORY pDirectory = (PIMAGE_DATA_DIRECTORY)pNt->OptionalHeader.DataDirectory;

	DWORD dwSectionCount = pNt->FileHeader.NumberOfSections;

	g_stu->s_SectionCount = dwSectionCount;

	int k = 0;
	// 保存\清空数据目录表
	for (DWORD i = 0; i < 16; ++i)
	{
		memcpy(&g_stu->s_DataDirectory[i][0], &pDirectory->VirtualAddress, sizeof(DWORD));
		memcpy(&g_stu->s_DataDirectory[i][1], &pDirectory->Size, sizeof(DWORD));
		//fprintf(fpFile, "%x %x", pDirectory->VirtualAddress, pDirectory->Size);
		fwrite(&pDirectory->VirtualAddress, sizeof(DWORD), 1, fpFile);
		fwrite(&pDirectory->Size, sizeof(DWORD), 1, fpFile);
		fflush(fpFile);
		pDirectory->VirtualAddress = 0;
		pDirectory->Size = 0;
		++pDirectory;
	}

	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);

	// 保存\清空区段文件大小及文件偏移
	for (DWORD i = 0; i < dwSectionCount - 2; ++i)
	{
		memcpy(&g_stu->s_SectionOffsetAndSize[i][0], &pSection->SizeOfRawData, sizeof(DWORD));
		memcpy(&g_stu->s_SectionOffsetAndSize[i][1], &pSection->PointerToRawData, sizeof(DWORD));
		//fprintf(fpFile, "% %04x", pSection->SizeOfRawData, pSection->PointerToRawData);
		fwrite(&pSection->SizeOfRawData, sizeof(DWORD), 1, fpFile);
		fwrite(&pSection->PointerToRawData, sizeof(DWORD), 1, fpFile);
		fflush(fpFile);
		pSection->SizeOfRawData = 0;
		pSection->PointerToRawData = 0;
		++pSection;
	}

	// 最后一个区段是壳区段 信息不变 修改文件偏移
	// 改变文件偏移对齐后文件偏移的地方
	pSection->PointerToRawData = Size;

	fclose(fpFile);

	return 0;
}