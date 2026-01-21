global start
extern kernel_main

; Cabeçalho Multiboot (Obrigatório para o GRUB/QEMU)
section .multiboot
align 4
dd 0x1BADB002             ; Magic number
dd 0x00                   ; Flags
dd -(0x1BADB002 + 0x00)   ; Checksum

section .text
bits 32

start:
    ; 1. Configura a Stack (Pilha)
    mov esp, stack_top

    ; 2. CRUCIAL: NÃO MEXA NOS REGISTRADORES DE SEGMENTO AQUI!
    ; O GRUB já configura eles corretamente. Se zerarmos (DS=0), o Kernel trava.
    
    ; 3. Chama o Kernel em C
    call kernel_main

    ; 4. Se o kernel retornar, trava a CPU
    cli
    hlt
.loop:
    jmp .loop

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KB Stack
stack_top: