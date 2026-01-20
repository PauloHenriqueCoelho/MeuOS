#include "../include/api.h"
#include "../include/vga.h"
#include "../include/io.h"
#include "../include/fs.h"
#include "../include/keyboard.h" 

// --- CONSOLE ---
void os_print(const char* message) { 
    vga_print(message); 
}

void os_print_color(const char* message, uint8_t fg, uint8_t bg) {
    vga_set_color(fg, bg); 
    vga_print(message); 
    vga_set_color(15, 1); // Restaura para branco sobre azul (Padrão do novo driver)
}

void os_clear_screen() { 
    vga_clear(); 
}

// --- INPUT (VIA INTERRUPÇÃO) ---
char os_wait_for_key() {
    while(1) {
        char c = keyboard_get_key();
        if (c != 0) return c;
        __asm__ volatile("hlt");
    }
}

// --- FILESYSTEM ---
int os_file_create(char* name, char* content) { 
    return fs_create(name, content); 
}

int os_file_read(char* name, char* buffer) { 
    return fs_read_to_buffer(name, buffer); 
}

int os_file_delete(char* name) { 
    return fs_delete(name); 
}

void os_file_list() { 
    fs_list(); 
}

// --- GUI / JANELAS ---

void os_create_window(char* title, int x, int y, int w, int h, uint8_t color) {
    gfx_draw_window(title, x, y, w, h, color);
}

void os_set_cursor(int x, int y) {
    vga_set_cursor(x, y);
}

// Lógica centralizada para detectar clique no botão FECHAR
int os_window_close_clicked(int win_x, int win_y, int win_w, int mouse_x, int mouse_y) {
    // Estas coordenadas devem bater com o que está desenhado em vga.c (gfx_draw_window)
    // Botão fica em (x + w - 10), na barra superior (y + 2)
    int btn_x = win_x + win_w - 10;
    int btn_y = win_y + 2;
    int btn_size = 8; // Tamanho 8x8 pixels

    // Verifica se o mouse está dentro do quadrado do botão
    if (mouse_x >= btn_x && mouse_x <= (btn_x + btn_size) &&
        mouse_y >= btn_y && mouse_y <= (btn_y + btn_size)) {
        return 1; // Clicou no X!
    }
    return 0;
}

// --- SYSTEM ---
void os_reboot() { 
    // Tenta reiniciar pelo controlador de teclado
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    
    // Se falhar, força Triple Fault
    __asm__ volatile("int3"); 
}

// Retorna 1 se o clique foi na barra de título (mas NÃO no botão fechar)
int os_window_title_clicked(int win_x, int win_y, int win_w, int mouse_x, int mouse_y) {
    // Barra de título tem 12 pixels de altura
    // Devemos ignorar o botão de fechar (últimos 15 pixels da direita)
    
    if (mouse_x >= win_x && mouse_x <= (win_x + win_w - 15) &&
        mouse_y >= win_y && mouse_y <= (win_y + 12)) {
        return 1;
    }
    return 0;
}