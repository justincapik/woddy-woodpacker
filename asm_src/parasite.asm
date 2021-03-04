BITS 64

global _start


;-x-x-x-x- CONSTANTS -x-x-x-x-;
SYS_WRITE   equ 0x1         ; 1
KEY_ADDR    equ 0x10        ; 16
;-x-x-x-x- CONSTANTS -x-x-x-x-;



section .text
_start:
	; Save register state, RBX can be safely used
	push rax
	push rcx
	push rdx
	push rdi

	jmp	parasite
	message:	db	"....WOODY....", 0xa
	filepath:   db  "/proc/self/maps", 0x0
	newline:	db 	0xa
	debug:		db 	"another one" , 0xa
	encStart:	dq	0x1111111111111111

parasite:

	; Print our message
	; %rax      | %rdi | %rsi | %rdx
	; sys_write	|  fd  | *buf | count

	xor	rax, rax					; Zero out RAX
	add	rax, SYS_WRITE				; Syscall number of write() - 0x1
	mov rdi, rax					; File descriptor - 0x1 (STDOUT)
	lea rsi, [rel message]			; Addresses the label relative to RIP (Instruction Pointer)
	xor rdx, rdx
	mov dl, 14					; message size = 30 bytes
	syscall

; .text is xor crypted, using a xor on .text mem to uncrypt it

;--------------------------------------------------------------------
unrox:

	push r13
	push r14
	push r15

	; r10 -> encryption start
	; r13 -> key start
	; r15 -> end encryption (key start)
	xor r13, r13
	lea r13, [rel _start - KEY_ADDR]
	mov r10, r13
	sub r10, [rel encStart]

	xor r13, r13
	xor r14, r14
	xor r15, r15
	lea r15, [rel _start - KEY_ADDR]
	lea r13, [rel _start - KEY_ADDR + 12]
	lea r14, [rel _start - KEY_ADDR + 5]

roxLoop:
	cmp r10, r15  ; if encryption addr = end encryption addr stop
	je endRoxLoop

	; xor encrypted byte and key byte
	xor cl, cl
	xor al, al
	mov cl, byte [r10]
	mov al, byte [r13]
	xor cl, al
	mov byte [r10], cl

	; look if key is at the end
	cmp r13, r14
	jne endYekStuff

	; if key is at the end, restart key
rebootYek:
	lea r13, [rel _start - KEY_ADDR + 13]

endYekStuff:
	; inc encryption addr and key addr then jump back at the beginning of loop
	sub r13, 1
	inc r10
	jmp roxLoop

endRoxLoop:

	pop r15
	pop r14
	pop r13

;--------------------------------------------------------------------


; void	dechiffrer (u_int32_t* v, u_int32_t* k)
; {
;     u_int32_t v0=v[0], v1=v[1], sum=0xC6EF3720, i;  /* initialisation */
;     u_int32_t delta=0x9e3779b9;                     /* constantes de clefs */
;     u_int32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* mise en cache de la clef */
;     for (i=0; i<32; i++)
;     {
;         v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
;         v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
;         sum -= delta;
;     }
;     v[0]=v0; v[1]=v1;
; }
	push r8
	push r9
	push r12
	push r13
	push r14
	push r15

	xor r10, r10
	xor r13, r13
	xor r15, r15

	; r10 -> key start
	; r13 -> encryption start
	; r15 -> encryption end (key start)
	lea r10, [rel _start - KEY_ADDR]
	mov r15, r10
	; sub r15, 8
	mov r13, r10
	sub r13, [rel encStart]
	; v0 = dword [r13]
	; v1 = dword [r13 + 4]

	; k0 = dword [r10]
	; k1 = dword [r10 + 4]
	; k2 = dword [r10 + 8]
	; k3 = dword [r10 + 12]

extendedTeaLoop:

	add r13, 8
	cmp r13, r15
	jg endExtendedTeaLoop

	sub r13, 8

	; xor	rax, rax
	; add	rax, SYS_WRITE
	; mov rdi, rax
	; mov rsi, r13
	; xor rdx, rdx
	; mov dl, 8
	; syscall

	; ######################################################
	; ######################################################

	xor r12, r12
	xor r14, r14

	; init loop
	mov r14d, 0xc6ef3720
	mov r12, 0
teaLoop:
	
	inc r12

	; ######################################################
	; ((v0<<4) + k2)
	xor r9, r9
	mov r9d, dword [r13]
	shl r9d, 4
	add r9d, dword [r10 + 8]
	; (v0 + sum)
	mov r8d, dword [r13]
	add r8d, r14d
	; ((v0<<4) + k2) ^ (v0 + sum)
	xor r9d, r8d
	; ((v0>>5) + k3)
	xor r8, r8
	mov r8d, dword [r13]
	shr r8d, 5
	add r8d , dword [r10 + 12]
	; ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3)
	xor r9d, r8d
	; v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
	sub dword [r13 + 4], r9d
	; ######################################################
	; ((v1<<4) + k0)
	xor r9, r9
	mov r9d, dword [r13 + 4]
	shl r9d, 4
	add r9d, dword [r10]
	; (v1 + sum)
	mov r8d, dword [r13 + 4]
	add r8d, r14d
	; ((v1<<4) + k0) ^ (v1 + sum)
	xor r9d, r8d
	; ((v1>>5) + k1)
	xor r8, r8
	mov r8d, dword [r13 + 4]
	shr r8d, 5
	add r8d , dword [r10 + 4]
	; ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1)
	xor r9d, r8d
	; v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
	sub dword [r13], r9d
	; ######################################################
	sub r14d, 0x9e3779b9

	cmp r12, 32
	jne teaLoop

endTeaLoop:

	; ######################################################
	; ######################################################

	add r13, 8
	jmp extendedTeaLoop

endExtendedTeaLoop:

	pop r15
	pop r14
	pop r13
	pop r12
	pop r9
	pop r8

;--------------------------------------------------------------------

unxor:

	push r13
	push r15

	; r10 -> encryption start
	; r13 -> key start
	; r15 -> end encryption (key start)
	xor r13, r13
	lea r13, [rel _start - KEY_ADDR]
	mov r10, r13
	sub r10, [rel encStart]

	xor r15, r15
	mov r15, r13

xorLoop:
	cmp r10, r15  ; if encryption addr = end encryption addr stop
	je endXorLoop

	; xor encrypted byte and key byte
	xor cl, cl
	xor al, al
	mov cl, byte [r10]
	mov al, byte [r13]
	xor cl, al
	mov byte [r10], cl

	; look if key is at the end
	cmp byte [r13], 0
	jne endKeyStuff

	; if key is at the end, restart key
rebootKey:
	lea r13, [rel _start - KEY_ADDR - 1]

endKeyStuff:
	; inc encryption addr and key addr then jump back at the beginning of loop
	inc r13
	inc r10
	jmp xorLoop

endXorLoop:

	pop r15
	pop r13

;--------------------------------------------------------------------

ender:
	; Restoring register state
	pop rdi
	pop rdx
	pop rcx
	pop rax
	
	push r12

	xor r12, r12
	xor r10, r10
	mov r12, 0xAAAAAAAAAAAAAAAA ; get jump addr from patch
	lea r10, [rel _start - KEY_ADDR] ; get rel addr of end of text
	sub r10, r12 ; sub size of [entrypoint - _start] to _start addr

	pop r12

	jmp	r10
