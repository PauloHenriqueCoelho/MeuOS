#include "../include/io.h"
#include "../include/utils.h" // Para os_print (se existir, ou declaramos extern)
#include "../include/api.h"   // Para os_print
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

#define CMD_READ_PIO    0x20
#define CMD_WRITE_PIO   0x30

#define STATUS_BSY      0x80
#define STATUS_DRQ      0x08
#define STATUS_ERR      0x01

// Timeout para não travar o OS (aprox)
#define ATA_TIMEOUT     1000000 

void ata_wait_bsy() {
    int timeout = ATA_TIMEOUT;
    while(inb(ATA_STATUS) & STATUS_BSY) {
        timeout--;
        if (timeout == 0) {
            os_print("[ATA] Timeout BSY!\n");
            break;
        }
    }
}

void ata_wait_drq() {
    int timeout = ATA_TIMEOUT;
    while(!(inb(ATA_STATUS) & STATUS_DRQ)) {
        timeout--;
        if (timeout == 0) {
            os_print("[ATA] Timeout DRQ!\n");
            break;
        }
    }
}

void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    // os_print("[ATA] Lendo LBA: "); 
    // char tmp_c = '0' + (lba % 10); char tmp_s[2] = {tmp_c, 0}; os_print(tmp_s);
    // os_print("\n");

    ata_wait_bsy();

    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_ERROR, 0x00);
    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LO, (uint8_t) lba);
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HI, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, CMD_READ_PIO);

    ata_wait_bsy();
    ata_wait_drq();

    // Verifica erro
    if (inb(ATA_STATUS) & STATUS_ERR) {
        os_print("[ATA] Erro de Leitura!\n");
        return;
    }

    for (int i = 0; i < 256; i++) {
        uint16_t tmp = inw(ATA_DATA);
        buffer[i * 2] = (uint8_t) tmp;
        buffer[i * 2 + 1] = (uint8_t)(tmp >> 8);
    }
}

void ata_write_sector(uint32_t lba, uint8_t* data) {
    ata_wait_bsy();

    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_ERROR, 0x00);
    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LO, (uint8_t) lba);
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HI, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, CMD_WRITE_PIO);

    ata_wait_bsy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        uint16_t tmp = data[i * 2] | (data[i * 2 + 1] << 8);
        outw(ATA_DATA, tmp);
    }
    
    outb(ATA_COMMAND, 0xE7); // Flush
    ata_wait_bsy();
}