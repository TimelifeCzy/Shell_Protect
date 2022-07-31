#pragma once
#ifndef STUD_H_
#define STUD_H_
#include <Windows.h>

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

/*
	Capstone Ptr
*/
typedef struct _PRT
{
	PVOID64 free;
	PVOID64 calloc;
	PVOID64 malloc;
	PVOID64 realloc;
	PVOID64 vsprintf_s;
};
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