#pragma once
#ifndef STUD_H_
#define STUD_H_

#ifdef _WIN64
	typedef struct _Stud
	{
		DWORD64 s_dwOepBase;
		DWORD64 s_Krenel32;
		DWORD64 s_msvcr100;
		DWORD64 s_User32;
		DWORD64 s_Gdi32;
		DWORD64 s_MSVCRT;
		HMODULE s_ModuleUser32;
		DWORD64 s_DirectoryCount;
		DWORD64 s_SectionCount;
		DWORD64 s_DataDirectory[16][2];
		DWORD s_SectionOffsetAndSize[20][2];
		DWORD s_blen[20];
		BOOL s_OneSectionSizeofData;
		DWORD64 s_CompressionSectionRva;
		DWORD64 s_SaveExportTabRVA;
	}Stud;
#else
	//  /NODEFAULTLIB:LIBCMT.lib 
	typedef struct _Stud
	{
		DWORD s_dwOepBase;
		DWORD s_Krenel32;
		DWORD s_User32;
		DWORD s_Gdi32;
		DWORD s_MSVCRT;
		DWORD s_DirectoryCount;
		DWORD s_SectionCount;
		DWORD s_DataDirectory[16][2];
		DWORD s_SectionOffsetAndSize[20][2];
		DWORD s_blen[20];
		BOOL s_OneSectionSizeofData;
		DWORD s_CompressionSectionRva;
		DWORD s_SaveExportTabRVA;
	}Stud;
#endif

typedef struct _ArrayHlerp
{
	unsigned int bytesize;			// 字节码大小
	unsigned short xorKey;			// 异或密钥
	unsigned int encodeflag;		// 是否被vmcode加密
	unsigned int startoffset;		// 距离当前加密VM代码段起始地址的偏移
	char mnemonic[32];				// 记录汇编指令
}ArrayHlerp;

typedef struct _VmNode
{
	unsigned int VmCount;			// 加密了多少代码段
	unsigned int VmAddroffset;		// Vmcode_Startaddrs
	unsigned int Vmencodeasmlen;	// VmCodeLine
	unsigned int Hlperdataoffset;	// HlperDataoffset
	ArrayHlerp* data;				// data
}VmNode, *PVmNode;

// 保存handler运行后属于当前代码段的寄存器状态
// 每个代码段应该有他自己的寄存器状态,进入hadnler之后还原才可以
typedef struct _x86regeditNode
{
	unsigned __int64 rax;
	unsigned __int64 rbx;
	unsigned __int64 rcx;		// struct.offset = 16
	unsigned __int64 rdx;		// struct.offset = 24
	unsigned __int64 rdi;
	unsigned __int64 rsi;
	unsigned __int64 rsp;		// struct.offset = 48
	unsigned __int64 rbp;
	unsigned __int64 r8;		// struct.offset = 64
	unsigned __int64 r9;		// struct.offset = 72
	unsigned __int64 r10;
	unsigned __int64 r11;
	unsigned __int64 r12;
	unsigned __int64 r13;
	unsigned __int64 r14;
	unsigned __int64 r15;
}x86regeditNode;

// Capstone Ptr
typedef struct _PRT
{
	PVOID64 free;
	PVOID64 calloc;
	PVOID64 malloc;
	PVOID64 realloc;
	PVOID64 vsprintf_s;
}PRT;

#endif

