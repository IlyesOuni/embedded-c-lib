#include "unity.h"
#include "debounce.h"

static debounce_t d;

void setUp(void)    { debounce_init(&d, 20); }   /* seuil : 20 ms */
void tearDown(void) {}

/* Outil : envoie "level" pendant "ms" millisecondes
   et compte combien de fois l'événement "wanted" apparaît */
static int feed(bool level, int ms, btn_event_t wanted)
{
    int count = 0;
    for (int i = 0; i < ms; i++) {
        if (debounce_update(&d, level) == wanted) {
            count++;
        }
    }
    return count;
}

/* 1. Bouton jamais touché → aucun événement */
void test_idle_gives_no_event(void)
{
    TEST_ASSERT_EQUAL_INT(0, feed(false, 100, BTN_EVT_PRESS));
    TEST_ASSERT_EQUAL(BTN_RELEASED, d.state);
}

/* 2. Bruit de 5 ms → ignoré */
void test_short_noise_is_ignored(void)
{
    TEST_ASSERT_EQUAL_INT(0, feed(true, 5, BTN_EVT_PRESS));
    TEST_ASSERT_EQUAL_INT(0, feed(false, 50, BTN_EVT_RELEASE));
    TEST_ASSERT_EQUAL(BTN_RELEASED, d.state);
}

/* 3. Appui stable → UN SEUL PRESS, exactement à la 20e ms */
void test_stable_press_gives_one_event(void)
{
    TEST_ASSERT_EQUAL_INT(0, feed(true, 19, BTN_EVT_PRESS));     /* pas encore */
    TEST_ASSERT_EQUAL(BTN_EVT_PRESS, debounce_update(&d, true)); /* 20e ms ! */
    TEST_ASSERT_EQUAL_INT(0, feed(true, 100, BTN_EVT_PRESS));    /* pas de doublon */
    TEST_ASSERT_EQUAL(BTN_PRESSED, d.state);
}

/* 4. Relâchement stable → UN SEUL RELEASE */
void test_stable_release_gives_one_event(void)
{
    feed(true, 20, BTN_EVT_PRESS);                               /* appuyer */
    TEST_ASSERT_EQUAL_INT(1, feed(false, 100, BTN_EVT_RELEASE));
    TEST_ASSERT_EQUAL(BTN_RELEASED, d.state);
}

/* 5. Rebonds puis stable → UN SEUL PRESS */
void test_bouncing_gives_one_press(void)
{
    for (int i = 0; i < 10; i++) {
        (void)debounce_update(&d, (i % 2) == 0);   /* 1,0,1,0,... */
    }
    TEST_ASSERT_EQUAL_INT(1, feed(true, 20, BTN_EVT_PRESS));
}

/* 6. Petite coupure pendant l'appui → pas de RELEASE */
void test_short_glitch_while_pressed_is_ignored(void)
{
    feed(true, 20, BTN_EVT_PRESS);
    TEST_ASSERT_EQUAL_INT(0, feed(false, 3, BTN_EVT_RELEASE));
    TEST_ASSERT_EQUAL_INT(0, feed(true, 50, BTN_EVT_RELEASE));
    TEST_ASSERT_EQUAL(BTN_PRESSED, d.state);
}
