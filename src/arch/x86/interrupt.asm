; src/arch/x86/interrupt.asm
section .text
extern isr_handler
global idt_flush

idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

; Macros
%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

%macro IRQ 2
  global irq%1
  irq%1:
    cli
    push byte 0
    push byte %2
    jmp isr_common_stub
%endmacro

; Definições
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_ERRCODE   14
ISR_NOERRCODE 128 ; Syscall

IRQ   0,    32
IRQ   1,    33
IRQ   8,    40
IRQ   12,   44
IRQ   14,   46
IRQ   15,   47

; --- O STUB CORRIGIDO (Passagem por Ponteiro) ---
isr_common_stub:
    pusha           ; Salva registradores

    mov ax, ds
    push eax        ; Salva DS

    mov ax, 0x10    ; Carrega Kernel DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp        ; <--- O PULO DO GATO: Empilha o ponteiro da stack!
    call isr_handler
    add esp, 4      ; Limpa o argumento (ponteiro)

    pop eax         ; Restaura DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa            ; Restaura registradores
    add esp, 8      ; Limpa erro/int_no
    sti
    iret