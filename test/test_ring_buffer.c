#include "unity.h"
#include "ring_buffer.h"

static ring_buffer_t rb;
static uint8_t storage[8];   /* 8 assiettes */

/* exécutée AVANT chaque test : un tapis neuf */
void setUp(void)
{
    rb_init(&rb, storage, sizeof(storage));
}

void tearDown(void) {}

/* Test 1 : un tapis neuf est vide */
void test_new_buffer_is_empty(void)
{
    TEST_ASSERT_TRUE(rb_is_empty(&rb));
    TEST_ASSERT_EQUAL_UINT(0, rb_count(&rb));
}
/* Test 2 : rb_init refuse les mauvaises valeurs */
void test_init_rejects_bad_parameters(void)
{
    ring_buffer_t r;
    uint8_t s[6];
    TEST_ASSERT_FALSE(rb_init(NULL, storage, 8));
    TEST_ASSERT_FALSE(rb_init(&r, NULL, 8));
    TEST_ASSERT_FALSE(rb_init(&r, s, 0));
    TEST_ASSERT_FALSE(rb_init(&r, s, 6));   /* 6 n'est pas une puissance de 2 */
}

/* Test 3 : poser 1 sushi → count = 1, plus vide */
void test_put_one_increases_count(void)
{
    TEST_ASSERT_TRUE(rb_put(&rb, 42));
    TEST_ASSERT_EQUAL_UINT(1, rb_count(&rb));
    TEST_ASSERT_FALSE(rb_is_empty(&rb));
}

/* Test 4 : on récupère la même valeur */
void test_put_then_get_returns_same_value(void)
{
    uint8_t v = 0;
    rb_put(&rb, 42);
    TEST_ASSERT_TRUE(rb_get(&rb, &v));
    TEST_ASSERT_EQUAL_UINT8(42, v);
}

/* Test 5 : premier entré, premier sorti (FIFO) */
void test_fifo_order(void)
{
    uint8_t v;
    rb_put(&rb, 1); rb_put(&rb, 2); rb_put(&rb, 3);
    rb_get(&rb, &v); TEST_ASSERT_EQUAL_UINT8(1, v);
    rb_get(&rb, &v); TEST_ASSERT_EQUAL_UINT8(2, v);
    rb_get(&rb, &v); TEST_ASSERT_EQUAL_UINT8(3, v);
}

/* Test 6 : tapis plein → put refuse */
void test_put_fails_when_full(void)
{
    for (uint8_t i = 0; i < 8; i++) {
        TEST_ASSERT_TRUE(rb_put(&rb, i));
    }
    TEST_ASSERT_TRUE(rb_is_full(&rb));
    TEST_ASSERT_FALSE(rb_put(&rb, 99));
}

/* Test 7 : tapis vide → get refuse */
void test_get_fails_when_empty(void)
{
    uint8_t v;
    TEST_ASSERT_FALSE(rb_get(&rb, &v));
}

/* Test 8 : le tapis fait le tour (wrap-around) */
void test_wrap_around(void)
{
    uint8_t v;
    for (uint8_t i = 0; i < 8; i++) { rb_put(&rb, i); }   /* plein   */
    for (uint8_t i = 0; i < 4; i++) { rb_get(&rb, &v); }  /* -4      */
    for (uint8_t i = 10; i < 14; i++) { rb_put(&rb, i); } /* +4 (tour) */
    for (uint8_t i = 4; i < 8; i++) {
        rb_get(&rb, &v); TEST_ASSERT_EQUAL_UINT8(i, v);
    }
    for (uint8_t i = 10; i < 14; i++) {
        rb_get(&rb, &v); TEST_ASSERT_EQUAL_UINT8(i, v);
    }
    TEST_ASSERT_TRUE(rb_is_empty(&rb));
}

/* Test 9 : reset vide le tapis */
void test_reset_empties_buffer(void)
{
    rb_put(&rb, 1); rb_put(&rb, 2);
    rb_reset(&rb);
    TEST_ASSERT_TRUE(rb_is_empty(&rb));
}