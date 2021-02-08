;ON VEUT =>
;	=> ouvir et fermer le fichier
;	=> applique la clef sur chaque bit un par un en modifiant ou on en est dans la clef a chaque fois
;	=> c'est tout (?)

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

KEY_SIZE    equ 16
;-x-x-x-x- CONSTANTS -x-x-x-x-;

global _start

section .text

_start:
	; Save register state, RBX can be safely used
	push rax
	push rcx
	push rdx
	push rdi
	push r11
	push r12

	

open_file:
	xor rax, rax
	xor rdi, rdi
	lea	rdi, [rel filename] 	; pathname
	xor rsi, rsi				; 0 for O_RDONLY macro
	mov al, SYS_OPEN			; syscall number for open()
	syscall

	xor r10, r10                ; Zeroing out temporary registers
    	xor r8, r8
    	xor rdi, rdi
    	xor rbx, rbx
    	mov dil, al                 ; Moving fd to rdi for read
    	;sub sp, ALLOC_SPACE         ; allocate space for /proc/<pid>/maps memory address string
                                ; (Max 16 chars from file | usually 12 chars 5567f9154000)
    	xor rdx, rdx

read_characters:
	; %rax      | %rdi | %rsi | %rdx
	; sys_read  |  fd  | *buf | count

	mov dx, 0x1                 ; count : Read 0x1 byte from file at a time
   	xor rax, rax                ; Syscall Number for read : 0x0
    	lea rsi, [rsp]              ; *buf  : get the content to be read on stack
    	mov rdi, 0x3                ; A CHANGER c'est de la merde le "mov dil, al"
	syscall                     ; Byte read at [rsp]
    	test eax, eax
	jz close_file               ; jump to the end if EOF
	jz done			    ; if error apparament	

	push rdi
	;; why tf does this not write
	mov rdx, 0x1                ; read returns amount of bytes read
	mov rax, 0x01               ; syscall write = 4
	mov rdi, 0x01               ; STDOUT = 1
	mov rsi, [rsp]              ; letter to print
	syscall
	pop rdi
	
	mov       rax, 1                  ; system call for write
        mov       rdi, 1                  ; file handle 1 is stdout
        mov       rsi, message            ; address of string to output
        mov       rdx, 13                 ; number of bytes
        syscall                           ; invoke operating system to do the write
	
	add rsp, 0x1                      ; increase rsp to keep reading
	
	;cmp BYTE [rsp], 0x22              ; check if the letter we're looking at is 'b'
	;je yesyesyes                      ; jump to yesyesyes if flais set to true
	jmp loop
 
loop:
        add rsp, 0x1                ; increment RSI to read character at next location
        lea rsi, [rsp]
        jmp read_characters

yesyesyes:
	mov       rax, 1                  ; system call for write
        mov       rdi, 1                  ; file handle 1 is stdout
       	mov       rsi, message            ; address of string to output
        mov       rdx, 13                 ; number of bytes
        syscall                           ; invoke operating system to do the write
	jmp loop	

close_file:
	;; CLOSE FILE

done:
	; Restoring register state
	pop r12
	pop r11
	pop rdi
	pop rdx
	pop rcx
	pop rax

        mov       rax, 60         	  ; system call for exit
        xor       rdi, rdi                ; exit code 0
        syscall                           ; invoke operating system to exit

section   .data
	message:  db        "Hello, World", 10      ; note the newline at the end
	filename: db	    "test", 0x0
	key:	  db        "123456789012345", 0x0  ; 15 character key and ending NULL character
