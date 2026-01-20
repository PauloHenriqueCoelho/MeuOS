global idt_flush
global isr1
extern keyboard_handler_main ; Função em C que criaremos depois

; Carrega a IDT (semelhante ao GDT flush)
idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

; Este é o código que roda quando a tecla é pressionada (IRQ 1)
isr1:
    cli                 ; Desabilita interrupções temporariamente
    pusha               ; Salva TODOS os registros (EAX, EBX, etc) na pilha

    ; Opcional: Chama função C para lidar com a lógica
    call keyboard_handler_main

    ; Envia sinal de "Recebido" (EOI) para o PIC Mestre (0x20)
    ; Se não fizermos isso, o PIC nunca mais manda outra interrupção
    mov al, 0x20
    out 0x20, al

    popa                ; Restaura os registros
    sti                 ; Reabilita interrupções
    iret                ; "Interrupt Return" - volta pro código que estava rodando

    ; ... (código anterior do isr1 ...)

global isr_generic_stub
isr_generic_stub:
    cli
    hlt         ; Trava o processador para sempre
    jmp isr_generic_stub