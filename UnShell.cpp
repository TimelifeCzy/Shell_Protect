#include "UnShell.h"
#include "./Stud/Stud.h"
#include <malloc.h>
#include "lz4/include/lz4.h"
#include "quick/quicklz.h"
#include "puPEinfoData.h"

extern _Stud* g_stu;
extern CString UnShllerProcPath;
extern char g_filenameonly[MAX_PATH];

#define NEWSECITONNAME ".VMP"

UnShell::UnShell()
{
	PuPEInfo obj_puPe;

	HANDLE tempHandle = obj_puPe.puFileHandle();

	CloseHandle(tempHandle);
	
	if (!UnShllerProcPath.IsEmpty())
		hFile = CreateFile(UnShllerProcPath, GENERIC_READ | GENERIC_WRITE, FALSE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	else
	{
		AfxMessageBox(L"UnShellerProPath Empty faliuer");
		return;
	}


	CString tmep = UnShllerProcPath;
	int n = tmep.ReverseFind('\\') + 1;
	int m = tmep.GetLength() - n;
	tmep = tmep.Right(m);
	DWORD dwNum = 0;
	dwNum = WideCharToMultiByte(CP_OEMCP, NULL, tmep, -1, NULL, NULL, 0, NULL);
	WideCharToMultiByte(CP_OEMCP, NULL, tmep, -1, g_filenameonly, dwNum, 0, NULL);
	if (g_filenameonly)
		strcat(g_filenameonly, "_FileData.txt");
	else
	{
		strcpy(g_filenameonly, "FileData.txt");
		AfxMessageBox(L"转换文件名有问题");
	}


	DWORD dwSize = GetFileSize(hFile, NULL);
	m_Base = (void *)malloc(dwSize);
	memset(m_Base, 0, dwSize);
	DWORD dwRead = 0;
	OVERLAPPED OverLapped = { 0 };
	int nRetCode = ReadFile(hFile, m_Base, dwSize, &dwRead, &OverLapped);
	if (hFile)
		CloseHandle(hFile);

	pDosHander = (PIMAGE_DOS_HEADER)m_Base;

	pHeadres = (PIMAGE_NT_HEADERS)(pDosHander->e_lfanew + (DWORD64)m_Base);

	pSection = IMAGE_FIRST_SECTION(pHeadres);

	m_NtAddress = (void*)pHeadres;

	if ((fpFile = fopen(g_filenameonly, "rb+")) == NULL)
	{
		AfxMessageBox(L"open file failure");
	}
}

UnShell::~UnShell()
{
	fclose(fpFile);
}

BOOL UnShell::RepCompressionData()
{
	// E:\Sheller-master\Release\Stud.dll
#ifdef _WIN64
	m_studBase = LoadLibraryEx(L"Stud.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
#else
	m_studBase = LoadLibraryEx(L"Stud.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
#endif
	g_stu = (_Stud*)GetProcAddress((HMODULE)m_studBase, "g_stud");
	if (!m_studBase || (!g_stu)) {
		return false;
	}

	DWORD SectionCount = pHeadres->FileHeader.NumberOfSections;

	for (DWORD i = 0; i < SectionCount - 3; ++i)
	{
		fread(&g_stu->s_blen[i], sizeof(DWORD), 1, fpFile);
	}


	for (DWORD i = 0; i < 16; ++i)
	{
		fread(&g_stu->s_DataDirectory[i][0], sizeof(DWORD), 1, fpFile);
		fread(&g_stu->s_DataDirectory[i][1], sizeof(DWORD), 1, fpFile);
		// fscanf(fpFile, "%04x %04x", &g_stu->s_DataDirectory[i][0], &g_stu->s_DataDirectory[i][1]);
	}

	for (DWORD i = 0; i < SectionCount - 2; ++i)
	{

		fread(&g_stu->s_SectionOffsetAndSize[i][0], sizeof(DWORD), 1, fpFile);
		fread(&g_stu->s_SectionOffsetAndSize[i][1], sizeof(DWORD), 1, fpFile);
		// fscanf(fpFile, "%04x %04x", &g_stu->s_SectionOffsetAndSize[i][0], &g_stu->s_SectionOffsetAndSize[i][1]);
	}

#ifdef _WIN64
	fread(&g_stu->s_dwOepBase, sizeof(DWORD64), 1, fpFile);
#else
	fread(&g_stu->s_dwOepBase, sizeof(DWORD), 1, fpFile);
#endif

	/*========================================================================*/

	TotaldwSize = 0;

	for (DWORD i = 0; i < SectionCount - 2; ++i)
	{
		TotaldwSize += g_stu->s_SectionOffsetAndSize[i][0];
	}


	Sectionbuf = (char*)malloc(TotaldwSize);

	DWORD DataStart = 0x400;

	DWORD Flag = 0;

	PuPEInfo obj_pePE;

	BYTE Name[] = ".UPX";

	PIMAGE_SECTION_HEADER address = (PIMAGE_SECTION_HEADER)obj_pePE.puGetSectionAddress((char*)m_Base, Name);
	if (!address) {
		return false;
	}

	int nFlag = 0;
	DWORD Address = address->PointerToRawData;
	for (DWORD i = 0; i < SectionCount - 2; ++i)
	{
		if (g_stu->s_blen[nFlag] == 0)
		{
			nFlag += 1;
			continue;
		}
#ifdef _WIN64
		qlz_state_decompress *state_decompress = (qlz_state_decompress *)VirtualAlloc(NULL, sizeof(qlz_state_decompress), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		int nRet = qlz_decompress((char*)(Address + (DWORD64)m_Base), &Sectionbuf[Flag], state_decompress);
#else
		// 缓冲区  RVA+加载基址  缓冲区大小  压缩过去的大小
		int nRet = LZ4_decompress_safe((char*)(Address + (DWORD)m_Base), &Sectionbuf[Flag], g_stu->s_blen[nFlag], g_stu->s_SectionOffsetAndSize[i][0]);
#endif
		Address += g_stu->s_blen[i];
		Flag += g_stu->s_SectionOffsetAndSize[i][0];
		nFlag++;
	}

	return TRUE;
}

BOOL UnShell::DeleteSectionInfo()
{
	DWORD dwSectionCount = pHeadres->FileHeader.NumberOfSections;

	PIMAGE_DATA_DIRECTORY pDataDirectory = (PIMAGE_DATA_DIRECTORY)pHeadres->OptionalHeader.DataDirectory;

	for (DWORD i = 0; i < 16; ++i)
	{
		if (0 != g_stu->s_DataDirectory[i][0])
			pDataDirectory->VirtualAddress = g_stu->s_DataDirectory[i][0];
		if (0 != g_stu->s_DataDirectory[i][1])
			pDataDirectory->Size = g_stu->s_DataDirectory[i][1];
		++pDataDirectory;
	}

	for (DWORD i = 0; i < dwSectionCount - 2; ++i)
	{
		if (0 != g_stu->s_SectionOffsetAndSize[i][0])
			pSection->SizeOfRawData = g_stu->s_SectionOffsetAndSize[i][0];
		if (0 != g_stu->s_SectionOffsetAndSize[i][1])
			pSection->PointerToRawData = g_stu->s_SectionOffsetAndSize[i][1];
		++pSection;
	}

	pHeadres->FileHeader.NumberOfSections -= 2;

	PIMAGE_SECTION_HEADER pSection_s = IMAGE_FIRST_SECTION(pHeadres);

	DWORD NewdwSectionOfSize = (dwSectionCount - 2) * 0x28;

	char* temp = (char*)malloc(80);

	memset(temp, 0, 80);

	DWORD old = 0;
	BYTE Name[] = ".VMP";
	BYTE Name1[] = ".UPX";
	PuPEInfo pePu;

	DWORD64 masAdd = (DWORD64)pePu.puGetSectionAddress((char*)m_Base, Name);
	if (!masAdd)
		return false;
	VirtualProtect((char*)masAdd, 40, PAGE_READWRITE, &old);
	memcpy((char*)masAdd, temp, 40);
	VirtualProtect((char*)masAdd, 40, old, &old);

	DWORD64 comAdd = (DWORD64)pePu.puGetSectionAddress((char*)m_Base, Name1);
	if (!comAdd)
		return false;
	VirtualProtect((char*)comAdd, 40, PAGE_READWRITE, &old);
	memcpy((char*)comAdd, temp, 40);
	VirtualProtect((char*)comAdd, 40, old, &old);

	free(temp);

	temp = nullptr;

	--pSection;

	pHeadres->OptionalHeader.SizeOfImage = pSection->VirtualAddress + pSection->SizeOfRawData;

	pHeadres->OptionalHeader.AddressOfEntryPoint = g_stu->s_dwOepBase;
	
	return TRUE;
}

BOOL UnShell::SaveUnShell()
{
	DWORD Size = 0x400 + TotaldwSize;

	UnShellNewFile = (char*)malloc(Size);

	memcpy(UnShellNewFile, m_Base, 0x400);

	memcpy(&UnShellNewFile[0x400], Sectionbuf, TotaldwSize);

	DWORD dwWrite = 0; 
	OVERLAPPED OverLapped = { 0, };


	HANDLE Handle = CreateFile(L"C:\\Users\\Administrator\\Desktop\\UnShellNewPro.exe", GENERIC_READ | GENERIC_WRITE, FALSE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	int nRet = WriteFile(Handle, UnShellNewFile, Size, &dwWrite, NULL);

	CloseHandle(Handle);

	if (!nRet)
		return FALSE;
	return TRUE;
}