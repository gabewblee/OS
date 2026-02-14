#include "ata_pio.h"
#include "vga.h"

#include "../io.h"
#include "../utils.h"

/**
 * ATA PIO primary bus base port
 */
#define ATA_PRIMARY_BUS_BASE_PORT                   0x1F0

/**
 * ATA PIO secondary bus base port
 */
#define ATA_SECONDARY_BUS_BASE_PORT                 0x170

/**
 * ATA PIO alternate status bus base port
 */
#define ATA_ALT_STATUS_BUS_PRIMARY_BASE_PORT        0x3F6

/**
 * ATA PIO alternate status bus secondary base port
 */
#define ATA_ALT_STATUS_BUS_SECONDARY_BASE_PORT      0x376

/**
 * ATA PIO primary master drive
 */
#define ATA_PRIMARY_MASTER_DRIVE                    0xA0

/**
 * ATA PIO primary slave drive
 */
#define ATA_PRIMARY_SLAVE_DRIVE                     0xB0

/**
 * ATA PIO secondary master drive
 */
#define ATA_SECONDARY_MASTER_DRIVE                  0xA0

/**     
 * ATA PIO secondary slave drive        
 */     
#define ATA_SECONDARY_SLAVE_DRIVE                   0xB0

/**     
 * ATA PIO IO Base Address Offsets      
 */     
#define ATA_RW_DATA_REG_OFFSET                      0x00

/**     
 * ATA PIO Sector Count Register Offset     
 */     
#define ATA_RW_SECTOR_COUNT_REG_OFFSET              0x02

/**     
 * ATA PIO LBA Low Register Offset      
 */     
#define ATA_RW_LBA_LO_REG_OFFSET                    0x03

/**     
 * ATA PIO LBA Mid Register Offset      
 */     
#define ATA_RW_LBA_MID_REG_OFFSET                   0x04

/**     
 * ATA PIO LBA High Register Offset     
 */     
#define ATA_RW_LBA_HI_REG_OFFSET                    0x05

/**     
 * ATA PIO Drive Register Offset        
 */     
#define ATA_RW_DRIVE_REG_OFFSET                     0x06

/**     
 * ATA PIO Status Register Offset       
 */     
#define ATA_R_STATUS_REG_OFFSET                     0x07

/**     
 * ATA PIO Command Register Offset      
 */     
#define ATA_W_COMMAND_REG_OFFSET                    0x07

/**     
 * ATA PIO Control Base Address Offsets     
 */     
#define ATA_R_ALT_STATUS_REG_OFFSET                 0x00

/**     
 * ATA PIO Status Register bits     
 */     
#define ATA_STATUS_ERR                              (1u << 0)

/**
 * ATA PIO DRQ Status Register bit
 */
#define ATA_STATUS_DRQ                              (1u << 3)

/**
 * ATA PIO DF Status Register bit
 */
#define ATA_STATUS_DF                               (1u << 5)

/**
 * ATA PIO BSY Status Register bit
 */
#define ATA_STATUS_BSY                              (1u << 7)

/**
 * ATA PIO Device Control Register NIEN bit
 */
#define ATA_DEV_CTRL_NIEN                           (1u << 1)

/**
 * ATA PIO Identify Command
 */
#define ATA_CMD_IDENTIFY                            0xEC

/**
 * ATA_NUM_DRIVES - Number of ATA drives
 */
#define ATA_NUM_DRIVES 4

/**
 * struct drive_config_t - Drive configuration
 * @present: Present bit
 * @lba28_supported: LBA28 supported bit
 * @lba48_supported: LBA48 supported bit
 * @total_num_sectors: Total number of sectors
 * @logical_sector_size: Logical sector size
 * @physical_sector_size: Physical sector size
 */
typedef struct drive_config_t {
    uint8_t present;    
    uint8_t lba28_supported;
    uint8_t lba48_supported;
    uint64_t total_num_sectors;
    uint32_t logical_sector_size;
    uint32_t physical_sector_size;
} drive_config_t;

/**
 * configs - Drive configurations array
 */
static drive_config_t configs[ATA_NUM_DRIVES];

/**
 * ata_pio_disable_disk_irq - Disable disk IRQ
 *
 * @alt: Base port of the alternate status ATA bus
 *
 * Return: Nothing
 */
static void ata_pio_disable_disk_irq(uint16_t alt) {
    outb(alt, ATA_DEV_CTRL_NIEN);
}

