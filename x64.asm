.code 
	start:
;----------------------------------------------------------------------------
; AsmCountTemp at _Start 
;----------------------------------------------------------------------------
; rcx = *temp
;----------------------------------------------------------------------------
 AsmCountTemp PROC
    PUSH rax
	PUSH rbx
	PUSH rcx
	PUSH rdx
	PUSH rsi
	PUSH rdi
	XOR rdi, rdi
	MOV rdi, rcx
	MOV esi, dword ptr [rcx]
	MOV eax, dword ptr [rcx]
	MOV edx, 1h
	MOV cx, 1000h
	div cx
	test dx, dx
	jz MemSucess
	shr dx, 12
	inc dx
    shl dx, 12;
    add esi, edx;
    shr esi, 12;
    shl esi, 12;
    mov dword ptr [rdi], esi;
MemSucess:
    POP rdi
    POP rsi
	POP rdx
	POP rcx
	POP rbx
	POP rax
	ret
AsmCountTemp ENDP

;----------------------------------------------------------------------------
; AsmCountTemp1 at _Start 
;----------------------------------------------------------------------------
; rcx = tempvalues
;----------------------------------------------------------------------------
AsmCountTemp1 PROC
    push rax
	push rbx
	push rcx
	push rdx
	push rsi
	PUSH rdi
	sub rdi, rdi
	mov rdi, rcx
    mov	    esi, dword ptr [rcx];
    mov	    edx, 1h;
    mov		eax, esi;
    mov		ecx, 200h;
    div		cx;
    test	dx, dx;
    jz		FileSucess
    xor		eax, eax
    mov		ax, 200h;
    sub		ax, dx;
    add		esi, eax;
    mov		dword ptr [rdi], esi;
FileSucess:
    POP     rdi
    pop     rsi
	pop     rdx
	pop     rcx
	pop     rbx
	pop     rax
	ret
AsmCountTemp1 ENDP
;----------------------------------------------------------
end
