[BITS 32]
global switch_to_task
global task_start_stub ; <--- Nova função global

; Função de troca de contexto
switch_to_task:
    mov eax, [esp + 4]    ; eax = &tasks[old].esp
    mov ecx, [esp + 8]    ; ecx = tasks[new].esp

    pusha                 ; Salva registradores da tarefa antiga
    mov [eax], esp        ; Guarda o ESP antigo

    mov esp, ecx          ; CARREGA O ESP DA NOVA TAREFA
    
    popa                  ; Restaura registradores (incluindo EAX com o entry_point)
    ret                   ; Pula para o EIP salvo (que será o task_start_stub)

; --- O SEGREDINHO ---
; Toda tarefa nova vai começar AQUI primeiro.
task_start_stub:
    sti         ; 1. LIGA AS INTERRUPÇÕES (Timer volta a funcionar!)
    call eax    ; 2. Chama a função main do programa (Endereço estava em EAX)
    
    ; Se o programa terminar (retornar do main), travamos aqui
    ; (Futuramente aqui chamaremos sys_exit)
    jmp $