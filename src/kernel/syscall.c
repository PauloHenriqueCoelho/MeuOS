#include "../include/api.h"
#include "../include/window.h"
#include "../include/utils.h" 
#include "../include/vga.h"
#include "../include/idt.h" // <--- CRUCIAL: Traz a definição correta de registers_t

// Removemos a struct local errada que estava aqui!

void syscall_handler(registers_t* regs) {
    // Agora regs->eax realmente aponta para EAX (o número da syscall)
    
    switch (regs->eax) {
        
        case 1: // os_print (EAX=1)
            // EBX contém o ponteiro da string
            os_print((char*)regs->ebx);
            break;

        case 12: { // os_window_create (EAX=12)
            char* title = (char*)regs->ebx;
            int x = (regs->ecx >> 16) & 0xFFFF;
            int y = regs->ecx & 0xFFFF;
            int w = (regs->edx >> 16) & 0xFFFF;
            int h = regs->edx & 0xFFFF;
            
            // Cria a janela e retorna o ID em EAX
            regs->eax = wm_create(2, title, x, y, w, h, 0xFFC0C0C0);
            break;
        }

        case 10: { // os_create_button (EAX=10)
            Window* w = wm_get(regs->ebx); // Win ID
            if (w && w->button_count < 20) { 
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
                
                // Desenha botão
                gfx_draw_button(b->label, w->x + b->x, w->y + b->y, b->w, b->h, 0xFFE0E0E0);
            }
            break;
        }

        case 11: // os_wait_event (EAX=11)
            regs->eax = wm_wait_click(regs->ebx); 
            break;

        case 13: // os_update_button (EAX=13)
            wm_update_button(regs->ebx, regs->ecx, (char*)regs->edx);
            break;

        default:
            // Opcional: Avisar se receber algo estranho
            // os_print("[SYSCALL] Desconhecida: ");
            // debug_print_hex(regs->eax);
            break;
    }
}