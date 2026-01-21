[BITS 32]
global switch_to_task

switch_to_task:
    mov eax, [esp + 4]    ; eax = &tasks[old].esp
    mov ecx, [esp + 8]    ; ecx = tasks[new].esp

    pusha                 ; Salva tudo da tarefa atual
    mov [eax], esp        ; Salva o ESP atual na estrutura da tarefa

    mov esp, ecx          ; TROCA PARA O ESP DA NOVA TAREFA
    
    popa                  ; Restaura tudo da nova tarefa
    ret                   ; Pula para o entry_point salvo na pilha!