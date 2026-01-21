#include "../include/editor.h"
#include "../include/api.h"
#include "../include/utils.h"
#include "../include/window.h"

char current_edit_file[32];
int editor_window_id = -1;

void editor_open(char* filename) {
    strcpy(current_edit_file, filename);
    
    // Cria uma janela branca para o editor
    editor_window_id = wm_create(TYPE_EDITOR, filename, 80, 80, 400, 250, 0xFFFFFFFF);
    
    if (editor_window_id != -1) {
        Window* w = wm_get(editor_window_id);
        // Tenta carregar o conteúdo se o arquivo já existir
        if (os_file_exists(filename)) {
            os_file_read(filename, w->buffer);
        } else {
            w->buffer[0] = '\0'; // Novo arquivo
        }
        os_print(" Editor aberto!");
    }
}

void editor_handle_key(char c) {
    if (editor_window_id == -1) return;
    Window* w = wm_get(editor_window_id);
    int len = strlen(w->buffer);

    if (c == '\n') { // Enter
        if (len < 1022) {
            w->buffer[len] = '\n';
            w->buffer[len+1] = '\0';
        }
    }
    else if (c == '\b') { // Backspace
        if (len > 0) w->buffer[len-1] = '\0';
    }
    else if (c == '\t') { // Tab (para identar o script)
        strcat(w->buffer, "    ");
    }
    else if (c >= 32 && c <= 126) { // Caracteres imprimíveis
        if (len < 1023) {
            w->buffer[len] = c;
            w->buffer[len+1] = '\0';
        }
    }
    
    // Comando especial: Se apertar 'ESC', salva e fecha (ou use uma tecla de função)
    // Nota: Você precisará mapear o ESC no seu driver de teclado
}

// Função para salvar o que está no buffer
void editor_save() {
    if (editor_window_id != -1) {
        Window* w = wm_get(editor_window_id);
        os_file_create(current_edit_file, w->buffer);
        os_msgbox("Editor", "Arquivo salvo com sucesso!");
    }
}