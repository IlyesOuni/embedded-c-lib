#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <stdint.h>
#include <stdbool.h>

/* Les 4 états possibles */
typedef enum {
    BTN_RELEASED,
    BTN_MAYBE_PRESSED,
    BTN_PRESSED,
    BTN_MAYBE_RELEASED
} btn_state_t;

/* Ce que la fonction annonce */
typedef enum {
    BTN_EVT_NONE,      /* rien de nouveau    */
    BTN_EVT_PRESS,     /* appui confirmé     */
    BTN_EVT_RELEASE    /* relâchement confirmé */
} btn_event_t;

typedef struct {
    btn_state_t state;      /* état actuel                       */
    uint8_t     counter;    /* depuis combien de ms c'est stable */
    uint8_t     threshold;  /* durée de stabilité exigée (ms)    */
} debounce_t;

void        debounce_init(debounce_t *d, uint8_t threshold_ms);
btn_event_t debounce_update(debounce_t *d, bool raw_level);  /* toutes les 1 ms */

#endif