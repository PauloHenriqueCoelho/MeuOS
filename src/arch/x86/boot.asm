; src/arch/x86/boot.asm

; --- Constantes do Multiboot ---
MBALIGN     equ  1 << 0
MEMINFO     equ  1 << 1
GRAPHICS    equ  1 << 2  ; Flag para pedir gráficos (Bit 2)
FLAGS       equ  MBALIGN | MEMINFO | GRAPHICS
MAGIC       equ  0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

; --- Configuração da Resolução (800x600x32) ---
MODE_TYPE   equ 0   ; 0 = Linear Graphics
WIDTH       equ 1024 ; <--- MUDOU DE 800
HEIGHT      equ 768  ; <--- MUDOU DE 600
DEPTH       equ 32  ; 32 bits (RGBA)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    
    ; --- Padding Obrigatório ---
    ; Como não estamos usando a flag de endereços (bit 16),
    ; precisamos preencher esses 5 campos com 0 para alcançar
    ; o offset 32, onde começam os campos de gráfico.
    dd 0, 0, 0, 0, 0

    ; --- Campos Gráficos ---
    dd MODE_TYPE
    dd WIDTH
    dd HEIGHT
    dd DEPTH

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KiB de Stack
stack_top:

section .text
global _start          ; Exporta _start para o Linker
extern kernel_main     ; Importa a função do C

_start:
    ; 1. Configura a Stack
    mov esp, stack_top
    and esp, -16  ; <--- Força alinhamento de 16 bytes na stack

    ; 2. Passa os argumentos do Multiboot para o Kernel
    ; O padrão C empilha da direita para a esquerda.
    ; void kernel_main(unsigned long magic, unsigned long addr)
    push ebx    ; Argumento 2: Endereço da Info Structure (addr)
    push eax    ; Argumento 1: Magic Number (magic)

    ; 3. Chama o Kernel
    call kernel_main

    ; 4. Trava se o kernel retornar
    cli
.hang:
    hlt
    jmp .hang