/*
		Function Hash Values:
			0x228C4218			KERNEL32.DLL - 64bit
			0xEC1C6278;			kernel32.dll
			0xC0D83287;			LoadlibraryExa
			0x4FD18963;			ExitPorcess
			0x5644673D			User32.dll
			0x1E380A6A			MessageBoxA
			0x9EBC86B			RtlExitUserProcess
			0xF4E2F2C8			GetModuleHandleW
			0xBB7420F9			CreateSolidBrush
			0xBC05E48			RegisterClassW
			0x1FDAF571			CreateWindowExW
			0xDD8B5FB8			ShowWindow
			0x9BB5D8DC			UpdateWindow
			0x61060461			GetMessageW
			0xE09980A2			TranslateMessage
			0x7A1506D8			DispatchMessageW
			0x457BF55A			GetWindowTextW
			0x7EAD1F86			lstrcmpW
			0x1E380A6A			MessageBoxA
			0xCAA94781			PostQuitMessage
			0x22E85CBA			DefWindowProcW
			0xC6B20165			LoadCursorW
			0x7636E8F4			LoadIconW
			0x1FDAF55B			CreateWindowA
			0x68D82F59			RegisterClassExW
			0x5D0CB479			GetDlgItem
			0x818F6ED7			Mymemcpy
			0x328CEB95			msvcrt.dll
			0x1EDE5967			VirtualAlloc
			0x2729F8BB			CreateThread		
			// ptr.malloc = puGetProcAddress(g_stud.s_User32, 0x7FB36681);
			// ptr.free = puGetProcAddress(g_stud.s_User32, 0xCBCB3065);
			// ptr.calloc = puGetProcAddress(g_stud.s_User32, 0x3FB36680);
			// ptr.realloc = puGetProcAddress(g_stud.s_User32, 0x9C336680);
			// ptr.vsprintf_s = puGetProcAddress(g_stud.s_User32, 0xFC541B4C);
			// Myfopen = (Fnfopen)puGetProcAddress(g_stud.s_User32, 0xCBC37ECE);
			// Myfread = (Fnfread)puGetProcAddress(g_stud.s_User32, 0xC39796C4);
			// Myfree = (Fnfree)puGetProcAddress(g_stud.s_User32, 0xCBCB3065);
			// Mymalloc = (Fnmalloc)puGetProcAddress(g_stud.s_User32, 0x7FB36681);
			// msvcr100.dll
			// puGetModule(0x228C4218, &g_stud.s_Krenel32);
			// MyLoadLibraryExA = (FnLoadLibraryExA)puGetProcAddress(g_stud.s_Krenel32, 0xC0D83287);
			// g_stud.msvcr = (DWORD64)MyLoadLibraryExA("msvcr100.dll", NULL, NULL);
*/

static TCHAR szWindowClass[] = TEXT("ZhuDongFangYu");

typedef void* (WINAPI* FnGetProcAddress)(HMODULE, const char*);
typedef HMODULE(WINAPI* FnLoadLibraryExA)(_In_ LPCSTR lpLibFileName, _Reserved_ HANDLE hFile, _In_ DWORD dwFlags);
typedef HMODULE(WINAPI* FnGetModuleHandleW)(_In_opt_ LPCWSTR lpModuleName);
typedef HBRUSH(WINAPI* FnCreateSolidBrush)(_In_ COLORREF color);
typedef ATOM(WINAPI* FnRegisterClassW)(_In_ CONST WNDCLASSW* lpWndClass);
typedef WINUSERAPI ATOM(WINAPI* FnRegisterClassExW)(_In_ CONST WNDCLASSEXW*);
typedef VOID* (WINAPIV* Fnmalloc)(_In_ _CRT_GUARDOVERFLOW size_t _Size);
typedef	WINUSERAPI HWND(WINAPI* FnCreateWindowExW)(
	_In_ DWORD dwExStyle,
	_In_opt_ LPCWSTR lpClassName,
	_In_opt_ LPCWSTR lpWindowName,
	_In_ DWORD dwStyle,
	_In_ int X,
	_In_ int Y,
	_In_ int nWidth,
	_In_ int nHeight,
	_In_opt_ HWND hWndParent,
	_In_opt_ HMENU hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID lpParam);
typedef BOOL(WINAPI* FnShowWindow)(_In_ HWND hWnd, _In_ int nCmdShow);
typedef BOOL(WINAPI* FnUpdateWindow)(_In_ HWND hWnd);
typedef BOOL(WINAPI* FnGetMessageW)(_Out_ LPMSG lpMsg, _In_opt_ HWND hWnd, _In_ UINT wMsgFilterMin, _In_ UINT wMsgFilterMax);
typedef BOOL(WINAPI* FnTranslateMessage)(_In_ CONST MSG* lpMsg);
typedef LRESULT(WINAPI* FnDispatchMessageW)(_In_ CONST MSG* lpMsg);
typedef int (WINAPI* FnGetWindowTextW)(_In_ HWND hWnd, _Out_writes_(nMaxCount) LPWSTR lpString, _In_ int nMaxCount);
typedef int (WINAPI* FnlstrcmpW)(_In_ LPCWSTR lpString1, _In_ LPCWSTR lpString2);
typedef int (WINAPI* FnMessageBoxA)(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType);
typedef VOID(WINAPI* FnPostQuitMessage)(_In_ int nExitCode);
typedef LRESULT(WINAPI* FnDefWindowProcW)(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);
typedef LRESULT(WINAPI* FnDefWindowProcA)(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);
typedef HCURSOR(WINAPI* FnLoadCursorW)(_In_opt_ HINSTANCE hInstance, _In_ LPCWSTR lpCursorName);
typedef HICON(WINAPI* FnLoadIconW)(_In_opt_ HINSTANCE hInstance, _In_ LPCWSTR lpIconName);
typedef VOID(WINAPI* FnExitProcess)(_In_ UINT uExitCode);
typedef HWND(WINAPI* FnGetDlgItem)(_In_opt_ HWND hDlg, _In_ int nIDDlgItem);
typedef BOOL(WINAPI* FnVirtualProtect)(_In_ LPVOID lpAddress, _In_ SIZE_T dwSize, _In_ DWORD flNewProtect, _Out_ PDWORD lpflOldProtect);
typedef
BOOL
(WINAPI*
	FnVirtualFree)(
		_Pre_notnull_ _When_(dwFreeType == MEM_DECOMMIT, _Post_invalid_) _When_(dwFreeType == MEM_RELEASE, _Post_ptr_invalid_) LPVOID lpAddress,
		_In_ SIZE_T dwSize,
		_In_ DWORD dwFreeType
		);
typedef HWND(WINAPI* FnFindWindowExW)(_In_opt_ HWND hWndParent, _In_opt_ HWND hWndChildAfter, _In_opt_ LPCWSTR lpszClass, _In_opt_ LPCWSTR lpszWindow);
typedef LRESULT(WINAPI* FnSendMessageW)(
	_In_ HWND hWnd,
	_In_ UINT Msg,
	_Pre_maybenull_ _Post_valid_ WPARAM wParam,
	_Pre_maybenull_ _Post_valid_ LPARAM lParam
	);
typedef HANDLE(WINAPI* FnCreateThread)(
	_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_ SIZE_T dwStackSize,
	_In_ LPTHREAD_START_ROUTINE lpStartAddress,
	_In_opt_ __drv_aliasesMem LPVOID lpParameter,
	_In_ DWORD dwCreationFlags,
	_Out_opt_ LPDWORD lpThreadId
	);
typedef VOID(WINAPI* FnSleep)(
	_In_ DWORD dwMilliseconds
	);
typedef LPVOID
(WINAPI* FnVirtualAlloc)(
	_In_opt_ LPVOID lpAddress,
	_In_     SIZE_T dwSize,
	_In_     DWORD flAllocationType,
	_In_     DWORD flProtect
	);
typedef int (WINAPI* FnGetDlgCtrlID)(
	_In_ HWND hWnd
	);
typedef HWND(WINAPI* FnFindWindowW)(
	_In_opt_ LPCWSTR lpClassName,
	_In_opt_ LPCWSTR lpWindowName
	);
typedef BOOL(WINAPI* FnPostMessage) (
	HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam
	);
typedef WINBASEAPI
_Check_return_
_Post_equals_last_error_
DWORD
(WINAPI* FnGetLastError)(
	VOID
	);
typedef _ACRTIMP FILE* (__cdecl* Fnfopen)(
	_In_z_ char const* _FileName,
	_In_z_ char const* _Mode
	);
