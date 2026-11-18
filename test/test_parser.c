#include <string.h>
#include "unity.h"
#include "parser.h"
#include "crc.h"          /* nécessaires pour que Ceedling compile */
#include "bits.h"         /* aussi crc.c, bits.c et ring_buffer.c  */
#include "ring_buffer.h"

static parser_t p;
static frame_t  last;     /* dernière trame reçue  */
static int      calls;    /* nombre de trames reçues */

/* Faux callback : il enregistre ce qu'il reçoit */
static void on_frame(const frame_t *f, void *ctx)
{
    (void)ctx;
    last = *f;
    calls++;
}

static void feed_bytes(const uint8_t *b, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        parser_feed(&p, b[i]);
    }
}

void setUp(void)
{
    parser_init(&p, on_frame, NULL);
    calls = 0;
    (void)memset(&last, 0, sizeof(last));
}

void tearDown(void) {}

/* ===== frame_build ===== */
void test_build_frame_layout(void)
{
    const uint8_t data[] = {0x01, 0x02};
    const uint8_t crc_in[] = {0x02, 0x10, 0x01, 0x02};   /* LEN CMD PAYLOAD */
    uint8_t out[16];

    TEST_ASSERT_EQUAL_UINT(7, frame_build(0x10, data, 2, out, sizeof(out)));
    TEST_ASSERT_EQUAL_HEX8(0xAA, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, out[1]);
    TEST_ASSERT_EQUAL_HEX8(0x10, out[2]);
    TEST_ASSERT_EQUAL_HEX8(0x01, out[3]);
    TEST_ASSERT_EQUAL_HEX8(0x02, out[4]);
    TEST_ASSERT_EQUAL_HEX16(crc16_ccitt(crc_in, 4), u16_from_be(&out[5]));
}

void test_build_rejects_bad_parameters(void)
{
    uint8_t data[33] = {0};
    uint8_t out[64];
    TEST_ASSERT_EQUAL_UINT(0, frame_build(0x10, data, 33, out, sizeof(out))); /* trop long */
    TEST_ASSERT_EQUAL_UINT(0, frame_build(0x10, data, 4, out, 5));           /* out trop petit */
    TEST_ASSERT_EQUAL_UINT(0, frame_build(0x10, NULL, 4, out, sizeof(out)));
}

/* ===== parser ===== */
void test_valid_frame_is_delivered(void)
{
    const uint8_t data[] = {0x11, 0x22, 0x33};
    uint8_t buf[16];
    size_t n = frame_build(0x42, data, 3, buf, sizeof(buf));

    feed_bytes(buf, n);

    TEST_ASSERT_EQUAL_INT(1, calls);
    TEST_ASSERT_EQUAL_HEX8(0x42, last.cmd);
    TEST_ASSERT_EQUAL_UINT8(3, last.len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, last.payload, 3);
    TEST_ASSERT_EQUAL_UINT32(1, p.frames_ok);
}

void test_bad_crc_is_rejected(void)
{
    const uint8_t data[] = {0x11, 0x22};
    uint8_t buf[16];
    size_t n = frame_build(0x42, data, 2, buf, sizeof(buf));

    buf[n - 1U] ^= 0xFF;          /* on abîme le CRC */
    feed_bytes(buf, n);

    TEST_ASSERT_EQUAL_INT(0, calls);
    TEST_ASSERT_EQUAL_UINT32(1, p.crc_errors);
}

void test_corrupted_payload_is_rejected(void)
{
    const uint8_t data[] = {0x11, 0x22};
    uint8_t buf[16];
    size_t n = frame_build(0x42, data, 2, buf, sizeof(buf));

    buf[3] ^= 0x01;               /* 1 bit du payload change */
    feed_bytes(buf, n);

    TEST_ASSERT_EQUAL_INT(0, calls);
    TEST_ASSERT_EQUAL_UINT32(1, p.crc_errors);
}

