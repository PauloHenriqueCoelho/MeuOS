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

; ... (código existente da GDT) ...

global loadPageDirectory
global enablePaging

loadPageDirectory:
    push ebp
    mov ebp, esp
    mov eax, [esp+8]
    mov cr3, eax
    mov esp, ebp
    pop ebp
    ret

enablePaging:
    push ebp
    mov ebp, esp
    mov eax, cr0
    or eax, 0x80000000 ; Set bit 31 (PG)
    mov cr0, eax
    mov esp, ebp
    pop ebp
    ret