/**
 * ata_pio_detect_floating_bus - Detect floating bus
 *
 * Detects if the ATA bus is floating.
 *
 * @base: Base port of the ATA bus
 *
 * Return: -1 if the ATA bus is floating, 0 otherwise
 */
static int ata_pio_detect_floating_bus(uint16_t base) {
    uint8_t status = inb(base + ATA_R_STATUS_REG_OFFSET);
    if (status == 0xFF)
        return -1;

    return 0;
}

/**
 * ata_io_wait - Wait for I/O operation to complete
 *
 * Waits for the I/O operation to complete.
 *
 * @alt: Base port of the alternate status ATA bus
 *
 * Return: Nothing
 */
static void ata_io_wait(uint16_t alt) {
    for (int i = 0; i < 15; i++)
        inb(alt + ATA_R_ALT_STATUS_REG_OFFSET);
}

/**
 * ata_bsy_wait - Wait for BSY to clear
 * @alt: Alternate status port
 */
static void ata_bsy_wait(uint16_t alt) {
    while (inb(alt + ATA_R_ALT_STATUS_REG_OFFSET) & ATA_STATUS_BSY)
        ;
}

/**
 * ata_drq_wait - Wait for DRQ; return on ERR/DF
 * @alt: Alternate status port
 * Return: 0 when DRQ set, -1 on ERR or DF
 */
static int ata_drq_wait(uint16_t alt) {
    for (;;) {
        uint8_t status = inb(alt + ATA_R_ALT_STATUS_REG_OFFSET);
        if (status & ATA_STATUS_ERR)
            return -1;
        if (status & ATA_STATUS_DF)
            return -1;
        if (status & ATA_STATUS_DRQ)
            return 0;
    }
}

/**
 * ata_read_sector - Read one sector (words) from data register
 * @base: ATA data base port
 * @buffer: Destination for sector data
 * @words_per_sector: Number of 16-bit words per sector
 */
static void ata_read_sector(uint16_t base, uint16_t *buffer, uint32_t words_per_sector) {
    uint16_t port = base + ATA_RW_DATA_REG_OFFSET;
    for (uint32_t i = 0; i < words_per_sector; i++)
        buffer[i] = inw(port);
}

/**
 * ata_write_sector - Write one sector (words) to data register
 * @base: ATA data base port
 * @buffer: Source of sector data
 * @words_per_sector: Number of 16-bit words per sector
 */
static void ata_write_sector(uint16_t base, const uint16_t *buffer, uint32_t words_per_sector) {
    uint16_t port = base + ATA_RW_DATA_REG_OFFSET;
    for (uint32_t i = 0; i < words_per_sector; i++)
        outw(port, buffer[i]);
}

/**
 * ata_write_flush - Wait for BSY, send CACHE FLUSH, wait BSY
 * @base: ATA data base port (for command register)
 * @alt: Alternate status port
 */
static void ata_write_flush(uint16_t base, uint16_t alt) {
    ata_bsy_wait(alt);
    outb(base + ATA_W_COMMAND_REG_OFFSET, 0xE7);
    ata_bsy_wait(alt);
}

/**
 * ata_get_drive_config_idx - Get the index of a drive
 *
 * @base: Base port of the ATA bus
 * @drive: Drive to get the index of
 *
 * Return: The index of the drive on success, -1 otherwise
 */
static int ata_get_drive_config_idx(uint16_t base, uint8_t drive) {
    int bus = (base == ATA_PRIMARY_BUS_BASE_PORT) ? 0 : 1;
    int dev = (drive == ATA_PRIMARY_MASTER_DRIVE) ? 0 : 1;
    return bus << 1 | dev;
}

/**
 * ata_pio_drive_config_zero - Zero out drive configuration
 *
 * @drive_config: Pointer to the drive configuration
 *
 * Return: Nothing
 */
 static void ata_pio_drive_config_zero(drive_config_t *drive_config) {
    drive_config->present = 0;
    drive_config->lba28_supported = 0;
    drive_config->lba48_supported = 0;
    drive_config->total_num_sectors = 0;
    drive_config->logical_sector_size = 0;
    drive_config->physical_sector_size = 0;
}

/**
 * ata_identify_drive - Identify a drive
 *
 * @base: Base port of the ATA bus
 * @alt: Base port of the alternate status ATA bus
 * @drive: Drive to identify
 *
 * Return: 0 if the drive is present, -1 otherwise
 */
