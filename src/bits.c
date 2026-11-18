#include "bits.h"

/* ---------- Champs ---------- */
uint32_t write_field(uint32_t reg, uint32_t mask, uint8_t pos, uint32_t value)
{
    reg &= ~(mask << pos);            /* 1) effacer la zone du champ      */
    reg |= (value & mask) << pos;     /* 2) écrire la valeur (coupée)     */
    return reg;
}

uint32_t read_field(uint32_t reg, uint32_t mask, uint8_t pos)
{
    return (reg >> pos) & mask;       /* ramener à droite, garder le champ */
}

/* ---------- Lecture ---------- */
uint16_t u16_from_be(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

uint16_t u16_from_le(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[1] << 8) | (uint16_t)p[0]);
}

uint32_t u32_from_be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |
            (uint32_t)p[3];
}

uint32_t u32_from_le(const uint8_t *p)
{
    return ((uint32_t)p[3] << 24) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[1] << 8)  |
            (uint32_t)p[0];
}

/* ---------- Écriture ---------- */
void u16_to_be(uint16_t value, uint8_t *p)
{
    p[0] = (uint8_t)(value >> 8);      /* gros octet en premier */
    p[1] = (uint8_t)(value & 0xFFU);
}

void u32_to_le(uint32_t value, uint8_t *p)
{
    p[0] = (uint8_t)(value & 0xFFU);   /* petit octet en premier */
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}