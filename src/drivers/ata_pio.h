#ifndef ATA_PIO_H
#define ATA_PIO_H

#include <stdint.h>

/**
 * ata_pio_init - Initialize the ATA PIO drivers
 *
 * Initializes the ATA PIO drivers and detects the drives.
 *
 * Return: Nothing
 */
void ata_pio_init(void);

/**
 * ata_pio_read28 - Read sectors via LBA28
 * @base: ATA data base port (e.g. 0x1F0)
 * @alt: Alternate status port (e.g. 0x3F6)
 * @drive: Drive select (0xA0 master, 0xB0 slave)
 * @lba: Starting LBA (28-bit)
 * @count: Number of sectors to read
 * @buffer: Buffer for data (count * 256 uint16_t)
 * Return: 0 on success, -1 on error
 */
int ata_pio_read28(uint16_t base, uint16_t alt, uint8_t drive, uint64_t lba, uint32_t count, uint16_t *buffer);

/**
 * ata_pio_write28 - Write sectors via LBA28
 * @base: ATA data base port (e.g. 0x1F0)
 * @alt: Alternate status port (e.g. 0x3F6)
 * @drive: Drive select (0xA0 master, 0xB0 slave)
 * @lba: Starting LBA (28-bit)
 * @count: Number of sectors to write
 * @buffer: Data to write (count * 256 uint16_t)
 * Return: 0 on success, -1 on error
 */
int ata_pio_write28(uint16_t base, uint16_t alt, uint8_t drive, uint32_t lba, uint32_t count, uint16_t *buffer);

/**
 * ata_pio_read48 - Read sectors via LBA48
 * @base: ATA data base port (e.g. 0x1F0)
 * @alt: Alternate status port (e.g. 0x3F6)
 * @drive: Drive select (0xA0 master, 0xB0 slave)
 * @lba: Starting LBA (48-bit)
 * @count: Number of sectors to read
 * @buffer: Buffer for data (count * 256 uint16_t)
 * Return: 0 on success, -1 on error
 */
int ata_pio_read48(uint16_t base, uint16_t alt, uint8_t drive, uint64_t lba, uint32_t count, uint16_t *buffer);

/**
 * ata_pio_write48 - Write sectors via LBA48
 * @base: ATA data base port (e.g. 0x1F0)
 * @alt: Alternate status port (e.g. 0x3F6)
 * @drive: Drive select (0xA0 master, 0xB0 slave)
 * @lba: Starting LBA (48-bit)
 * @count: Number of sectors to write
 * @buffer: Buffer to write from
 * Return: 0 on success, -1 on error
 */
int ata_pio_write48(uint16_t base, uint16_t alt, uint8_t drive, uint64_t lba, uint32_t count, uint16_t *buffer);

#endif