void test_too_long_len_is_rejected_then_parser_recovers(void)
{
    const uint8_t bad[] = {0xAA, 50};          /* LEN = 50 > 32 */
    uint8_t buf[16];
    size_t n = frame_build(0x01, NULL, 0, buf, sizeof(buf));

    feed_bytes(bad, sizeof(bad));
    feed_bytes(buf, n);                        /* trame valide ensuite */

    TEST_ASSERT_EQUAL_UINT32(1, p.len_errors);
    TEST_ASSERT_EQUAL_INT(1, calls);
}

void test_garbage_before_sof_is_ignored(void)
{
    const uint8_t garbage[] = {0x00, 0x13, 0x37, 0xFF};
    const uint8_t data[] = {0x05};
    uint8_t buf[16];
    size_t n = frame_build(0x07, data, 1, buf, sizeof(buf));

    feed_bytes(garbage, sizeof(garbage));
    feed_bytes(buf, n);

    TEST_ASSERT_EQUAL_INT(1, calls);
    TEST_ASSERT_EQUAL_HEX8(0x07, last.cmd);
}

void test_stray_sof_before_frame(void)
{
    const uint8_t stray[] = {0xAA};            /* un SOF parasite */
    const uint8_t data[] = {0x05};
    uint8_t buf[16];
    size_t n = frame_build(0x07, data, 1, buf, sizeof(buf));

    feed_bytes(stray, 1);
    feed_bytes(buf, n);                        /* flux : AA AA 01 07 05 .. */

    TEST_ASSERT_EQUAL_INT(1, calls);
}

void test_two_frames_back_to_back(void)
{
    const uint8_t d1[] = {0x01};
    const uint8_t d2[] = {0x02, 0x03};
    uint8_t buf[32];
    size_t n1 = frame_build(0x10, d1, 1, buf, sizeof(buf));
    size_t n2 = frame_build(0x20, d2, 2, &buf[n1], sizeof(buf) - n1);

    feed_bytes(buf, n1 + n2);

    TEST_ASSERT_EQUAL_INT(2, calls);
    TEST_ASSERT_EQUAL_HEX8(0x20, last.cmd);    /* la dernière reçue */
}

void test_payload_containing_sof(void)
{
    const uint8_t data[] = {0xAA, 0xAA, 0xAA};
    uint8_t buf[16];
    size_t n = frame_build(0x33, data, 3, buf, sizeof(buf));

    feed_bytes(buf, n);

    TEST_ASSERT_EQUAL_INT(1, calls);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, last.payload, 3);
}

void test_empty_payload(void)
{
    uint8_t buf[16];
    size_t n = frame_build(0x99, NULL, 0, buf, sizeof(buf));

    TEST_ASSERT_EQUAL_UINT(5, n);
    feed_bytes(buf, n);

    TEST_ASSERT_EQUAL_INT(1, calls);
    TEST_ASSERT_EQUAL_HEX8(0x99, last.cmd);
    TEST_ASSERT_EQUAL_UINT8(0, last.len);
}

/* ===== Intégration : ISR UART → ring buffer → parser ===== */
void test_ring_buffer_to_parser_integration(void)
{
    ring_buffer_t rb;
    uint8_t storage[64];
    const uint8_t data[] = {0xDE, 0xAD};
    uint8_t buf[16];
    uint8_t byte;
    size_t n = frame_build(0x55, data, 2, buf, sizeof(buf));

    TEST_ASSERT_TRUE(rb_init(&rb, storage, sizeof(storage)));

    /* "l'ISR UART" range les octets reçus */
    for (size_t i = 0; i < n; i++) {
        TEST_ASSERT_TRUE(rb_put(&rb, buf[i]));
    }

    /* "la boucle principale" vide le buffer dans le parseur */
    while (rb_get(&rb, &byte)) {
        parser_feed(&p, byte);
    }

    TEST_ASSERT_EQUAL_INT(1, calls);
    TEST_ASSERT_EQUAL_HEX8(0x55, last.cmd);
    TEST_ASSERT_TRUE(rb_is_empty(&rb));
}