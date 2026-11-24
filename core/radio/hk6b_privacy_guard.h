#ifndef HK6B_PRIVACY_GUARD_H
#define HK6B_PRIVACY_GUARD_H

#include <stdbool.h>

/*
 * HK6B V05: conservative guard for deferred DMR privacy changes.
 * The caller must obtain all states from the radio task.
 * This helper neither reads flash nor changes radio configuration.
 */
typedef struct {
    bool radio_idle;
    bool engine_idle;
    bool phase_idle;
    bool ptt;
    bool dmr_keyed;
    bool tx_no_ptt;
    bool hotspot;
    bool sfr_active;
    bool trunk_active;
    bool data_busy;
    bool roaming;
    bool emergency;
} hk6b_privacy_guard_t;

static inline bool hk6b_privacy_can_apply(
    const hk6b_privacy_guard_t *g)
{
    if (g == 0) {
        return false;
    }

    return g->radio_idle &&
           g->engine_idle &&
           g->phase_idle &&
           !g->ptt &&
           !g->dmr_keyed &&
           !g->tx_no_ptt &&
           !g->hotspot &&
           !g->sfr_active &&
           !g->trunk_active &&
           !g->data_busy &&
           !g->roaming &&
           !g->emergency;
}

#endif