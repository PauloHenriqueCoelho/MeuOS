#include "../include/idt.h"
#include "../include/io.h"
#include "../include/vga.h"
#include "../include/shell.h" // Necessário para rodar comandos
#include "../include/utils.h" // Necessário se usar strlen/etc

// --- Variáveis Globais do Shell ---
char shell_buffer[256];
int shell_index = 0;

// --- Mapa de Teclado (Movido do main.c) ---
char kbd_US [128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Estruturas IDT
idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;
extern void isr_generic_stub();
extern void idt_flush(uint32_t);
extern void isr1(); 

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags; 
}

void init_pic() {
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait(); 
    outb(0xA1, 0x28); io_wait();
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();

    // Máscara: Bloqueia tudo (0xFF) exceto Teclado (bit 1 desligado = 0xFD)
    outb(0x21, 0xFD); 
    outb(0xA1, 0xFF); 
}

void init_idt() {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // Limpa Buffer do Shell antes de começar
    for(int i=0; i<256; i++) shell_buffer[i] = 0;
    shell_index = 0;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)isr_generic_stub, 0x08, 0x8E);
    }

    init_pic();
    idt_set_gate(33, (uint32_t)isr1, 0x08, 0x8E); // IRQ1 = Int 33
    idt_flush((uint32_t)&idt_ptr);
}

// --- O NOVO CÉREBRO (Chamado automaticamente pela CPU) ---
void keyboard_handler_main() {
    // 1. Ler scancode (OBRIGATÓRIO para destravar o teclado)
    uint8_t scancode = inb(0x60);

    // 2. Se for tecla solta (break code), ignoramos
    if (scancode & 0x80) return;

    // 3. Traduzir e processar
    if (scancode < 128) {
        char c = kbd_US[scancode];

        if (c != 0) {
            // ENTER
            if (c == '\n') {
                shell_buffer[shell_index] = '\0';
                shell_execute(shell_buffer);
                
                shell_index = 0;
                for(int i=0; i<256; i++) shell_buffer[i] = 0;
                vga_print("\n> ");
            } 
            // BACKSPACE
            else if (c == '\b') {
                if (shell_index > 0) {
                    shell_index--;
                    shell_buffer[shell_index] = 0;
                    vga_backspace();
                }
            }
            // CARACTERES
            else {
                if (shell_index < 255) {
                    vga_putchar(c);
                    shell_buffer[shell_index] = c;
                    shell_index++;
                }
            }
        }
    }
    // Nota: O EOI (End of Interrupt) é enviado no Assembly (isr1), 
    // então não precisamos enviar outb(0x20, 0x20) aqui.
}