static int ata_identify_drive(uint16_t base, uint16_t alt, uint8_t drive) {
    /* Select drive */
    outb(base + ATA_RW_DRIVE_REG_OFFSET, drive);
    ata_io_wait(alt);

    /* Set sector count, LBA low, LBA mid, and LBA high to 0 */
    outb(base + ATA_RW_SECTOR_COUNT_REG_OFFSET, 0x00);
    outb(base + ATA_RW_LBA_LO_REG_OFFSET, 0x00);
    outb(base + ATA_RW_LBA_MID_REG_OFFSET, 0x00);
    outb(base + ATA_RW_LBA_HI_REG_OFFSET, 0x00);

    /* Send identify command */
    outb(base + ATA_W_COMMAND_REG_OFFSET, ATA_CMD_IDENTIFY);

    /* If 0, no drive is present */
    uint8_t status = inb(alt + ATA_R_ALT_STATUS_REG_OFFSET);
    if (status == 0)
        return -1;

    /* Wait for BSY bit to be clear */
    while (inb(alt + ATA_R_ALT_STATUS_REG_OFFSET) & ATA_STATUS_BSY);

    /* Read LBA mid and LBA high */
    uint8_t lba_mid = inb(base + ATA_RW_LBA_MID_REG_OFFSET);
    uint8_t lba_hi = inb(base + ATA_RW_LBA_HI_REG_OFFSET);

    /* Check if LBA mid and LBA high are 0 */
    if (lba_mid != 0 || lba_hi != 0) {
        vga_print_string("Error: non-ATA device detected\n", WHITE, BLACK);
        return -1;
    }
    
    /* Wait for data to be ready */
    while (1) {
        status = inb(alt + ATA_R_ALT_STATUS_REG_OFFSET);
        if (status & ATA_STATUS_ERR)
            return -1;

        /* Wait for DRQ bit to be set */
        if (status & ATA_STATUS_DRQ)
            break;
    }
    
    /* Read identify data */
    uint16_t data[256];
    for (int i = 0; i < 256; i++)
        data[i] = inw(base + ATA_RW_DATA_REG_OFFSET);

    /* Parse identify data */
    int idx = ata_get_drive_config_idx(base, drive);
    if (idx == -1)
        return -1;

    struct drive_config_t *drive_config = &configs[idx];
    ata_pio_drive_config_zero(drive_config);
    drive_config->present = 1;
    drive_config->lba28_supported = (data[49] & (1 << 9)) != 0;
    drive_config->lba48_supported = (data[83] & (1 << 10)) != 0;

    uint64_t total_num_sectors = 0;
    if (drive_config->lba48_supported)
        total_num_sectors = (uint64_t)data[100] | ((uint64_t)data[101] << 16) | ((uint64_t)data[102] << 32) | ((uint64_t)data[103] << 48);
    else if (drive_config->lba28_supported)
        total_num_sectors = (uint64_t)data[60] | ((uint64_t)data[61] << 16);
    else
        return -1;

    drive_config->total_num_sectors = total_num_sectors;

    uint32_t logical_sector_size = 512;
    if (data[53] & (1 << 1) && data[106] & (1 << 12))
        logical_sector_size = ((uint32_t)data[117] | ((uint32_t)data[118] << 16)) << 1;
    
    drive_config->logical_sector_size = logical_sector_size;

    uint32_t physical_sector_size = logical_sector_size;
    if (data[53] & (1 << 1) && data[106] & (1 << 13))
        physical_sector_size = logical_sector_size * (1 << (data[106] & 0xF));

    drive_config->physical_sector_size = physical_sector_size;
    return 0;
}

/**
 * ata_pio_init - Initialize the ATA PIO drivers
 *
 * Initializes the ATA PIO drivers and detects the drives.
 *
 * Return: Nothing
 */
