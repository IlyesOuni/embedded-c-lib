#ifndef CRC_H
#define CRC_H

#include <stdint.h>
#include <stddef.h>

/* CRC16-CCITT (FALSE) : poly 0x1021, init 0xFFFF */
uint16_t crc16_ccitt(const uint8_t *data, size_t len);

/* CRC32 (Ethernet/ZIP) : poly 0xEDB88320, init 0xFFFFFFFF */
uint32_t crc32(const uint8_t *data, size_t len);

/* CRC32 rapide (avec table) - bonus */
uint32_t crc32_fast(const uint8_t *data, size_t len);

#endif