#include "../include/shell.h"
#include "../include/vga.h"
#include "../include/io.h"
#include "../include/utils.h"
#include "../include/window.h" 
#include "../include/api.h"
#include "../include/script.h"
#include "../include/mouse.h"
#include "../include/fs.h"

// Buffer interno do Shell
char shell_buffer[256];
int shell_cursor_pos = 0;

void shell_init() {
    for(int i=0; i<256; i++) shell_buffer[i] = 0;
    shell_cursor_pos = 0;
}

// Esta é a função que o Linker não estava encontrando
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

char* get_argument(char* command) {
    while (*command && *command != ' ') command++;
    if (*command == ' ') command++;
    return command;
}

void shell_execute(char* command) {
    // AJUDA
    if (strcmp(command, "ajuda") == 0) {
        os_msgbox("Ajuda", "ls: Listar\ntouch: Criar\ncat: Ler\nrm: Apagar\nreboot: Reiniciar");
    }
    // LIMPAR
    else if (strcmp(command, "limpar") == 0) {
        shell_draw();
    }
    // MKFILE
    else if (strncmp(command, "mkfile ", 7) == 0) {
        char* args = command + 7;
        char filename[32];
        char content[256];
        int i = 0;
        while(args[i] && args[i] != ' ' && i < 31) {
            filename[i] = args[i];
            i++;
        }
        filename[i] = 0;
        
        if (args[i] == ' ') {
            char* text = &args[i+1];
            int j=0, k=0;
            while(text[j]) {
                if(text[j] == ';') content[k] = '\n';
                else content[k] = text[j];
                j++; k++;
            }
            content[k] = 0;
            os_file_create(filename, content);
            os_print(" Arquivo salvo!");
        }
    }
    // RUN
    else if (strncmp(command, "run ", 4) == 0) {
        char* filename = get_argument(command);
        run_script(filename);
    }
    // REBOOT
    else if (strcmp(command, "reboot") == 0) {
        os_reboot();
    }
    else if (strncmp(command, "exec ", 5) == 0) {
    char* filename = get_argument(command);
    os_msgbox("Ajuda", "ls: Listar\ntouch: Criar\ncat: Ler\nrm: Apagar\nreboot: Reiniciar");
    os_execute_bin(filename);
}
    // LS
    else if (strcmp(command, "ls") == 0) {
        char list_buffer[512];
        fs_get_list_str(list_buffer);
        int id = wm_create(TYPE_TEXT, "Arquivos", 50, 50, 180, 120, 1);
        if (id != -1) {
            Window* w = wm_get(id);
            strcpy(w->buffer, list_buffer);
            os_print(" Janela aberta.");
        }
    }
    // TOUCH
    else if (strncmp(command, "touch ", 6) == 0) {
        char* filename = get_argument(command);
        if (*filename) os_file_create(filename, "Vazio");
    }
    // RM
    else if (strncmp(command, "rm ", 3) == 0) {
        char* filename = get_argument(command);
        if (*filename) os_file_delete(filename);
    }
    // CAT
    else if (strncmp(command, "cat ", 4) == 0) {
        char* filename = get_argument(command);
        char content[1024];
        if (os_file_read(filename, content)) {
            int id = wm_create(TYPE_TEXT, filename, 70, 70, 250, 150, 15);
            if (id != -1) {
                Window* w = wm_get(id);
                strcpy(w->buffer, content);
            }
        }
    }
    // MSG
    else if (strncmp(command, "msg ", 4) == 0) {
        os_msgbox("Mensagem", command + 4);
    }
}