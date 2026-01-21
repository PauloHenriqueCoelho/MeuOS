#include "../include/io.h"
#include "../include/vga.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

// Lê um registrador do CMOS
uint8_t rtc_read(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

// Converte BCD para Binário normal
// Ex: Em BCD, o número 15 é gravado como 0x15, que em decimal é 21. Precisamos corrigir.
uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd & 0x0F);
}

void rtc_print_time() {
    // Registradores padrão do RTC:
    // 0x00: Segundos, 0x02: Minutos, 0x04: Horas
    // 0x07: Dia, 0x08: Mês, 0x09: Ano
    
    uint8_t second = bcd_to_binary(rtc_read(0x00));
    uint8_t minute = bcd_to_binary(rtc_read(0x02));
    uint8_t hour   = bcd_to_binary(rtc_read(0x04));
    uint8_t day    = bcd_to_binary(rtc_read(0x07));
    uint8_t month  = bcd_to_binary(rtc_read(0x08));
    uint8_t year   = bcd_to_binary(rtc_read(0x09));

    // Ajuste de Fuso Horário (Ex: -3 para Brasil)
    // Isso é uma simplificação, pois pode dar underflow (horas negativas), 
    // mas serve para teste.
    // if(hour >= 3) hour -= 3; else hour += 21;

    vga_print("Data: ");
    
    // Imprime Dia
    char buf[3];
    buf[0] = (day / 10) + '0'; buf[1] = (day % 10) + '0'; buf[2] = 0;
    vga_print(buf); vga_print("/");

    // Imprime Mês
    buf[0] = (month / 10) + '0'; buf[1] = (month % 10) + '0'; buf[2] = 0;
    vga_print(buf); vga_print("/20"); // Assumindo século 21

    // Imprime Ano
    buf[0] = (year / 10) + '0'; buf[1] = (year % 10) + '0'; buf[2] = 0;
    vga_print(buf);
    
    vga_print(" - Hora: ");

    // Imprime Hora
    buf[0] = (hour / 10) + '0'; buf[1] = (hour % 10) + '0'; buf[2] = 0;
    vga_print(buf); vga_print(":");

    // Imprime Minuto
    buf[0] = (minute / 10) + '0'; buf[1] = (minute % 10) + '0'; buf[2] = 0;
    vga_print(buf); vga_print(":");

    // Imprime Segundo
    buf[0] = (second / 10) + '0'; buf[1] = (second % 10) + '0'; buf[2] = 0;
    vga_print(buf); vga_print("\n");
}