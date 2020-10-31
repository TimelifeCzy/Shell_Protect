#include "HashOfFunction.h"

HashofFunction::HashofFunction()
{

}

HashofFunction::~HashofFunction()
{

}

#ifdef _WIN64

#else
DWORD HashofFunction::GetModule(const DWORD Hash)
{
	DWORD	nDllBase = 0;
	__asm{
	/*Ö÷Ä£¿é*/
		pushad
		push		Hash;
		call		GetModulVA
		mov			nDllBase, eax;
		popad

	/*º¯Êý1£º±éÀúPEB_LDR_DATAÁ´±íHASH¼ÓÃÜ*/
	GetModulVA :
		push		ebp;
		mov			ebp, esp;
		sub			esp, 0x20;
		push		edx;
		push		ebx;
		push		edi;
		push		esi;
		mov			ecx, 8;
		mov			eax, 0CCCCCCCCh;
		lea			edi, dword ptr[ebp - 0x20];
		rep stos	dword ptr es : [edi]
		mov			esi, dword ptr fs : [0x30];
		mov			esi, dword ptr[esi + 0x0C];
		mov			esi, dword ptr[esi + 0x1C];
	tag_Modul:
		mov			dword ptr[ebp - 0x8], esi;	// ±£´æLDR_DATA_LIST_ENTRY
		mov			ebx, dword ptr[esi + 0x20];	// DLLµÄÃû³ÆÖ¸Õë(Ó¦¸ÃÖ¸ÏòÒ»¸ö×Ö·û´®)
		mov			eax, dword ptr[ebp + 0x8];
		push		eax;
		push		ebx;						// +0xC
		call		HashModulVA
		test		eax, eax;
		jnz			_ModulSucess
		mov			esi, dword ptr[ebp - 0x8];
		mov			esi, [esi];					// ±éÀúÏÂÒ»¸ö
		LOOP		tag_Modul
	_ModulSucess :
		mov			esi, dword ptr[ebp - 0x8];
		mov			eax, dword ptr[esi + 0x8];
		pop			esi;
		pop			edi;
		pop			ebx;
		pop			edx;
		mov			esp, ebp;
		pop			ebp;
		ret

	/*º¯Êý2£ºHASH½âÃÜËã·¨£¨¿í×Ö·û½âÃÜ£©*/
	HashModulVA :
		push		ebp;
		mov			ebp, esp;
		sub			esp, 0x04;
		mov			dword ptr[ebp - 0x04], 0x00
		push		ebx;
		push		ecx;
		push		edx;
		push		esi;
		// »ñÈ¡×Ö·û´®¿ªÊ¼¼ÆËã
		mov			esi, [ebp + 0x8];
		test		esi, esi;
		jz			tag_failuers
		xor			ecx, ecx;
		xor			eax, eax;
	tag_loops:
		mov			al, [esi + ecx];		// »ñÈ¡×Ö½Ú¼ÓÃÜ
		test		al, al					// 0ÔòÍË³ö
		jz			tag_ends
		mov			ebx, [ebp - 0x04];
		shl			ebx, 0x19;
		mov			edx, [ebp - 0x04];
		shr         edx, 0x07;
		or			ebx, edx;
		add			ebx, eax;
		mov[ebp - 0x4], ebx;
		inc			ecx;
		inc			ecx;
		jmp			tag_loops
	tag_ends :
		mov			ebx, [ebp + 0x0C];		// »ñÈ¡HASH
		mov			edx, [ebp - 0x04];
		xor			eax, eax;
		cmp			ebx, edx;
		jne			tag_failuers
		mov			eax, 1
		jmp			tag_funends
		tag_failuers :
		mov			eax, 0
	tag_funends :
		pop			esi;
		pop			edx;
		pop			ecx;
		pop			ebx;
		mov			esp, ebp;
		pop			ebp;
		ret			0x08
	}
	return nDllBase;
}

