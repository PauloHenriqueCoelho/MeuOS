#include "../include/shell.h"
#include "../include/vga.h"
#include "../include/io.h"
#include "../include/utils.h"
#include "../include/window.h" 
#include "../include/api.h"

// Buffer interno do Shell
char shell_buffer[256];
int shell_cursor_pos = 0;

void shell_init() {
    for(int i=0; i<256; i++) shell_buffer[i] = 0;
    shell_cursor_pos = 0;
}

void shell_draw() {
    Window* w = wm_get(WIN_ID_SHELL);
    if (!w) return;

    os_create_window("Terminal", w->x, w->y, w->w, w->h, 0);

    int text_x = w->x + 10;
    int text_y = w->y + 30;

    vga_set_cursor(text_x, text_y);
    vga_set_color(15, 0); 
    vga_print("> ");
    vga_print(shell_buffer);
}

void shell_handle_key(char c) {
    if (c == 0) return;

    if (c == '\n') {
        shell_execute(shell_buffer);
        
        shell_cursor_pos = 0;
        for(int i=0; i<256; i++) shell_buffer[i] = 0;
        
        shell_draw();
    }
    else if (c == '\b') {
        if (shell_cursor_pos > 0) {
            shell_cursor_pos--;
            shell_buffer[shell_cursor_pos] = 0;
            shell_draw(); 
        }
    }
    else {
        if (shell_cursor_pos < 254) {
            shell_buffer[shell_cursor_pos] = c;
            shell_cursor_pos++;
            shell_buffer[shell_cursor_pos] = 0; 
            
            Window* w = wm_get(WIN_ID_SHELL);
            int prompt_size = 2 * 8; 
            int text_offset = (shell_cursor_pos - 1) * 8;
            vga_set_cursor(w->x + 10 + prompt_size + text_offset, w->y + 30);
            vga_putchar(c);
        }
    }
}

// Helper para pegar argumento (ex: "cat arquivo.txt" -> retorna "arquivo.txt")
char* get_argument(char* command) {
    while (*command && *command != ' ') command++; // Vai até o espaço
    if (*command == ' ') command++; // Pula o espaço
    return command;
}

void shell_execute(char* command) {
    // Comando: AJUDA
    if (strcmp(command, "ajuda") == 0) {
        os_create_window("Ajuda", 100, 100, 200, 120, 1); 
        vga_set_cursor(110, 130); vga_print("ls     - Listar arqs");
        vga_set_cursor(110, 140); vga_print("touch  - Criar arq");
        vga_set_cursor(110, 150); vga_print("cat    - Ler arq");
        vga_set_cursor(110, 160); vga_print("rm     - Apagar arq");
        vga_set_cursor(110, 170); vga_print("reboot - Reiniciar");
        
        // Delay simples para ler
        for(volatile int i=0; i<50000000; i++);
        shell_draw(); 
    }
    // Comando: LIMPAR
    else if (strcmp(command, "limpar") == 0) {
        shell_draw();
    }
    // Comando: REBOOT
    else if (strcmp(command, "reboot") == 0) {
        os_reboot();
    }
    // Comando: LS (Listar Arquivos)
    else if (strcmp(command, "ls") == 0) {
        os_create_window("Arquivos", 80, 80, 200, 150, 1);
        vga_set_cursor(90, 100);
        
        // Como o fs_list imprime direto, ele pode sair da janela.
        // O ideal seria o fs_list retornar uma string, mas vamos usar direto por enquanto.
        os_file_list(); 
        
        for(volatile int i=0; i<80000000; i++);
        shell_draw();
    }
    // Comando: TOUCH (Criar arquivo vazio)
    else if (strncmp(command, "touch ", 6) == 0) {
        char* filename = get_argument(command);
        if (*filename) {
            os_file_create(filename, "Vazio"); // Cria com texto dummy
            os_print(" Criado!");
        }
    }
    // Comando: RM (Deletar)
    else if (strncmp(command, "rm ", 3) == 0) {
        char* filename = get_argument(command);
        if (*filename) {
            os_file_delete(filename);
        }
    }
    // Comando: CAT (Ler arquivo)
    else if (strncmp(command, "cat ", 4) == 0) {
        char* filename = get_argument(command);
        char content[1024]; // Buffer temporário
        
        // Limpa buffer
        for(int i=0; i<1024; i++) content[i] = 0;

        if (os_file_read(filename, content)) {
            // Mostra o conteúdo numa janela popup
            os_create_window(filename, 60, 60, 250, 150, 15); // Janela Branca
            vga_set_color(0, 15); // Texto preto
            vga_set_cursor(70, 90);
            vga_print(content);
            vga_set_color(15, 0); // Restaura cor branca
            
            for(volatile int i=0; i<100000000; i++); // Espera ler
            shell_draw();
        } else {
            // Se falhar (arquivo não existe), o cursor do shell apenas pisca
        }
    }
}