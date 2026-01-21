#include "../include/api.h"
#include "../include/window.h"

// Estrutura para aceder aos registadores salvos pelo pusha
typedef struct {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
} registers_t;

void syscall_handler(registers_t* regs) {
    // O registador EAX define qual função chamar
    switch (regs->eax) {
        
        case 1: // Syscall: os_print
            // O ponteiro da mensagem estará em EBX
            os_print((char*)regs->ebx);
            break;

        case 2: // Syscall: os_msgbox
            // EBX = Título, ECX = Mensagem
            os_msgbox((char*)regs->ebx, (char*)regs->ecx);
            break;

        case 3: // Syscall: os_reboot
            os_reboot();
            break;

        default:
            os_print("Erro: Syscall desconhecida!\n");
            break;
    }
}