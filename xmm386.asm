; xmm 386
.model tiny

.386
.code

PUBLIC C SWITCH32

SWITCH32 PROC
        mov ax, ss
        mov word ptr cs:[rm_ss], ax
        mov ax, cs
        mov word ptr cs:[rm_cs], ax
        mov ax, sp
        mov word ptr cs:[rm_sp], ax
        xor eax, eax
        mov ax, cs
        shl eax, 4
        add dword ptr cs:[cs_desc+2], eax ; base of cs in bytes
        add dword ptr cs:[gdt_desc+2], eax ; address of GDT (physical linear address)

        jmp enter_pmode

gdt_desc:
        dw      20h
        dd      offset gdt

gdt:
        dq      0       ; NULL descriptor
cs_desc:
        dw 0ffffh       ; limit FFFF
        dw 00h          ; base ??  0000
        db 00h          ;      ??00????
        db 10011010b    ; access byte [Ac 0, Readable DC 0 EX 1 S 1 Privl 00 Pr 1]
        db 10001111b    ; flags 1000 limit 1111 (4kb limit 16bit)
        db 00           ; base 00??????
data_desc:
        db 0ffh, 0ffh, 0, 0, 0, 10010010b, 11001111b, 0
stack_desc:
        db 0ffh, 0ffh, 0, 0, 0, 10010010b, 11001111b, 0

.386p
enter_pmode:
        lgdt fword ptr cs:[gdt_desc]
        cli
        in al, 70h
        or al, 80h
        out 70h, al

        mov eax, cr0
        or eax, 1
        mov cr0, eax
        
; JMP FAR 08:PMODE_BLOCK => switch into 32bit mode!
        db 0eah
pmode_patch:
        dd offset pmode_block
        dw 08h

pmode_block:
        mov bx, 10h             ; set ds/es
        mov ds, bx
        mov es, bx
        
        mov esi, 0b8000h
        mov al, 'H'
        mov ah, 28h
        mov word ptr [esi], ax
        mov al, 'e'
        add esi, 2
        mov word ptr [esi], ax

exit_pmode:
        mov eax, cr0
        and al, 0feh
        mov cr0, eax
        db 0eah  ; jmp far
        dw offset real_mode
rm_cs:  dw 0
rm_ss:  dw 0
rm_sp:  dw 0

HelloBack db 'Hello world! We are in real mode!$'

.386
real_mode:
        in al, 70h
        and al, 07fh
        out 70h, al
        sti

        mov ax, cs
        mov sp, word ptr cs:[rm_sp]
        mov ss, word ptr cs:[rm_ss]     ; restore ss/sp complete..
        
        mov ds, ax
        mov es, ax
        lea dx, HelloBack

exit:
        mov ax, 4c00h
        int 21h

switch_32bit:
        jmp real_mode

SWITCH32 ENDP
end
