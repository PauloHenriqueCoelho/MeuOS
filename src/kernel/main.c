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
#include "../include/timer.h" // <--- Novo include

// ID da janela que está focada (recebe teclado)
int current_app_id = WIN_ID_NONE; 

// ID da janela sendo arrastada
int dragging_window_id = WIN_ID_NONE;
int drag_offset_x = 0;
int drag_offset_y = 0;

// Função central para redesenhar a tela

void draw_clock() {
    // Pega o tempo desde o boot (em ticks). 
    // Assumindo timer a 100Hz -> 100 ticks = 1 segundo
    uint32_t seconds = get_tick() / 100;
    
    // Converte para minutos:segundos
    int m = (seconds / 60) % 60;
    int s = seconds % 60;
    
    char time_str[16];
    // Formatação manual rápida (Ex: "05:09")
    time_str[0] = (m / 10) + '0';
    time_str[1] = (m % 10) + '0';
    time_str[2] = ':';
    time_str[3] = (s / 10) + '0';
    time_str[4] = (s % 10) + '0';
    time_str[5] = '\0';
    
    // Desenha no canto direito da barra cinza
    // Barra está em Y=185. Vamos por em X=270
    // Primeiro limpa o fundo (cinza)
    gfx_fill_rect(270, 187, 45, 10, 7); 
    
    vga_set_cursor(275, 189);
    vga_print(time_str);
}

void refresh_screen() {
    // 1. Desenha o Desktop (Fundo e Ícones)
    desktop_draw();
    
    // 2. Percorre as janelas ativas e desenha
    // (Poderíamos fazer um loop no WM, mas vamos fazer manual por enquanto para controlar ordem)
    
    // Se a calculadora estiver aberta
    Window* w_calc = wm_get(WIN_ID_CALC);
    if (w_calc->active) {
        calculator_draw();
    }

    // Se o shell estiver aberto
    Window* w_shell = wm_get(WIN_ID_SHELL);
    if (w_shell->active) {
        shell_draw(); // O shell agora sabe se desenhar!
    }
}

void kernel_main() {
    init_gdt(); video_init(); init_idt(); keyboard_init(); mouse_init(); wm_init();
    init_timer(100);
    __asm__ volatile("sti");

    refresh_screen();

    while(1) {

        draw_clock();
        
        int mx = mouse_get_x();
        int my = mouse_get_y();
        int click = (mouse_get_status() & 1);
        int redraw_needed = 0;

        // --- 1. LÓGICA DE ARRASTAR JANELA ---
        if (dragging_window_id != WIN_ID_NONE) {
            if (click) {
                // Continua arrastando
                Window* w = wm_get(dragging_window_id);
                w->x = mx - drag_offset_x;
                w->y = my - drag_offset_y;
                redraw_needed = 1;
            } else {
                // Soltou o mouse
                dragging_window_id = WIN_ID_NONE;
            }
        }
        
        // --- 2. LÓGICA DE CLIQUE NOVO ---
        else if (click) {
            // A. Verifica clique no Botão FECHAR (Genérico)
            int close_id = wm_check_close_collision(mx, my);
            if (close_id != WIN_ID_NONE) {
                wm_close(close_id);
                if (current_app_id == close_id) current_app_id = WIN_ID_NONE;
                
                redraw_needed = 1;
                // Espera soltar para não clicar em mais nada
                while(mouse_get_status() & 1); 
            }
            
            // B. Verifica clique na BARRA DE TÍTULO (Inicia Arraste)
            else {
                int title_id = wm_check_title_collision(mx, my);
                if (title_id != WIN_ID_NONE) {
                    dragging_window_id = title_id;
                    current_app_id = title_id; // Dá foco
                    Window* w = wm_get(title_id);
                    drag_offset_x = mx - w->x;
                    drag_offset_y = my - w->y;
                }
                
                // C. Verifica cliques no conteúdo (Botões da Calc, Ícones)
                else {
                    // Calculadora
                    Window* w_calc = wm_get(WIN_ID_CALC);
                    if (w_calc->active && 
                        mx >= w_calc->x && mx <= w_calc->x + w_calc->w &&
                        my >= w_calc->y && my <= w_calc->y + w_calc->h) {
                        
                        current_app_id = WIN_ID_CALC;
                        calculator_click(mx, my);
                        while(mouse_get_status() & 1);
                    }

                    // Shell (Corpo)
                    Window* w_shell = wm_get(WIN_ID_SHELL);
                    if (w_shell->active && 
                        mx >= w_shell->x && mx <= w_shell->x + w_shell->w &&
                        my >= w_shell->y && my <= w_shell->y + w_shell->h) {
                        current_app_id = WIN_ID_SHELL;
                    }

                    // Ícones do Desktop (Só se nenhuma janela estiver por cima "teoricamente")
                    // Simplificação: só checa ícones se clicou no fundo
                    if (current_app_id == WIN_ID_NONE || dragging_window_id == WIN_ID_NONE) {
                         if (mx >= 20 && mx <= 52 && my >= 20 && my <= 52) { // Shell Icon
                            wm_open(WIN_ID_SHELL, "Terminal", 10, 10, 300, 180, 0);
                            current_app_id = WIN_ID_SHELL;
                            shell_init();
                            redraw_needed = 1;
                            while(mouse_get_status() & 1);
                        }
                        if (mx >= 20 && mx <= 52 && my >= 80 && my <= 112) { // Calc Icon
                            wm_open(WIN_ID_CALC, "Calculadora", 100, 50, 120, 150, 7);
                            current_app_id = WIN_ID_CALC;
                            redraw_needed = 1;
                            while(mouse_get_status() & 1);
                        }
                    }
                }
            }
        }

        // --- 3. REDESENHO (Se necessário) ---
        if (redraw_needed) {
            mouse_reset_background();
            refresh_screen();
            
            // --- USO DA NOVA TECNOLOGIA ---
            // Em vez de for(volatile int i...), usamos:
            if (dragging_window_id != WIN_ID_NONE) {
                sleep(2); // Espera 20ms (muito mais suave e estável)
            }
        }

        // --- 4. TECLADO ---
        if (current_app_id == WIN_ID_SHELL) {
             char c = keyboard_get_key();
             if (c != 0) {
                 shell_handle_key(c); // Shell se vira para desenhar a letra
             }
        } else {
            keyboard_get_key(); // Joga fora teclas se não for o shell
        }
        
        __asm__ volatile("hlt");
    }
}