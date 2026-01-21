#include "../include/mouse.h"
#include "../include/io.h"
#include "../include/utils.h"
#include "../include/vga.h"
#include "../include/idt.h"

#define CURSOR_W 12
#define CURSOR_H 19

static uint32_t backbuffer[CURSOR_W * CURSOR_H];
static int saved_x = 0;
static int saved_y = 0;
static int has_saved = 0;

uint8_t mouse_cycle = 0;
int8_t mouse_byte[3];
int mouse_x = 512; 
int mouse_y = 384; 
int mouse_status = 0;

static int mouse_cursor[19][12] = {
    {1,1,0,0,0,0,0,0,0,0,0,0}, {1,2,1,0,0,0,0,0,0,0,0,0}, {1,2,2,1,0,0,0,0,0,0,0,0},
    {1,2,2,2,1,0,0,0,0,0,0,0}, {1,2,2,2,2,1,0,0,0,0,0,0}, {1,2,2,2,2,2,1,0,0,0,0,0},
    {1,2,2,2,2,2,2,1,0,0,0,0}, {1,2,2,2,2,2,2,2,1,0,0,0}, {1,2,2,2,2,2,2,2,2,1,0,0},
    {1,2,2,2,2,2,1,1,1,1,0,0}, {1,2,2,2,1,2,1,0,0,0,0,0}, {1,2,1,1,0,1,2,1,0,0,0,0},
    {1,1,0,0,0,1,2,1,0,0,0,0}, {0,0,0,0,0,0,1,2,1,0,0,0}, {0,0,0,0,0,0,1,2,1,0,0,0},
    {0,0,0,0,0,0,0,1,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0}
};

void mouse_draw() {
    if (has_saved) {
        for(int y=0; y < CURSOR_H; y++) {
            for(int x=0; x < CURSOR_W; x++) {
                gfx_put_pixel(saved_x+x, saved_y+y, backbuffer[y * CURSOR_W + x]);
            }
        }
    }
    saved_x = mouse_x;
    saved_y = mouse_y;
    for(int y=0; y < CURSOR_H; y++) {
        for(int x=0; x < CURSOR_W; x++) {
            backbuffer[y * CURSOR_W + x] = gfx_get_pixel(mouse_x+x, mouse_y+y);
        }
    }
    has_saved = 1;
    for(int y=0; y < CURSOR_H; y++) {
        for(int x=0; x < CURSOR_W; x++) {
            int p = mouse_cursor[y][x];
            if (p==1) gfx_put_pixel(mouse_x+x, mouse_y+y, 0xFF000000); 
            if (p==2) gfx_put_pixel(mouse_x+x, mouse_y+y, 0xFFFFFFFF); 
        }
    }
}

// --- FUNÇÃO DE COMPATIBILIDADE ---
// Mantida vazia para satisfazer o linker (window.c chama ela)
void mouse_reset_background() {
    // Não precisa fazer nada, mouse_draw() já gerencia o fundo
}
// ---------------------------------

void mouse_handler_isr(registers_t* r) {
    (void)r;
    uint8_t status = inb(0x64);
    if (!(status & 1)) return;
    uint8_t val = inb(0x60);
    
    if (mouse_cycle == 0 && !(val & 0x08)) return;
    mouse_byte[mouse_cycle++] = val;

    if (mouse_cycle >= 3) {
        mouse_cycle = 0;
        int dx = mouse_byte[1];
        int dy = mouse_byte[2];
        if (mouse_byte[0] & 0x10) dx |= 0xFFFFFF00;
        if (mouse_byte[0] & 0x20) dy |= 0xFFFFFF00;
        mouse_x += dx;
        mouse_y -= dy;
        int w = get_screen_width();
        int h = get_screen_height();
        if (w==0) w=1024; if (h==0) h=768;
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_x >= w-1) mouse_x = w-1;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_y >= h-1) mouse_y = h-1;
        mouse_status = mouse_byte[0];
    }
}

void mouse_wait(uint8_t type) {
    uint32_t t = 100000;
    while(t--) { if((inb(0x64) & (type?2:1)) == (type?0:1)) return; }
}

void mouse_write(uint8_t write) {
    mouse_wait(1); outb(0x64, 0xD4);
    mouse_wait(1); outb(0x60, write);
}

uint8_t mouse_read() {
    mouse_wait(0); return inb(0x60);
}

void mouse_init() {
    register_interrupt_handler(44, mouse_handler_isr);
    mouse_wait(1); outb(0x64, 0xA8);
    mouse_wait(1); outb(0x64, 0x20);
    mouse_wait(0); uint8_t s = (inb(0x60) | 2);
    mouse_wait(1); outb(0x64, 0x60);
    mouse_wait(1); outb(0x60, s);
    mouse_write(0xF6); mouse_read();
    mouse_write(0xF4); mouse_read();
}

void draw_mouse_cursor() {
    mouse_draw();
}

int mouse_get_x() { return mouse_x; }
int mouse_get_y() { return mouse_y; }
int mouse_get_status() { return mouse_status; }