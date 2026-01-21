#include "../include/vga.h"
#include "../include/io.h"
#include "../include/utils.h"
#include "../include/shell.h"
#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/keyboard.h"
#include "../include/mouse.h"
#include "../include/programs.h"
#include "../include/api.h"
#include "../include/window.h"
#include "../include/timer.h"
#include "../include/memory.h" // Adicione este include
#include "../include/paging.h"  // <--- ADICIONE ESTA LINHA AQUI

int dragging_window_id = WIN_ID_NONE;
int drag_offset_x = 0, drag_offset_y = 0;

void draw_clock() {
    uint32_t seconds = get_tick() / 100;
    int m = (seconds / 60) % 60; int s = seconds % 60;
    char time_str[16];
    time_str[0] = (m/10)+'0'; time_str[1]=(m%10)+'0'; time_str[2]=':';
    time_str[3] = (s/10)+'0'; time_str[4]=(s%10)+'0'; time_str[5]='\0';
    gfx_fill_rect(270, 187, 45, 10, 7); 
    vga_set_cursor(275, 189); vga_print(time_str);
}



void kernel_main() {
    int last_mx = 0, last_my = 0; // Adicione no topo da kernel_main
    init_gdt(); video_init(); init_idt(); keyboard_init(); mouse_init(); wm_init(); init_timer(100);
    pmm_init(128 * 1024 * 1024);
    paging_init();
    __asm__ volatile("sti");

    refresh_screen();
    
    // Inicia Shell Padrão
    wm_create(TYPE_SHELL, "Terminal", 10, 10, 300, 180, 0);
    shell_init();
    current_app_id = 0; // Foco no shell

    while(1) {
        draw_clock();
        int mx = mouse_get_x();
        int my = mouse_get_y();
        int click = (mouse_get_status() & 1);
        int redraw_needed = 0;

        if (mx != last_mx || my != last_my) {
            draw_mouse_cursor();
            last_mx = mx;
            last_my = my;
        }

        // --- ARRASTE ---
        if (dragging_window_id != WIN_ID_NONE) {
            if (click) {
                Window* w = wm_get(dragging_window_id);
                if (w) {
                    w->x = mx - drag_offset_x;
                    w->y = my - drag_offset_y;
                    redraw_needed = 1;
                }
            } else {
                dragging_window_id = WIN_ID_NONE;
            }
        }
        // --- CLIQUES ---
        // --- CLIQUES ---
        else if (click) {
            int title_hit = wm_check_title_collision(mx, my);
            int body_hit = wm_check_body_collision(mx, my);
            int close_hit = wm_check_close_collision(mx, my);

            if (close_hit != WIN_ID_NONE || title_hit != WIN_ID_NONE || body_hit != WIN_ID_NONE) {
                
                // Prioridade 1: Botão fechar (Clique Único - Mantém o while)
                if (close_hit != WIN_ID_NONE) {
                    wm_close(close_hit);
                    if (current_app_id == close_hit) current_app_id = WIN_ID_NONE;
                    redraw_needed = 1;
                    while(mouse_get_status() & 1); // Debounce para não fechar várias coisas
                } 
                // Prioridade 2: Arrastar pela barra de título (NÃO pode ter while aqui)
                else if (title_hit != WIN_ID_NONE) {
                    wm_focus(title_hit); 
                    dragging_window_id = title_hit;
                    Window* w = wm_get(title_hit);
                    drag_offset_x = mx - w->x;
                    drag_offset_y = my - w->y;
                    redraw_needed = 1;
                    // Note que removi o while aqui para permitir o movimento contínuo
                }
                // Prioridade 3: Clicou no corpo
                else if (body_hit != WIN_ID_NONE) {
                    if (current_app_id != body_hit) {
                        wm_focus(body_hit);
                        redraw_needed = 1;
                    }
                    
                    Window* w = wm_get(body_hit);
                    if (w->type == TYPE_CALC) {
                        calculator_click(mx, my);
                        while(mouse_get_status() & 1); // Botão da calc é clique único
                    }
                }
            } 
            // 2. Ícones do Desktop (Clique Único - Mantém o while)
            else {
                if (mx >= 20 && mx <= 52) {
                    if (my >= 20 && my <= 52) { // Shell
                        wm_create(TYPE_SHELL, "Terminal", 10, 10, 300, 180, 0);
                        redraw_needed = 1;
                    }
                    else if (my >= 80 && my <= 112) { // Calc
                        wm_create(TYPE_CALC, "Calc", 100, 50, 120, 150, 7);
                        redraw_needed = 1;
                    }
                    while(mouse_get_status() & 1); // Debounce para ícones
                }
            }
        }
        // Teclado só vai para o Shell se ele estiver focado
       if (current_app_id != WIN_ID_NONE && wm_get(current_app_id)->type == TYPE_SHELL) {
             char c = keyboard_get_key();
             if (c != 0) {
                 shell_handle_key(c);
                 redraw_needed = 1; // <--- SE DIGITOU, PRECISA REDESENHAR!
             }
        }

        // Se o redraw_needed for 1, a tela será atualizada e a janela nova 
        // aparecerá no topo instantaneamente.
        if (redraw_needed) {
            refresh_screen();
            draw_mouse_cursor(); // Redesenha o mouse por cima do novo cenário
            redraw_needed = 0; // Reseta para não pesar a CPU
        }
        
        __asm__ volatile("hlt");
    }
}