#include <string.h>
#include "parser.h"
#include "crc.h"
#include "bits.h"

/* CRC16 calculé sur LEN + CMD + PAYLOAD */
static uint16_t frame_crc(const frame_t *f)
{
    uint8_t tmp[2U + FRAME_MAX_PAYLOAD];

    tmp[0] = f->len;
    tmp[1] = f->cmd;
    (void)memcpy(&tmp[2], f->payload, f->len);
    return crc16_ccitt(tmp, (size_t)f->len + 2U);
}

void parser_init(parser_t *p, frame_cb_t cb, void *ctx)
{
    (void)memset(p, 0, sizeof(*p));
    p->state    = PS_WAIT_SOF;
    p->callback = cb;
    p->ctx      = ctx;
}

void parser_feed(parser_t *p, uint8_t byte)
{
    switch (p->state) {

    case PS_WAIT_SOF:
        if (byte == FRAME_SOF) {
            p->state = PS_GET_LEN;
        }
        break;

    case PS_GET_LEN:
        if (byte > FRAME_MAX_PAYLOAD) {
            p->len_errors++;
            /* si cet octet est un SOF, une nouvelle trame commence peut-être */
            p->state = (byte == FRAME_SOF) ? PS_GET_LEN : PS_WAIT_SOF;
        } else {
            p->frame.len = byte;
            p->state     = PS_GET_CMD;
        }
        break;

    case PS_GET_CMD:
        p->frame.cmd = byte;
        p->index     = 0U;
        p->state     = (p->frame.len == 0U) ? PS_GET_CRC_H : PS_GET_PAYLOAD;
        break;

    case PS_GET_PAYLOAD:
        p->frame.payload[p->index] = byte;
        p->index++;
        if (p->index >= p->frame.len) {
            p->state = PS_GET_CRC_H;
        }
        break;

    case PS_GET_CRC_H:
        p->crc_rx = (uint16_t)((uint16_t)byte << 8);
        p->state  = PS_GET_CRC_L;
        break;

    case PS_GET_CRC_L:
        p->crc_rx |= byte;
        if (p->crc_rx == frame_crc(&p->frame)) {
            p->frames_ok++;
            if (p->callback != NULL) {
                p->callback(&p->frame, p->ctx);
            }
        } else {
            p->crc_errors++;
        }
        p->state = PS_WAIT_SOF;       /* prêt pour la trame suivante */
        break;

    default:
        p->state = PS_WAIT_SOF;       /* état corrompu → état sûr */
        break;
    }
}

size_t frame_build(uint8_t cmd, const uint8_t *payload, uint8_t len,
                   uint8_t *out, size_t out_size)
{
    frame_t f;
    size_t  total = (size_t)len + FRAME_OVERHEAD;

    if ((out == NULL) || (len > FRAME_MAX_PAYLOAD) || (out_size < total) ||
        ((payload == NULL) && (len > 0U))) {
        return 0U;
    }

    f.cmd = cmd;
    f.len = len;
    if (len > 0U) {
        (void)memcpy(f.payload, payload, len);
    }

    out[0] = FRAME_SOF;
    out[1] = len;
    out[2] = cmd;
    if (len > 0U) {
        (void)memcpy(&out[3], payload, len);
    }
    u16_to_be(frame_crc(&f), &out[3U + len]);   /* CRC en big-endian */

    return total;
}