void ata_pio_init(void) {
    if (ata_pio_detect_floating_bus(ATA_PRIMARY_BUS_BASE_PORT) == -1)
        panic("Error: floating primary ATA bus\n");

    if (ata_pio_detect_floating_bus(ATA_SECONDARY_BUS_BASE_PORT) == -1)
        panic("Error: floating secondary ATA bus\n");

    const char* founds[ATA_NUM_DRIVES] = {
        "Primary master drive found\n",
        "Primary slave drive found\n",
        "Secondary master drive found\n",
        "Secondary slave drive found\n"
    };

    const char* errors[ATA_NUM_DRIVES] = {
        "Error: primary master drive not found\n",
        "Error: primary slave drive not found\n",
        "Error: secondary master drive not found\n",
        "Error: secondary slave drive not found\n"
    };

    const uint16_t bases[ATA_NUM_DRIVES] = {
        ATA_PRIMARY_BUS_BASE_PORT,
        ATA_PRIMARY_BUS_BASE_PORT,
        ATA_SECONDARY_BUS_BASE_PORT,
        ATA_SECONDARY_BUS_BASE_PORT
    };

    const uint16_t alts[ATA_NUM_DRIVES] = {
        ATA_ALT_STATUS_BUS_PRIMARY_BASE_PORT,
        ATA_ALT_STATUS_BUS_PRIMARY_BASE_PORT,
        ATA_ALT_STATUS_BUS_SECONDARY_BASE_PORT,
        ATA_ALT_STATUS_BUS_SECONDARY_BASE_PORT
    };

    const uint8_t drives[ATA_NUM_DRIVES] = {
        ATA_PRIMARY_MASTER_DRIVE,
        ATA_PRIMARY_SLAVE_DRIVE,
        ATA_SECONDARY_MASTER_DRIVE,
        ATA_SECONDARY_SLAVE_DRIVE
    };

    for (int i = 0; i < ATA_NUM_DRIVES; i++) {
        if (ata_identify_drive(bases[i], alts[i], drives[i]) == -1) {
            vga_print_string(errors[i], WHITE, BLACK);
            configs[i].present = 0;
        } else {
            vga_print_string(founds[i], WHITE, BLACK);
            ata_pio_disable_disk_irq(alts[i]);
        }
    }
}

/**
 * ata_pio_read28 - Read sectors via LBA28
 *
 * @base: Base port of the ATA bus
 * @alt: Base port of the alternate status ATA bus
 * @drive: Drive to read from
 * @lba: Starting LBA (28-bit)
 * @count: Number of sectors to read
 * @buffer: Buffer to read into
 *
 * Return: 0 on success, -1 on error
 */
int ata_pio_read28(uint16_t base, uint16_t alt, uint8_t drive, uint64_t lba, uint32_t count, uint16_t *buffer) {
    int idx = ata_get_drive_config_idx(base, drive);
    if (idx == -1)
        return -1;
    
    drive_config_t *drive_config = &configs[idx];
    if (drive_config->present == 0) {
        vga_print_string("Error: drive not present\n", WHITE, BLACK);
        return -1;
    }
    
    if (drive_config->lba28_supported == 0) {
        vga_print_string("Error: LBA28 not supported\n", WHITE, BLACK);
        return -1;
    }

    if (lba + count > drive_config->total_num_sectors) {
        vga_print_string("Error: read out of range\n", WHITE, BLACK);
        return -1;
    }

    uint32_t words_per_sector = drive_config->logical_sector_size / 2;
    uint32_t total_sectors_read = 0;
    uint8_t slave = (drive == ATA_PRIMARY_SLAVE_DRIVE || drive == ATA_SECONDARY_SLAVE_DRIVE) ? 1 : 0;
    while (total_sectors_read < count) {
        uint32_t nsectors = (count - total_sectors_read > 256) ? 256 : (count - total_sectors_read);

        ata_bsy_wait(alt);
        outb(base + ATA_RW_DRIVE_REG_OFFSET, 0xE0 | (slave << 4) | ((lba >> 24) & 0x0F));
        ata_io_wait(alt);
        
        outb(base + ATA_RW_SECTOR_COUNT_REG_OFFSET, nsectors == 256 ? 0 : (uint8_t)nsectors);
        outb(base + ATA_RW_LBA_LO_REG_OFFSET, (uint8_t)(lba));
        outb(base + ATA_RW_LBA_MID_REG_OFFSET, (uint8_t)(lba >> 8));
        outb(base + ATA_RW_LBA_HI_REG_OFFSET, (uint8_t)(lba >> 16));
        outb(base + ATA_W_COMMAND_REG_OFFSET, 0x20);

        for (uint32_t s = 0; s < nsectors; s++) {
            ata_bsy_wait(alt);
            if (ata_drq_wait(alt) == -1)
                return -1;
            
            ata_read_sector(base, buffer + (total_sectors_read + s) * words_per_sector, words_per_sector);
        }

        total_sectors_read += nsectors;
        lba += nsectors;
    }

    return 0;
}

