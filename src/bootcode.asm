; bootcode.asm
;
; Copyright (C) 2024 Raine Fairlie <raine@chaoticgoose.com>

bits 16
org 0x7c00

mov si, 0

print:
    mov ah, 0x0e
    mov al, [hello + si]
    int 0x10
    add si, 1
    cmp byte [hello + si], 0
    jne print
    
int 0x18
jmp $

hello:
    db "This is not a bootable disk", 0xa, 0xd, 0

times 510 - ($ - $$) db 0
dw 0xAA55
