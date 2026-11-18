#ifndef BITS_H
#define BITS_H

#include <stdint.h>

/* ---------- Un seul bit ---------- */
#define BIT(n)              (1UL << (n))
#define SET_BIT(reg, n)     ((reg) |=  BIT(n))
#define CLEAR_BIT(reg, n)   ((reg) &= ~BIT(n))
#define TOGGLE_BIT(reg, n)  ((reg) ^=  BIT(n))
#define READ_BIT(reg, n)    (((reg) >> (n)) & 1UL)

/* ---------- Un champ de plusieurs bits ---------- */
/* mask = largeur du champ non décalée (ex : 3 bits → 0x7) */
uint32_t write_field(uint32_t reg, uint32_t mask, uint8_t pos, uint32_t value);
uint32_t read_field(uint32_t reg, uint32_t mask, uint8_t pos);

/* ---------- Endianness ---------- */
uint16_t u16_from_be(const uint8_t *p);
uint16_t u16_from_le(const uint8_t *p);
uint32_t u32_from_be(const uint8_t *p);
uint32_t u32_from_le(const uint8_t *p);
void     u16_to_be(uint16_t value, uint8_t *p);
void     u32_to_le(uint32_t value, uint8_t *p);

#endif