/**
 * ata_pio_write28 - Write sectors via LBA28
 *
 * @base: Base port of the ATA bus
 * @alt: Base port of the alternate status ATA bus
 * @drive: Drive to write to
 * @lba: Starting LBA (28-bit)
 * @count: Number of sectors to write
 * @buffer: Buffer to write from
 *
 * Return: 0 on success, -1 on error
 */
int ata_pio_write28(uint16_t base, uint16_t alt, uint8_t drive, uint32_t lba, uint32_t count, uint16_t *buffer) {
    int idx = ata_get_drive_config_idx(base, drive);
    if (idx == -1)
        return -1;

    drive_config_t *drive_config = &configs[idx];
    if (drive_config->present == 0) {
        vga_print_string("Error: drive not present\n", WHITE, BLACK);
        return -1;
    }

    if (drive_config->lba28_supported == 0) {
        vga_print_string("Error: LBA28 not supported\n", WHITE, BLACK);
        return -1;
    }

    if (lba + count > drive_config->total_num_sectors) {
        vga_print_string("Error: write out of range\n", WHITE, BLACK);
        return -1;
    }

    uint32_t words_per_sector = drive_config->logical_sector_size / 2;
    uint8_t slave = (drive == ATA_PRIMARY_SLAVE_DRIVE || drive == ATA_SECONDARY_SLAVE_DRIVE) ? 1 : 0;

    while (count) {
        uint32_t nsectors = (count > 256) ? 256 : count;

        ata_bsy_wait(alt);
        outb(base + ATA_RW_DRIVE_REG_OFFSET, 0xE0 | (slave << 4) | ((lba >> 24) & 0x0F));
        ata_io_wait(alt);

        outb(base + ATA_RW_SECTOR_COUNT_REG_OFFSET, nsectors == 256 ? 0 : (uint8_t)nsectors);
        outb(base + ATA_RW_LBA_LO_REG_OFFSET, (uint8_t)lba);
        outb(base + ATA_RW_LBA_MID_REG_OFFSET, (uint8_t)(lba >> 8));
        outb(base + ATA_RW_LBA_HI_REG_OFFSET, (uint8_t)(lba >> 16));
        outb(base + ATA_W_COMMAND_REG_OFFSET, 0x30);

        for (uint32_t s = 0; s < nsectors; s++) {
            if (ata_drq_wait(alt) == -1)
                return -1;

            ata_write_sector(base, buffer + s * words_per_sector, words_per_sector);
        }

        lba += nsectors;
        buffer += nsectors * words_per_sector;
        count -= nsectors;
    }

    ata_write_flush(base, alt);
    return 0;
}

/**
 * ata_pio_read48 - Read sectors via LBA48
 *
 * @base: Base port of the ATA bus
 * @alt: Base port of the alternate status ATA bus
 * @drive: Drive to read from
 * @lba: Starting LBA (48-bit)
 * @count: Number of sectors to read
 * @buffer: Buffer to read into
 *
 * Return: 0 on success, -1 on error
 */
int ata_pio_read48(uint16_t base, uint16_t alt, uint8_t drive, uint64_t lba, uint32_t count, uint16_t *buffer) {
    int idx = ata_get_drive_config_idx(base, drive);
    if (idx == -1)
        return -1;
    
    drive_config_t *drive_config = &configs[idx];
    if (drive_config->present == 0) {
        vga_print_string("Error: drive not present\n", WHITE, BLACK);
        return -1;
    }
    
    if (drive_config->lba48_supported == 0) {
        vga_print_string("Error: LBA48 not supported\n", WHITE, BLACK);
        return -1;
    }

    if (lba + count > drive_config->total_num_sectors) {
        vga_print_string("Error: read out of range\n", WHITE, BLACK);
        return -1;
    }

    uint32_t words_per_sector = drive_config->logical_sector_size / 2;
    uint32_t total_sectors_read = 0;
    uint8_t slave = (drive == ATA_PRIMARY_SLAVE_DRIVE || drive == ATA_SECONDARY_SLAVE_DRIVE) ? 1 : 0;

    while (total_sectors_read < count) {
        uint32_t nsectors = (count - total_sectors_read > 65536) ? 65536 : (count - total_sectors_read);

        ata_bsy_wait(alt);
        outb(base + ATA_RW_DRIVE_REG_OFFSET, 0x40 | (slave << 4));
        ata_io_wait(alt);

        uint16_t sector_count = nsectors == 65536 ? 0 : (uint16_t)nsectors;
        outb(base + ATA_RW_SECTOR_COUNT_REG_OFFSET, (uint8_t)(sector_count >> 8));
        outb(base + ATA_RW_LBA_LO_REG_OFFSET, (uint8_t)(lba >> 24));
        outb(base + ATA_RW_LBA_MID_REG_OFFSET, (uint8_t)(lba >> 32));
        outb(base + ATA_RW_LBA_HI_REG_OFFSET, (uint8_t)(lba >> 40));
        outb(base + ATA_RW_SECTOR_COUNT_REG_OFFSET, (uint8_t)(sector_count));
        outb(base + ATA_RW_LBA_LO_REG_OFFSET, (uint8_t)(lba));
        outb(base + ATA_RW_LBA_MID_REG_OFFSET, (uint8_t)(lba >> 8));
        outb(base + ATA_RW_LBA_HI_REG_OFFSET, (uint8_t)(lba >> 16));
        outb(base + ATA_W_COMMAND_REG_OFFSET, 0x24);

        for (uint32_t s = 0; s < nsectors; s++) {
            ata_bsy_wait(alt);
            if (ata_drq_wait(alt) == -1)
                return -1;

            ata_read_sector(base, buffer + (total_sectors_read + s) * words_per_sector, words_per_sector);
        }

        total_sectors_read += nsectors;
        lba += nsectors;
    }

    return 0;
}

