#include "../include/api.h"
#include "../include/window.h"
#include "../include/utils.h" 

typedef struct {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
} registers_t;

void syscall_handler(registers_t* regs) {
    switch (regs->eax) {
        
        case 1: // os_print
            os_print((char*)regs->ebx);
            break;

        case 2: // os_msgbox
            os_msgbox((char*)regs->ebx, (char*)regs->ecx);
            break;

        case 3: // os_reboot
            os_reboot();
            break;
            
        case 10: { // os_create_button
            Window* w = wm_get(regs->ebx);
            if (w && w->button_count < 10) {
                Button* b = &w->buttons[w->button_count++];
                
                b->x = (regs->ecx >> 16) & 0xFFFF; 
                b->y = regs->ecx & 0xFFFF;
                b->w = regs->edi;
                b->h = regs->ebp;
                b->id = regs->esi;
                
                char* src = (char*)regs->edx; 
                int i = 0;
                while(src[i] != '\0' && i < 15) {
                    b->label[i] = src[i];
                    i++;
                }
                b->label[i] = '\0';
            }
            break;
        } // <--- Fechamos o case 10 aqui!

        case 11: // os_wait_event
            regs->eax = wm_wait_click(regs->ebx); 
            break;

        case 12: { // os_window_create
            char* title = (char*)regs->ebx;
            int x = (regs->ecx >> 16) & 0xFFFF;
            int y = regs->ecx & 0xFFFF;
            int w = (regs->edx >> 16) & 0xFFFF;
            int h = regs->edx & 0xFFFF;
            regs->eax = wm_create(TYPE_CALC, title, x, y, w, h, 7);
            break;
        }

        // --- NOVA SYSCALL 13 (Agora no lugar certo) ---
        case 13: // os_update_button
            wm_update_button(regs->ebx, regs->ecx, (char*)regs->edx);
            break;

        default:
            os_print("Erro: Syscall desconhecida!\n"); // Debug opcional
            break;
    }
}