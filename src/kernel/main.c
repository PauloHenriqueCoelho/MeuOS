#include "../include/multiboot.h"
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
#include "../include/editor.h" // <--- ADICIONE ESTA LINHA

int dragging_window_id = WIN_ID_NONE;
int drag_offset_x = 0, drag_offset_y = 0;

void draw_clock() {
    // Relógio no canto inferior direito
    gfx_fill_rect(964, 748, 50, 15, 0xFFC0C0C0); 
    uint32_t seconds = get_tick() / 100;
    int m = (seconds / 60) % 60; int s = seconds % 60;
    char time_str[16];
    time_str[0] = (m/10)+'0'; time_str[1]=(m%10)+'0'; time_str[2]=':';
    time_str[3] = (s/10)+'0'; time_str[4]=(s%10)+'0'; time_str[5]='\0';
    
    int x = 969;
    for(int i=0; time_str[i]; i++) {
        gfx_draw_char(x, 750, time_str[i], 0xFF000000);
        x += 8;
    }
}

void kernel_main(unsigned long magic, unsigned long addr) {
    init_gdt(); 
    init_idt();

    uint32_t fb_addr = 0;
    uint32_t fb_size = 0;

    if (magic == 0x2BADB002) {
        multiboot_info_t* mboot = (multiboot_info_t*)addr;
        if (mboot->flags & (1 << 12)) {
            vga_init_from_multiboot(
                mboot->framebuffer_addr,
                mboot->framebuffer_width,
                mboot->framebuffer_height,
                mboot->framebuffer_pitch,
                mboot->framebuffer_bpp
            );
            
            // SALVA OS DADOS PARA O MAPEAMENTO
            fb_addr = mboot->framebuffer_addr;
            // Calcula tamanho total (Largura * Altura * BytesPorPixel)
            fb_size = mboot->framebuffer_width * mboot->framebuffer_height * (mboot->framebuffer_bpp / 8);
        }
    }

    keyboard_init(); 
    mouse_init(); 
    wm_init(); 
    task_init();
    init_timer(100);
    pmm_init(128 * 1024 * 1024);
        paging_init();
        if (fb_addr > 0 && fb_size > 0) {
        // Adiciona uma margem de segurança
        for (uint32_t i = 0; i < fb_size + 0x4000; i += 0x1000) {
            paging_map_page(fb_addr + i, fb_addr + i, kernel_page_directory);
        }
    }

    enablePaging();


    __asm__ volatile("sti");

    // Limpa a tela inicial
    gfx_clear_screen(0xFF303080);
    refresh_screen();
    
    // Inicia o mouse
    draw_mouse_cursor();

    int last_mx = mouse_get_x();
    int last_my = mouse_get_y();
    
    while(1) {
        draw_clock();
        
        int mx = mouse_get_x();
        int my = mouse_get_y();
        int click = (mouse_get_status() & 1);
        int redraw_needed = 0;

        // --- CORREÇÃO DE PERFORMANCE ---
        // Se o mouse moveu, NÃO redesenhe tudo. 
        // Apenas atualize o mouse (que agora usa Save-Under).
        if (mx != last_mx || my != last_my) {
            draw_mouse_cursor(); // <--- Rápido e sem rastro!
            last_mx = mx;
            last_my = my;
        }

        Window* active_win = wm_get(current_app_id);
    if (active_win) {
        char c = keyboard_get_key();
        if (c != 0) {
            // Se a janela focada for o Shell, manda pra lá
            if (active_win->type == TYPE_SHELL) {
                shell_handle_key(c);
                redraw_needed = 1;
            }
            // SE FOR O EDITOR, MANDA PARA O HANDLER DO EDITOR
            else if (active_win->type == TYPE_EDITOR) {
                editor_handle_key(c); // Esta função atualiza o buffer da janela
                redraw_needed = 1;    // Avisa que a janela mudou e precisa ser redesenhada
            }
        }
    }

        // Se estiver arrastando janela, AÍ SIM precisa de redraw total
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
        else if (click) {
            int title_hit = wm_check_title_collision(mx, my);
            int body_hit = wm_check_body_collision(mx, my);
            int close_hit = wm_check_close_collision(mx, my);

            if (close_hit != WIN_ID_NONE || title_hit != WIN_ID_NONE || body_hit != WIN_ID_NONE) {
                if (close_hit != WIN_ID_NONE) {
                    wm_close(close_hit);
                    if (current_app_id == close_hit) current_app_id = WIN_ID_NONE;
                    redraw_needed = 1;
                    while(mouse_get_status() & 1); 
                } 
                else if (title_hit != WIN_ID_NONE) {
                    wm_focus(title_hit); 
                    dragging_window_id = title_hit;
                    Window* w = wm_get(title_hit);
                    drag_offset_x = mx - w->x;
                    drag_offset_y = my - w->y;
                    redraw_needed = 1;
                }
                else if (body_hit != WIN_ID_NONE) {
                    if (current_app_id != body_hit) {
                        wm_focus(body_hit);
                        redraw_needed = 1;
                    }
                }
            } 
            else {
                // Ícones
                if (mx >= 20 && mx <= 52 && my >= 20 && my <= 52) { 
                    wm_create(TYPE_SHELL, "Terminal", 100, 100, 300, 180, 0xFFC0C0C0);
                    redraw_needed = 1;
                    while(mouse_get_status() & 1);
                }
                else if (mx >= 20 && mx <= 52 && my >= 80 && my <= 112) {
                    wm_create(TYPE_CALC, "Calc", 150, 150, 160, 220, 0xFFC0C0C0);
                    redraw_needed = 1;
                    while(mouse_get_status() & 1);
                }
            }
        }

        if (windows_changed) {
            redraw_needed = 1;
            windows_changed = 0; // Reseta o sinal
        }
        
        // Shell
        if (current_app_id != WIN_ID_NONE && wm_get(current_app_id)->type == TYPE_SHELL) {
             char c = keyboard_get_key();
             if (c != 0) {
                 shell_handle_key(c);
                 redraw_needed = 1;
             }
        }

        // Só faz refresh pesado se realmente necessário (janelas)
        if (redraw_needed) {
            refresh_screen();
            draw_mouse_cursor(); // Garante que o mouse fique no topo
            redraw_needed = 0;
        }
        
        __asm__ volatile("hlt");
    }
}