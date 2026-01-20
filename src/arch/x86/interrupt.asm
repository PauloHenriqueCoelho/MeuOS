global idt_flush
global isr1
extern keyboard_handler_isr ; Nome da função C que vai receber o sinal

; Carrega a tabela IDT
idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

; --- HANDLER DO TECLADO (IRQ 1) ---
isr1:
    cli                 ; 1. Desliga interrupções para não encavalar
    pusha               ; 2. Salva todos os registradores (A, B, C, D...)

    call keyboard_handler_isr ; 3. Chama o código C

    ; 4. Envia o sinal de "Pronto" (EOI - End of Interrupt) para o PIC
    mov al, 0x20
    out 0x20, al

    popa                ; 5. Restaura os registradores
    sti                 ; 6. Religa interrupções
    iret                ; 7. Volta para onde o código estava antes de você digitar