DWORD HashofFunction::GetProcAddress(const DWORD dllvalues, const DWORD Hash)
{
	DWORD FunctionAddress = 0;
	__asm{
		pushad;
		push		Hash;						// Hash¼ÓÃÜµÄº¯ÊýÃû³Æ
		push		dllvalues;					// Ä£¿é»ùÖ·.dll
		call		GetHashFunVA;				// GetProcess
		mov			FunctionAddress, eax;		// ¡î ±£´æµØÖ·
		popad;
		// ×Ô¶¨Òåº¯Êý¼ÆËãHashÇÒ¶Ô±È·µ»ØÕýÈ·µÄº¯Êý
	GetHashFunVA:
		push		ebp;
		mov			ebp, esp;
		sub			esp, 0x30;
		push		edx;
		push		ebx;
		push		esi;
		push		edi;
		lea			edi, dword ptr[ebp - 0x30];
		mov			ecx, 12;
		mov			eax, 0CCCCCCCCh;
		rep	stos	dword ptr es : [edi];
		// ÒÔÉÏ¿ª±ÙÕ»Ö¡²Ù×÷£¨Debug°æ±¾Ä£Ê½£©
		mov			eax, [ebp + 0x8];				// ¡î kernel32.dll(MZ)
		mov			dword ptr[ebp - 0x8], eax;
		mov			ebx, [ebp + 0x0c];				// ¡î GetProcAddress HashÖµ
		mov			dword ptr[ebp - 0x0c], ebx;
		// »ñÈ¡PEÍ·ÓëRVA¼°ENT
		mov			edi, [eax + 0x3C];				// e_lfanew
		lea			edi, [edi + eax];				// e_lfanew + MZ = PE
		mov			dword ptr[ebp - 0x10], edi;		// ¡î ±£´æPE£¨VA£©
		// »ñÈ¡ENT
		mov			edi, dword ptr[edi + 0x78];		// »ñÈ¡µ¼³ö±íRVA
		lea			edi, dword ptr[edi + eax];		// µ¼³ö±íVA
		mov[ebp - 0x14], edi;						// ¡î ±£´æµ¼³ö±íVA
		// »ñÈ¡º¯ÊýÃû³ÆÊýÁ¿
		mov			ebx, [edi + 0x18];
		mov			dword ptr[ebp - 0x18], ebx;		// ¡î ±£´æº¯ÊýÃû³ÆÊýÁ¿
		// »ñÈ¡ENT
		mov			ebx, [edi + 0x20];				// »ñÈ¡ENT(RVA)
		lea			ebx, [eax + ebx];				// »ñÈ¡ENT(VA)
		mov			dword ptr[ebp - 0x20], ebx;		// ¡î ±£´æENT(VA)
		// ±éÀúENT ½âÃÜ¹þÏ£Öµ¶Ô±È×Ö·û´®
		mov			edi, dword ptr[ebp - 0x18];
		mov			ecx, edi;
		xor			esi, esi;
		mov			edi, dword ptr[ebp - 0x8];
		jmp			_WHILE
		// Íâ²ã´óÑ­»·
		_WHILE :
		mov			edx, dword ptr[ebp + 0x0c];		// HASH
		push		edx;
		mov			edx, dword ptr[ebx + esi * 4];	// »ñÈ¡µÚÒ»¸öº¯ÊýÃû³ÆµÄRVA
		lea			edx, [edi + edx];				// »ñÈ¡Ò»¸öº¯ÊýÃû³ÆµÄVAµØÖ·
		push		edx;							// ENT±íÖÐµÚÒ»¸ö×Ö·û´®µØÖ·
		call		_STRCMP
		cmp			eax, 0;
		jnz			_SUCESS
		inc			esi;
		LOOP		_WHILE
		jmp			_ProgramEnd
		// ¶Ô±È³É¹¦Ö®ºó»ñÈ¡Ñ­»·´ÎÊý£¨ÏÂ±ê£©cx±£´æÏÂ±êÊý
	_SUCESS :
		// »ñÈ¡EOTµ¼³öÐòºÅ±íÄÚÈÝ
		mov			ecx, esi
		mov			ebx, dword ptr[ebp - 0x14];
		mov			esi, dword ptr[ebx + 0x24];
		mov			ebx, dword ptr[ebp - 0x8];
		lea			esi, [esi + ebx];				// »ñÈ¡EOTµÄVA
		xor			edx, edx
		mov			dx, [esi + ecx * 2];			// ×¢ÒâË«×Ö »ñÈ¡ÐòºÅ
		// »ñÈ¡EATµØÖ·±íRVA
		mov			esi, dword ptr[ebp - 0x14];		// Export VA
		mov			esi, [esi + 0x1C];
		mov			ebx, dword ptr[ebp - 0x8];
		lea			esi, [esi + ebx]				// »ñÈ¡EATµÄVA			
		mov			eax, [esi + edx * 4]			// ·µ»ØÖµeax£¨GetProcessµØÖ·£©
		lea			eax, [eax + ebx]
		jmp			_ProgramEnd

	_ProgramEnd :
		pop			edi;
		pop			esi;
		pop			ebx;
		pop			edx;
		mov			esp, ebp;
		pop			ebp;
		ret			0x8;

		// Ñ­»·¶Ô±ÈHASHÖµ
	_STRCMP:
		push		ebp;
		mov			ebp, esp;
		sub			esp, 0x04;
		mov			dword ptr[ebp - 0x04], 0x00;
		push		ebx;
		push		ecx;
		push		edx;
		push		esi;
		// »ñÈ¡×Ö·û´®¿ªÊ¼¼ÆËã
		mov			esi, [ebp + 0x8];
		xor			ecx, ecx;
		xor			eax, eax;

	tag_loop:
		mov			al, [esi + ecx];		// »ñÈ¡×Ö½Ú¼ÓÃÜ
		test		al, al;					// 0ÔòÍË³ö
		jz			tag_end;
		mov			ebx, [ebp - 0x04];
		shl			ebx, 0x19;
		mov			edx, [ebp - 0x04];
		shr         edx, 0x07;
		or			ebx, edx;
		add			ebx, eax;
		mov[ebp - 0x4], ebx;
		inc			ecx;
		jmp			tag_loop

	tag_end :
		mov			ebx, [ebp + 0x0C];		// »ñÈ¡HASH
		mov			edx, [ebp - 0x04];
		xor			eax, eax;
		cmp			ebx, edx;
		jne			tag_failuer;
		mov			eax, 1;
		jmp			tag_funend

	tag_failuer :
		mov			eax, 0

	tag_funend :
		pop			esi;
		pop			edx;
		pop			ecx;
		pop			ebx;
		mov			esp, ebp;
		pop			ebp;
		ret			0x08
	}
	return FunctionAddress;
}
#endif 