/**
 * ata_pio_write48 - Write sectors via LBA48
 *
 * @base: Base port of the ATA bus
 * @alt: Base port of the alternate status ATA bus
 * @drive: Drive to write to
 * @lba: Starting LBA (48-bit)
 * @count: Number of sectors to write
 * @buffer: Buffer to write from
 *
 * Return: 0 on success, -1 on error
 */
int ata_pio_write48(uint16_t base, uint16_t alt, uint8_t drive, uint64_t lba, uint32_t count, uint16_t *buffer) {
    int idx = ata_get_drive_config_idx(base, drive);
    if (idx == -1)
        return -1;

    drive_config_t *drive_config = &configs[idx];
    if (drive_config->present == 0) {
        vga_print_string("Error: drive not present\n", WHITE, BLACK);
        return -1;
    }

    if (drive_config->lba48_supported == 0) {
        vga_print_string("Error: LBA48 not supported\n", WHITE, BLACK);
        return -1;
    }

    if (lba + count > drive_config->total_num_sectors) {
        vga_print_string("Error: write out of range\n", WHITE, BLACK);
        return -1;
    }

    uint32_t words_per_sector = drive_config->logical_sector_size / 2;
    uint8_t slave = (drive == ATA_PRIMARY_SLAVE_DRIVE || drive == ATA_SECONDARY_SLAVE_DRIVE) ? 1 : 0;

    while (count) {
        uint32_t nsectors = (count > 65536) ? 65536 : count;

        ata_bsy_wait(alt);
        outb(base + ATA_RW_DRIVE_REG_OFFSET, 0x40 | (slave << 4));
        ata_io_wait(alt);

        uint16_t sector_count = nsectors == 65536 ? 0 : (uint16_t)nsectors;
        outb(base + ATA_RW_SECTOR_COUNT_REG_OFFSET, (uint8_t)(sector_count >> 8));
        outb(base + ATA_RW_LBA_LO_REG_OFFSET, (uint8_t)(lba >> 24));
        outb(base + ATA_RW_LBA_MID_REG_OFFSET, (uint8_t)(lba >> 32));
        outb(base + ATA_RW_LBA_HI_REG_OFFSET, (uint8_t)(lba >> 40));
        outb(base + ATA_RW_SECTOR_COUNT_REG_OFFSET, (uint8_t)(sector_count));
        outb(base + ATA_RW_LBA_LO_REG_OFFSET, (uint8_t)(lba));
        outb(base + ATA_RW_LBA_MID_REG_OFFSET, (uint8_t)(lba >> 8));
        outb(base + ATA_RW_LBA_HI_REG_OFFSET, (uint8_t)(lba >> 16));
        outb(base + ATA_W_COMMAND_REG_OFFSET, 0x34);

        for (uint32_t s = 0; s < nsectors; s++) {
            if (ata_drq_wait(alt) == -1)
                return -1;

            ata_write_sector(base, buffer + s * words_per_sector, words_per_sector);
        }

        lba += nsectors;
        buffer += nsectors * words_per_sector;
        count -= nsectors;
    }

    ata_write_flush(base, alt);
    return 0;
}