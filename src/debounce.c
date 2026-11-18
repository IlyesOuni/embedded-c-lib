#include "debounce.h"

void debounce_init(debounce_t *d, uint8_t threshold_ms)
{
    d->state     = BTN_RELEASED;
    d->counter   = 0U;
    d->threshold = (threshold_ms < 2U) ? 2U : threshold_ms;  /* minimum 2 */
}

btn_event_t debounce_update(debounce_t *d, bool raw_level)
{
    btn_event_t evt = BTN_EVT_NONE;

    switch (d->state) {

    case BTN_RELEASED:
        if (raw_level) {                     /* ça bouge ! on commence à compter */
            d->state   = BTN_MAYBE_PRESSED;
            d->counter = 1U;
        }
        break;

    case BTN_MAYBE_PRESSED:
        if (raw_level) {                     /* toujours appuyé */
            d->counter++;
            if (d->counter >= d->threshold) {
                d->state = BTN_PRESSED;      /* confirmé ! */
                evt      = BTN_EVT_PRESS;
            }
        } else {
            d->state = BTN_RELEASED;         /* c'était du bruit */
        }
        break;

    case BTN_PRESSED:
        if (!raw_level) {                    /* ça bouge ! on commence à compter */
            d->state   = BTN_MAYBE_RELEASED;
            d->counter = 1U;
        }
        break;

    case BTN_MAYBE_RELEASED:
        if (!raw_level) {                    /* toujours relâché */
            d->counter++;
            if (d->counter >= d->threshold) {
                d->state = BTN_RELEASED;     /* confirmé ! */
                evt      = BTN_EVT_RELEASE;
            }
        } else {
            d->state = BTN_PRESSED;          /* c'était du bruit */
        }
        break;

    default:                                 /* état corrompu (bug, rayonnement...) */
        d->state = BTN_RELEASED;             /* on revient à un état sûr */
        break;
    }

    return evt;
}