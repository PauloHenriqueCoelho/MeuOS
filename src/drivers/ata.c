#include "../include/io.h"
#include <stdint.h>

#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LO      0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HI      0x1F5
#define ATA_DRIVE_HEAD  0x1F6
#define ATA_STATUS      0x1F7
#define ATA_COMMAND     0x1F7

// Comandos ATA
#define CMD_READ_PIO    0x20
#define CMD_WRITE_PIO   0x30

// Status bits
#define STATUS_BSY      0x80  // Busy (Ocupado)
#define STATUS_DRQ      0x08  // Data Request (Pronto para transferir)
#define STATUS_ERR      0x01  // Erro

// Espera o HD ficar pronto (sem usar interrupções)
void ata_wait_bsy() {
    while(inb(ATA_STATUS) & STATUS_BSY);
}

void ata_wait_drq() {
    while(!(inb(ATA_STATUS) & STATUS_DRQ));
}

// Lê um setor (512 bytes) do disco
// LBA: Endereço do bloco (0, 1, 2...)
// buffer: Onde salvar os dados na memória (deve ter 512 bytes)
void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    ata_wait_bsy();

    // Seleciona o Drive Master (0xE0) e os bits altos do LBA
    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    
    // Envia NULL byte para portas altas (legado)
    outb(ATA_ERROR, 0x00);
    
    // Quantos setores ler? (1)
    outb(ATA_SECTOR_CNT, 1);
    
    // Envia o endereço LBA (Low, Mid, High)
    outb(ATA_LBA_LO, (uint8_t) lba);
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HI, (uint8_t)(lba >> 16));
    
    // Envia comando "READ"
    outb(ATA_COMMAND, CMD_READ_PIO);

    // Espera o disco processar
    ata_wait_bsy();
    ata_wait_drq();

    // Lê os dados! (256 words = 512 bytes)
    for (int i = 0; i < 256; i++) {
        uint16_t tmp = inw(ATA_DATA);
        buffer[i * 2] = (uint8_t) tmp;
        buffer[i * 2 + 1] = (uint8_t)(tmp >> 8);
    }
}

// Escreve um setor (512 bytes) no disco
void ata_write_sector(uint32_t lba, uint8_t* data) {
    ata_wait_bsy();

    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_ERROR, 0x00);
    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LO, (uint8_t) lba);
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HI, (uint8_t)(lba >> 16));
    
    // Envia comando "WRITE"
    outb(ATA_COMMAND, CMD_WRITE_PIO);

    ata_wait_bsy();
    ata_wait_drq();

    // Escreve os dados
    for (int i = 0; i < 256; i++) {
        uint16_t tmp = data[i * 2] | (data[i * 2 + 1] << 8);
        outw(ATA_DATA, tmp);
    }
    
    // Comando de Flush (garantir que gravou)
    outb(ATA_COMMAND, 0xE7); // Cache Flush
    ata_wait_bsy();
}