#include "../include/api.h"
#include "../include/utils.h"
#include "../include/window.h" 
#include "../include/vga.h" 

// Função auxiliar
char* skip_spaces(char* str) {
    while (*str == ' ') str++;
    return str;
}

void run_script(char* filename) {
    // 1. PREPARAÇÃO VISUAL: Limpa a tela do Shell para o script rodar limpo
    // Isso continua sendo útil porque o Shell ainda é "texto corrido"
    Window* w = wm_get(WIN_ID_SHELL);
    if (w) {
        gfx_fill_rect(w->x + 2, w->y + 14, w->w - 4, w->h - 16, 0);
        os_set_cursor(w->x + 10, w->y + 30);
    }

    // --- LEITURA DO ARQUIVO ---
    char content[512]; 
    
    os_print(" -> Rodando: "); os_print(filename); os_print("\n");

    if (!os_file_read(filename, content)) {
        os_print(" [Erro] Arquivo nao encontrado.\n");
        return;
    }

    int i = 0;
    char line[128];
    int line_idx = 0;

    // --- INTERPRETADOR ---
    while (content[i] != '\0') {
        if (content[i] != '\n' && line_idx < 127) {
            line[line_idx++] = content[i++];
        } 
        else {
            line[line_idx] = '\0';
            i++; 
            
            char* cmd = skip_spaces(line);
            if (line_idx == 0) continue; 

            // --- COMANDO: PRINT (Escreve no Shell) ---
            if (strncmp(cmd, "print ", 6) == 0) {
                char* msg = cmd + 6;
                os_print(msg);
                os_print("\n");
            }
            
            // --- COMANDO: MSGBOX (NOVO - MUITO MAIS SIMPLES) ---
            // Agora apenas chamamos a API. O Window Manager cuida de desenhar,
            // focar, arrastar e fechar. O script NÃO trava mais aqui.
            else if (strncmp(cmd, "msgbox ", 7) == 0) {
                char* msg = cmd + 7;
                // Chama a função poderosa que criamos no api.c
                os_msgbox("Script Diz", msg);
            }
            
            // --- COMANDO: CLEAR (Limpa o Shell) ---
            else if (strcmp(cmd, "clear") == 0) {
                 if(w) gfx_fill_rect(w->x + 2, w->y + 14, w->w - 4, w->h - 16, 0);
                 if(w) os_set_cursor(w->x + 10, w->y + 30);
            }
            
            // --- COMANDO: WAIT (Pausa simples) ---
            else if (strcmp(cmd, "wait") == 0) {
                os_print(" ...\n");
                // Loop de delay (ainda manual, no futuro podemos usar sleep do timer)
                for(volatile int d=0; d<30000000; d++);
            }
            
            else {
                os_print(" [CMD] "); os_print(cmd); os_print("\n");
            }

            line_idx = 0;
        }
    }
    os_print(" -> Concluido.\n");
}