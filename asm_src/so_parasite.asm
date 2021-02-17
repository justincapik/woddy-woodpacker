BITS 64

global _start


;-x-x-x-x- CONSTANTS -x-x-x-x-;
STDIN       equ 0
STDOUT      equ 1
STDERR      equ 2

SYS_WRITE   equ 0x1         ; 1
SYS_EXIT    equ 0x3c        ; 60
SYS_READ    equ 0x0         ; 0
SYS_OPEN    equ 0x2         ; 2
SYS_CLOSE   equ 0x3         ; 3
SYS_LSEEK   equ 0x8         ; 8
SYS_EXECVE  equ 0x3b        ; 59
ALLOC_SPACE equ 0x20		; 32

O_RDONLY    equ 0x0         ; 0

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
	filepath:   db  "/proc/self/maps", 0x0          ; 16 bytes
	newline:	db 	0xa
	debug:		db 	"reboot" , 0xa
	encStart:	dq	0x1111111111111111
	;x0:		dq	0
	;x1:		dq	0
	;t0:		dq	0
	;t1:		dq	0

parasite:

	; Print our message
	; %rax      | %rdi | %rsi | %rdx
	; sys_write	|  fd  | *buf | count

	xor	rax, rax					; Zero out RAX
	add	rax, SYS_WRITE				; Syscall number of write() - 0x1
	mov rdi, rax					; File descriptor - 0x1 (STDOUT)
	lea rsi, [rel message]			; Addresses the label relative to RIP (Instruction Pointer), i.e. 
									; dynamically identifying the address of the 'message' label.
	xor rdx, rdx
	mov dl, 0x20					; message size = 30 bytes
	syscall

; .text is xor crypted, using a xor on .text mem to uncrypt it

;--------------------------------------------------------------------
unrox:

	;push r13
	;push r14
	;push r15

	; r10 -> encryption start
	; r13 -> key start
	; r15 -> end encryption (key start)
	;xor r13, r13
	;lea r13, [rel _start - KEY_ADDR]
	;mov r10, r13
	;sub r10, [rel encStart]
	
	;xor r13, r13
	;xor r14, r14
	;xor r15, r15
	;lea r15, [rel _start - KEY_ADDR]
	;lea r13, [rel _start - KEY_ADDR + 12]
	;lea r14, [rel _start - KEY_ADDR + 5]

roxLoop:
	;cmp r10, r15  ; if encryption addr = end encryption addr stop
	;je endRoxLoop

	; xor encrypted byte and key byte
	;xor cl, cl
	;xor al, al
	;mov cl, byte [r10]
	;mov al, byte [r13]
	;xor cl, al
	;mov byte [r10], cl

	; look if key is at the end
	;cmp r13, r14
	;jne endYekStuff

	; if key is at the end, restart key
rebootYek:
	;lea r13, [rel _start - KEY_ADDR + 13]

endYekStuff:
	; inc encryption addr and key addr then jump back at the beginning of loop
	;sub r13, 1
	;inc r10
	;jmp roxLoop

endRoxLoop:

	;pop r15
	;pop r14
	;pop r13

;--------------------------------------------------------------------

	%define enstart	r10
	%define key	r13
	%define x0  	r8d
    	%define x1  	r9d
    	%define t0  	r15d
    	%define t1  	r14d
    	%define sum 	r12d

	push enstart
	push r8
	push r9
	push r10
	push r12
	push r13
	push r14
	push r15
    
