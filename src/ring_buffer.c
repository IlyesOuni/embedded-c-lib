#include "ring_buffer.h"

/* vrai si x vaut 1, 2, 4, 8, 16... */
static bool is_power_of_two(size_t x)
{
    return (x != 0U) && ((x & (x - 1U)) == 0U);
}

bool rb_init(ring_buffer_t *rb, uint8_t *storage, size_t size)
{
    if ((rb == NULL) || (storage == NULL) || !is_power_of_two(size)) {
        return false;
    }
    rb->buffer = storage;
    rb->size   = size;
    rb->head   = 0U;
    rb->tail   = 0U;
    return true;
}

size_t rb_count(const ring_buffer_t *rb)
{
    return rb->head - rb->tail;
}

bool rb_is_empty(const ring_buffer_t *rb)
{
    return rb_count(rb) == 0U;
}

bool rb_is_full(const ring_buffer_t *rb)
{
    return rb_count(rb) == rb->size;
}

bool rb_put(ring_buffer_t *rb, uint8_t data)
{
    if (rb_is_full(rb)) {
        return false;
    }
    rb->buffer[rb->head & (rb->size - 1U)] = data;  /* 1) écrire la donnée */
    rb->head++;                                     /* 2) puis publier     */
    return true;
}

bool rb_get(ring_buffer_t *rb, uint8_t *data)
{
    if ((data == NULL) || rb_is_empty(rb)) {
        return false;
    }
    *data = rb->buffer[rb->tail & (rb->size - 1U)]; /* 1) lire la donnée   */
    rb->tail++;                                     /* 2) puis libérer     */
    return true;
}

void rb_reset(ring_buffer_t *rb)
{
    rb->head = 0U;
    rb->tail = 0U;
}