#include "../include/api.h"
#include "../include/vga.h"
#include "../include/io.h"
#include "../include/fs.h"
#include "../include/keyboard.h" 
#include "../include/mouse.h"
#include "../include/window.h"
#include "../include/utils.h"
#include "../include/task.h"

// --- CONSOLE ---
void os_print(const char* message) { 
    // Usa branco como padrão
    vga_print_color(message, COLOR_WHITE); 
}

void os_print_color(const char* message, uint8_t fg, uint8_t bg) {
    // ADAPTAÇÃO: O sistema antigo usava IDs de cor VGA (0-15).
    // O sistema novo usa True Color (32-bit).
    // Vamos fazer um "de-para" simples ou apenas usar branco por enquanto para não quebrar.
    
    uint32_t color32 = COLOR_WHITE;
    if (fg == 1) color32 = COLOR_BLUE;
    else if (fg == 4) color32 = COLOR_RED;
    else if (fg == 2) color32 = COLOR_GREEN;
    else if (fg == 0) color32 = COLOR_BLACK;
    
    vga_print_color(message, color32); 
    // bg é ignorado no console transparente, ou você pode implementar fundo no texto se quiser
}

void os_clear_screen() { 
    vga_clear(); 
}

// --- INPUT ---
char os_wait_for_key() {
    while(1) {
        char c = keyboard_get_key();
        if (c != 0) return c;
        __asm__ volatile("hlt");
    }
}

// --- FILESYSTEM ---
int os_file_create(char* name, char* content) { return fs_create(name, content); }
int os_file_read(char* name, char* buffer) { return fs_read_to_buffer(name, buffer); }
int os_file_delete(char* name) { return fs_delete(name); }
void os_file_list() { fs_list(); }

// --- GUI ---
void os_create_window(char* title, int x, int y, int w, int h, uint8_t color_idx) {
    // Converte cor VGA antiga para 32-bit
    uint32_t body_color = COLOR_GRAY; 
    if (color_idx == 1) body_color = COLOR_BLUE;
    if (color_idx == 7) body_color = COLOR_GRAY;
    
    gfx_draw_window(title, x, y, w, h, body_color);
}

void os_set_cursor(int x, int y) {
    vga_set_cursor(x, y);
}

int os_window_close_clicked(int win_x, int win_y, int win_w, int mouse_x, int mouse_y) {
    // Ajustado para o novo tamanho do botão na vga.c (14x12 px, offset 18px)
    int btn_x = win_x + win_w - 18;
    int btn_y = win_y + 4;
    
    if (mouse_x >= btn_x && mouse_x <= (btn_x + 14) &&
        mouse_y >= btn_y && mouse_y <= (btn_y + 12)) {
        return 1;
    }
    return 0;
}

void os_wait_interaction() {
    while(mouse_get_status() & 1); 
    while(1) {
        if (mouse_get_status() & 1) {
            while(mouse_get_status() & 1);
            return;
        }
        char c = keyboard_get_key();
        if (c == '\n') return;
        __asm__ volatile("hlt");
    }
}

void os_reboot() { 
    uint8_t good = 0x02;
    while (good & 0x02) good = inb(0x64);
    outb(0x64, 0xFE);
    __asm__ volatile("int3"); 
}

int os_window_title_clicked(int win_x, int win_y, int win_w, int mouse_x, int mouse_y) {
    // Barra de título agora tem 20px de altura
    if (mouse_x >= win_x && mouse_x <= (win_x + win_w - 20) &&
        mouse_y >= win_y && mouse_y <= (win_y + 20)) {
        return 1;
    }
    return 0;
}

int os_file_exists(char* name) { return fs_exists(name); }

void os_msgbox(char* title, char* text) {
    static int offset = 0;
    offset += 20;
    if (offset > 100) offset = 0;

    // Type Text = 2, Cor Gray = 7
    int id = wm_create(TYPE_TEXT, title, 150 + offset, 150 + offset, 300, 150, 7); 
    
    if (id != -1) {
        Window* w = wm_get(id);
        strcpy(w->buffer, text);
    }
}