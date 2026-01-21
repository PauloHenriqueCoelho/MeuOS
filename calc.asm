BITS 32
ORG 0x400000

section .text
    global _start

_start:
    ; --- 1. Criar a Janela (Syscall 12) ---
    mov eax, 12          ; os_window_create
    mov ebx, title_str   ; Ponteiro para o título
    mov ecx, 0x00640032  ; X=100, Y=50 (empacotado)
    mov edx, 0x00960078  ; W=150, H=120 (empacotado)
    int 0x80
    mov [win_id], eax    ; Salva o ID real retornado pelo Kernel

    ; --- 2. Criar Botão "1" (ID 101) ---
    mov eax, 10          ; os_create_button
    mov ebx, [win_id]    ; Usa o ID retornado
    mov ecx, 0x000A0028  ; X=10, Y=40
    mov edi, 30          ; Largura
    mov ebp, 25          ; Altura
    mov esi, 101         ; ID do botão
    mov edx, btn_1_text  ; Texto "1"
    int 0x80

    ; --- 3. Criar Botão "2" (ID 102) ---
    mov eax, 10
    mov ebx, [win_id]
    mov ecx, 0x00320028  ; X=50, Y=40
    mov edi, 30
    mov ebp, 25
    mov esi, 102
    mov edx, btn_2_text
    int 0x80

loop_eventos:
    mov eax, 11          ; os_wait_event
    mov ebx, [win_id]
    int 0x80

    cmp eax, 101
    je clicou_1
    cmp eax, 102
    je clicou_2
    jmp loop_eventos

clicou_1:
    mov ebx, msg_1
    mov eax, 1           ; os_print
    int 0x80
    jmp loop_eventos

clicou_2:
    mov ebx, msg_2
    mov eax, 1           ; os_print
    int 0x80
    jmp loop_eventos

section .data
    win_id       dd 0
    title_str    db "Calculadora", 0
    btn_1_text   db "1", 0
    btn_2_text   db "2", 0
    msg_1        db "Clicou no 1", 10, 0
    msg_2        db "Clicou no 2", 10, 0