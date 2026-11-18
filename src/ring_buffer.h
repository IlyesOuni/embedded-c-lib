#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint8_t         *buffer;  /* les assiettes                  */
    size_t           size;    /* nombre d'assiettes (puiss. de 2) */
    volatile size_t  head;    /* sushis posés par le cuisinier  */
    volatile size_t  tail;    /* sushis pris par le client      */
} ring_buffer_t;

bool   rb_init(ring_buffer_t *rb, uint8_t *storage, size_t size);
bool   rb_put(ring_buffer_t *rb, uint8_t data);
bool   rb_get(ring_buffer_t *rb, uint8_t *data);
bool   rb_is_empty(const ring_buffer_t *rb);
bool   rb_is_full(const ring_buffer_t *rb);
size_t rb_count(const ring_buffer_t *rb);
void   rb_reset(ring_buffer_t *rb);

#endif