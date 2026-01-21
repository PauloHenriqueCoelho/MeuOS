#include "../include/multiboot.h" // <--- ESSENCIAL PARA LER O VÍDEO
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
#include "../include/memory.h"
#include "../include/paging.h"
#include "../include/task.h"

int dragging_window_id = WIN_ID_NONE;
int drag_offset_x = 0, drag_offset_y = 0;

// Cores em Hex (ARGB) para 32-bit
#define COLOR_GRAY 0xFFC0C0C0
#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xFFFFFFFF

void draw_clock() {
    uint32_t seconds = get_tick() / 100;
    int m = (seconds / 60) % 60; int s = seconds % 60;
    char time_str[16];
    time_str[0] = (m/10)+'0'; time_str[1]=(m%10)+'0'; time_str[2]=':';
    time_str[3] = (s/10)+'0'; time_str[4]=(s%10)+'0'; time_str[5]='\0';
    
    // Atualizado para ficar no canto inferior direito de 800x600
    // Fundo cinza cobrindo o texto anterior
    gfx_fill_rect(740, 580, 50, 15, COLOR_GRAY); 
    
    // Desenhando caractere por caractere (vga_print não funciona bem em modo gráfico puro)
    int x = 745;
    for(int i=0; time_str[i]; i++) {
        gfx_draw_char(x, 582, time_str[i], COLOR_BLACK);
        x += 8;
    }
}

// A main agora recebe argumentos do Bootloader!
void kernel_main(unsigned long magic, unsigned long addr) {
    int last_mx = 0, last_my = 0;
    
    // 1. Inicializa GDT e IDT primeiro (base do sistema)
    init_gdt(); 
    init_idt();

    // 2. Inicializa Vídeo via Multiboot (VBE)
    // Verifica se o Bootloader passou o número mágico correto
    if (magic == 0x2BADB002) {
        multiboot_info_t* mboot = (multiboot_info_t*)addr;
        
        // Verifica se a flag de Framebuffer (bit 12) está ativa
        if (mboot->flags & (1 << 12)) {
            vga_init_from_multiboot(
                mboot->framebuffer_addr,
                mboot->framebuffer_width,
                mboot->framebuffer_height,
                mboot->framebuffer_pitch,
                mboot->framebuffer_bpp
            );
        }
    } else {
        // Se falhar, você não terá vídeo, então é bom ter cuidado aqui
        // Por enquanto vamos deixar passar, mas a tela ficará preta.
    }

    // 3. Inicializa o resto
    keyboard_init(); 
    mouse_init(); 
    wm_init(); 
    init_timer(100);
    pmm_init(128 * 1024 * 1024);
    paging_init();
    task_init();

    __asm__ volatile("sti");

    // Limpa a tela com cinza de fundo (Desktop)
    gfx_clear_screen(0xFF303080); // Um azul escuro bonito para o fundo

    refresh_screen();
    
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
        else if (click) {
            int title_hit = wm_check_title_collision(mx, my);
            int body_hit = wm_check_body_collision(mx, my);
            int close_hit = wm_check_close_collision(mx, my);

            if (close_hit != WIN_ID_NONE || title_hit != WIN_ID_NONE || body_hit != WIN_ID_NONE) {
                
                // Botão fechar
                if (close_hit != WIN_ID_NONE) {
                    wm_close(close_hit);
                    if (current_app_id == close_hit) current_app_id = WIN_ID_NONE;
                    redraw_needed = 1;
                    while(mouse_get_status() & 1); 
                } 
                // Barra de título (Arrastar)
                else if (title_hit != WIN_ID_NONE) {
                    wm_focus(title_hit); 
                    dragging_window_id = title_hit;
                    Window* w = wm_get(title_hit);
                    drag_offset_x = mx - w->x;
                    drag_offset_y = my - w->y;
                    redraw_needed = 1;
                }
                // Corpo da janela
                else if (body_hit != WIN_ID_NONE) {
                    if (current_app_id != body_hit) {
                        wm_focus(body_hit);
                        redraw_needed = 1;
                    }
                }
            } 
            // Ícones do Desktop
            else {
                if (mx >= 20 && mx <= 52) {
                    // Atualizei a cor para COLOR_GRAY (VBE)
                    if (my >= 20 && my <= 52) { // Shell
                        wm_create(TYPE_SHELL, "Terminal", 100, 100, 300, 180, COLOR_GRAY);
                        redraw_needed = 1;
                    }
                    else if (my >= 80 && my <= 112) { // Calc
                        wm_create(TYPE_CALC, "Calc", 150, 150, 160, 220, COLOR_GRAY);
                        redraw_needed = 1;
                    }
                    while(mouse_get_status() & 1);
                }
            }
        }
        
        // Teclado
        if (current_app_id != WIN_ID_NONE && wm_get(current_app_id)->type == TYPE_SHELL) {
             char c = keyboard_get_key();
             if (c != 0) {
                 shell_handle_key(c);
                 redraw_needed = 1;
             }
        }

        if (redraw_needed) {
            refresh_screen();
            draw_mouse_cursor();
            redraw_needed = 0;
        }
        
        __asm__ volatile("hlt");
    }
}