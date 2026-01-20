global gdt_flush

gdt_flush:
    mov eax, [esp + 4] ; Pega o ponteiro da GDT passado como argumento
    lgdt [eax]         ; Carrega a nova GDT

    ; Atualiza os registradores de segmento de DADOS (Offset 0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Pulo distante (Far Jump) para atualizar o segmento de CÓDIGO (CS)
    ; 0x08 é o offset do Code Segment
    jmp 0x08:.flush   

.flush:
    ret
