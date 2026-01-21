global idt_flush
global isr1
extern keyboard_handler_isr ; Nome da função C que vai receber o sinal
global isr12
extern mouse_handler_isr
global isr0   ; IRQ 0 = Timer
extern timer_callback
global isr128
extern syscall_handler


; Carrega a tabela IDT
idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

isr0:
    cli
    pusha
    
    call timer_callback
    
    ; Envia EOI para o PIC Mestre (0x20)
    mov al, 0x20
    out 0x20, al
    
    popa
    sti
    iret

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

isr12:
    cli
    pusha               ; Salva registradores
    
    call mouse_handler_isr ; Chama o driver C do mouse
    
    ; Envia EOI (End of Interrupt) para PIC Mestre (0x20) e Escravo (0xA0)
    ; O mouse está no Escravo, então precisamos avisar os dois.
    mov al, 0x20
    out 0xA0, al        ; Avisa o Escravo
    out 0x20, al        ; Avisa o Mestre
    
    popa                ; Restaura registradores
    sti
    iret

isr128:
    cli             ; Desativa interrupções
    pusha           ; Salva EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    ; Empurra os registadores como um argumento para a função C
    push esp        
    call syscall_handler
    add esp, 4      ; Limpa o argumento da stack
    
    popa            ; Restaura os registadores
    sti             ; Reativa interrupções
    iret            ; Volta para o programa de utilizador