xtea:
	xor key, key
	lea key, [rel _start - KEY_ADDR]
	mov enstart, key
	sub enstart, [rel encStart]
    	
	mov    	rcx, 4		; counter
   	xor    	sum, sum          ; sum = 0
	mov	x0, dword [rel enstart]
	mov	x1, dword [rel enstart + 4]

	push rcx
		
	xor	rax, rax					; Zero out RAX
	add	rax, SYS_WRITE				; Syscall number of write() - 0x1
	mov rdi, rax					; File descriptor - 0x1 (STDOUT)
	lea rsi, [x0]
	mov dl, 0x20					; message size = 30 bytes
	syscall

	xor	rax, rax					; Zero out RAX
	add	rax, SYS_WRITE				; Syscall number of write() - 0x1
	mov rdi, rax					; File descriptor - 0x1 (STDOUT)
	lea rsi, [x1]
	mov dl, 0x20					; message size = 30 bytes
	syscall

	xor	rax, rax					; Zero out RAX
	add	rax, SYS_WRITE				; Syscall number of write() - 0x1
	mov rdi, rax					; File descriptor - 0x1 (STDOUT)
	lea rsi, [0]
	mov dl, 0x20					; message size = 30 bytes
	syscall
	
	pop rcx

xtea_enc:
    	mov    t0, x1            ; t0   = x1 << 4
    	
	shl    t0, 4
    
    	mov    t1, x1            ; t1   = x1 >> 5
    	shr    t1, 5    
    
    	xor    t0, t1            ; t0  ^= t1
    	add    t0, x1            ; t0  += x1;
    
    	mov    t1, sum           ; t1   = sum
    	test   cl, 1
    	jz     xtea_l1
    
    	add    sum, 0x9E3779B9   ; sum += 0x9E3779B9   
    	mov    t1, sum     
    	shr    t1, 11            ; t1 = sum >> 11  
xtea_l1:    
    	
	and    	t1, 3             ; t1  &= 3
	add	t1, t1		 ; t1 * 4
	add	t1, t1	

    	add	t1, dword [key]      ; t1 = sum + k[t1]
    	add    	t1, sum
    
    	xor    t0, t1            ; t0 ^= t1
    
    	;add   	x0, t0            ; x0 += t0
    	;xchg   x0, x1            ; XCHG(x0, x1); 
    	loop   	xtea_enc    
	
	push rcx
		
	xor	rax, rax					; Zero out RAX
	add	rax, SYS_WRITE				; Syscall number of write() - 0x1
	mov rdi, rax					; File descriptor - 0x1 (STDOUT)
	lea rsi, [x0]
	mov dl, 0x20					; message size = 30 bytes
	syscall

	xor	rax, rax					; Zero out RAX
	add	rax, SYS_WRITE				; Syscall number of write() - 0x1
	mov rdi, rax					; File descriptor - 0x1 (STDOUT)
	lea rsi, [t0]
	mov dl, 0x20					; message size = 30 bytes
	syscall

	xor	rax, rax					; Zero out RAX
	add	rax, SYS_WRITE				; Syscall number of write() - 0x1
	mov rdi, rax					; File descriptor - 0x1 (STDOUT)
	lea rsi, [0]
	mov dl, 0x20					; message size = 30 bytes
	syscall
	
	pop rcx

	;mov dword [enstart], x0
	;mov dword [enstart + 4], x1 
	


	pop r15
	pop r14
	pop r13
	pop r12
	pop r10
	pop r9
	pop r8
	pop enstart
    

;--------------------------------------------------------------------

unxor:

	;push r13
	;push r15

	; r10 -> encryption start
	; r13 -> key start
	; r15 -> end encryption (key start)
	;xor r13, r13
	;lea r13, [rel _start - KEY_ADDR]
	;mov r10, r13
	;sub r10, [rel encStart]

	;xor r15, r15
	;mov r15, r13

xorLoop:
	;cmp r10, r15  ; if encryption addr = end encryption addr stop
	;je endXorLoop

	; xor encrypted byte and key byte
	;xor cl, cl
	;xor al, al
	;mov cl, byte [r10]
	;mov al, byte [r13]
	;xor cl, al
	;mov byte [r10], cl

	; look if key is at the end
	;cmp byte [r13], 0
	;jne endKeyStuff

	; if key is at the end, restart key
rebootKey:
	;lea r13, [rel _start - KEY_ADDR - 1]

endKeyStuff:
	; inc encryption addr and key addr then jump back at the beginning of loop
	;inc r13
	;inc r10
	;jmp xorLoop

endXorLoop:

	;pop r15
	;pop r13

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
