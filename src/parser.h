#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stddef.h>

#define FRAME_SOF          0xAAU   /* Start Of Frame                    */
#define FRAME_MAX_PAYLOAD  32U     /* taille max des données            */
#define FRAME_OVERHEAD     5U      /* SOF + LEN + CMD + CRC_H + CRC_L   */

/* Une trame décodée */
typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint8_t payload[FRAME_MAX_PAYLOAD];
} frame_t;

/* Fonction appelée quand une trame valide est reçue */
typedef void (*frame_cb_t)(const frame_t *frame, void *ctx);

/* Les états du parseur */
typedef enum {
    PS_WAIT_SOF,
    PS_GET_LEN,
    PS_GET_CMD,
    PS_GET_PAYLOAD,
    PS_GET_CRC_H,
    PS_GET_CRC_L
} parser_state_t;

typedef struct {
    parser_state_t state;
    frame_t        frame;      /* trame en cours de réception   */
    uint8_t        index;      /* position dans le payload      */
    uint16_t       crc_rx;     /* CRC reçu                      */
    frame_cb_t     callback;
    void          *ctx;
    /* statistiques */
    uint32_t       frames_ok;
    uint32_t       crc_errors;
    uint32_t       len_errors;
} parser_t;

void   parser_init(parser_t *p, frame_cb_t cb, void *ctx);
void   parser_feed(parser_t *p, uint8_t byte);

/* Construit une trame complète dans out. Renvoie sa taille, ou 0 si erreur. */
size_t frame_build(uint8_t cmd, const uint8_t *payload, uint8_t len,
                   uint8_t *out, size_t out_size);

#endif