typedef _ACRTIMP size_t(__cdecl* Fnfread)(
	_Out_writes_bytes_(_ElementSize* _ElementCount) void* _Buffer,
	_In_                                             size_t _ElementSize,
	_In_                                             size_t _ElementCount,
	_Inout_                                          FILE* _Stream
	);
typedef _ACRTIMP _CRT_HYBRIDPATCHABLE
void(__cdecl* Fnfree)(
	_Pre_maybenull_ _Post_invalid_ void* _Block
	);
typedef void* (__cdecl* Fnmemset)(
	_Out_writes_bytes_all_(_Size) void* _Dst,
	_In_                          int    _Val,
	_In_                          size_t _Size
	);
typedef void* (__cdecl* Fnmemcpy)(
	_Out_writes_bytes_all_(_Size) void* _Dst,
	_In_reads_bytes_(_Size)       void const* _Src,
	_In_                          size_t      _Size
	);
typedef _VCRTIMP void* (__cdecl* Fnmemmove)(
	_Out_writes_bytes_all_opt_(_Size) void* _Dst,
	_In_reads_bytes_opt_(_Size)       void const* _Src,
	_In_                              size_t      _Size
	);
typedef _ACRTIMP int(__cdecl* FnMy_stricmp)(
	_In_z_ char const* _String1,
	_In_z_ char const* _String2
	);
typedef _ACRTIMP int(__cdecl* Fn_strnicmp)(
	_In_reads_or_z_(_MaxCount) char const* _String1,
	_In_reads_or_z_(_MaxCount) char const* _String2,
	_In_                       size_t      _MaxCount
	);

#ifdef _WIN64
extern "C" {
	//==============================================================================
	//    x32:fs  x64:gs  Load dll Module and Get API Address
	//==============================================================================
	void __stdcall puGetModule(const DWORD Hash, DWORD64* address);
	PVOID64 __stdcall puGetProcAddress(const DWORD64 modules, DWORD Hash);
	void __stdcall CodeExecEntry(DWORD64 oep);
	//==============================================================================
	//    VmCode Exec Handler
	//==============================================================================
	void __stdcall VmSub_RSPHandle(unsigned char* opcode, unsigned __int64 addr);	// 48:83EC + imm立即数

	void __stdcall VmLea_RDXHandle(unsigned char* opcode, unsigned __int64 addr, unsigned __int64 regaddr);	// 48:8D15 6DD10000
	void __stdcall VmLea_RCXHandle(unsigned char* opcode, unsigned __int64 addr, unsigned __int64 regaddr);	// 48:8D15 6DD10000

	void __stdcall VmMov_RCXHandle(unsigned char* opcode, unsigned __int64 addr, unsigned __int64 regaddr);	// B9 18428C22
	void __stdcall VmMov_EDXHandle(unsigned char* opcode, unsigned __int64 addr, unsigned __int64 regaddr);
	void __stdcall VmMov_ECXHandle(unsigned char* opcode, unsigned __int64 addr, unsigned __int64 regaddr);
	void __stdcall VmMov_MemHandle(unsigned char* opcode, unsigned __int64 addr, unsigned __int64 regaddr);


	void __stdcall VmXor_r8dHandle(unsigned char* opcode, unsigned __int64 regaddr);
	void __stdcall VmXor_EcxHandle(unsigned char* opcode, unsigned __int64 regaddr);
	void __stdcall VmXor_EdxHandle(unsigned char* opcode, unsigned __int64 regaddr);

	void __stdcall VmCallE8_Handle(unsigned char* opcode, unsigned __int64 addr, unsigned __int64 regaddr, unsigned __int64 retrax);
	void __stdcall VmCallFF15_Handle(unsigned char* opcode, unsigned __int64 addr, unsigned __int64 regaddr, unsigned __int64 retrax);

	void __stdcall VmAdd_RspHandle(unsigned char* opcode, unsigned __int64 regaddr);
	void __stdcall VmRet_Handle(unsigned char* opcode, unsigned __int64 addr);
}
#else
	DWORD puGetModule(const DWORD Hash);
	DWORD puGetProcAddress(const DWORD dllvalues, const DWORD Hash);
#endif