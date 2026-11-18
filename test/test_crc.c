#include "unity.h"
#include "crc.h"

static const uint8_t check[] = "123456789";
#define CHECK_LEN 9U

void setUp(void) {}
void tearDown(void) {}

/* ---------- CRC16 ---------- */
void test_crc16_reference_value(void)
{
    TEST_ASSERT_EQUAL_HEX16(0x29B1, crc16_ccitt(check, CHECK_LEN));
}

void test_crc16_empty_returns_init(void)
{
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, crc16_ccitt(check, 0));
}

void test_one_bit_flip_changes_crc(void)
{
    uint8_t msg[] = "123456789";
    uint16_t before = crc16_ccitt(msg, CHECK_LEN);
    msg[4] ^= 0x01;
    TEST_ASSERT_NOT_EQUAL(before, crc16_ccitt(msg, CHECK_LEN));
}

/* ---------- CRC32 lent ---------- */
void test_crc32_reference_value(void)
{
    TEST_ASSERT_EQUAL_HEX32(0xCBF43926, crc32(check, CHECK_LEN));
}

void test_crc32_empty_returns_zero(void)
{
    TEST_ASSERT_EQUAL_HEX32(0x00000000, crc32(check, 0));
}

/* ---------- CRC32 rapide (NOUVEAU) ---------- */
void test_crc32_fast_reference_value(void)
{
    TEST_ASSERT_EQUAL_HEX32(0xCBF43926, crc32_fast(check, CHECK_LEN));
}

void test_crc32_fast_matches_slow(void)
{
    uint8_t buf[256];
    for (size_t i = 0; i < sizeof(buf); i++) {
        buf[i] = (uint8_t)(i * 7U + 3U);   /* données variées */
    }
    TEST_ASSERT_EQUAL_HEX32(crc32(buf, sizeof(buf)), crc32_fast(buf, sizeof(buf)));
}