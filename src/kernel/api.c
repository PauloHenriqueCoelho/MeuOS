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
    vga_set_color(15, 0); // Restaura para branco
}

void os_clear_screen() { 
    vga_clear(); 
}

// --- INPUT (AGORA VIA INTERRUPÇÃO) ---
// Esta função trava a CPU (HLT) até alguém apertar uma tecla.
// Super eficiente para bateria e processador.
char os_wait_for_key() {
    while(1) {
        char c = keyboard_get_key(); // Pega do buffer circular
        
        if (c != 0) return c;
        
        // Se o buffer estiver vazio, dorme até a próxima interrupção
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

void os_create_window(char* title, int x, int y, int w, int h, uint8_t color) {
    gfx_draw_window(title, x, y, w, h, color);
}

void os_set_cursor(int x, int y) {
    vga_set_cursor(x, y);
}

// --- SYSTEM ---
void os_reboot() { 
    outb(0x64, 0xFE); 
}