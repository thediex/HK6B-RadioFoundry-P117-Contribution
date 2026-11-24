#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "radio/hk6b_privacy_guard.h"

static hk6b_privacy_guard_t idle_guard(void)
{
    hk6b_privacy_guard_t g = {0};

    g.radio_idle = true;
    g.engine_idle = true;
    g.phase_idle = true;

    return g;
}

#define BLOCKED(field, number, description) do { \
    hk6b_privacy_guard_t g = idle_guard(); \
    g.field = true; \
    assert(!hk6b_privacy_can_apply(&g)); \
    printf("%02d %s: OK\n", number, description); \
} while (0)

int main(void)
{
    hk6b_privacy_guard_t g = idle_guard();

    assert(hk6b_privacy_can_apply(&g));
    puts("01 FULLY IDLE RADIO: OK");

    g.radio_idle = false;
    assert(!hk6b_privacy_can_apply(&g));
    puts("02 ACTIVE RADIO STATE BLOCKED: OK");

    g = idle_guard();
    g.engine_idle = false;
    assert(!hk6b_privacy_can_apply(&g));
    puts("03 ACTIVE DMR ENGINE BLOCKED: OK");

    g = idle_guard();
    g.phase_idle = false;
    assert(!hk6b_privacy_can_apply(&g));
    puts("04 ACTIVE DMR PHASE BLOCKED: OK");

    BLOCKED(ptt, 5, "PTT BLOCKED");
    BLOCKED(dmr_keyed, 6, "RF KEYED BLOCKED");
    BLOCKED(tx_no_ptt, 7, "AUTOMATIC TX BLOCKED");
    BLOCKED(hotspot, 8, "HOTSPOT BLOCKED");
    BLOCKED(sfr_active, 9, "SFR BLOCKED");
    BLOCKED(trunk_active, 10, "TRUNK BLOCKED");
    BLOCKED(data_busy, 11, "DATA SERVICE BLOCKED");
    BLOCKED(roaming, 12, "ROAMING BLOCKED");
    BLOCKED(emergency, 13, "EMERGENCY BLOCKED");

    assert(!hk6b_privacy_can_apply(NULL));
    puts("14 NULL GUARD BLOCKED: OK");

    g = idle_guard();
    g.ptt = true;
    g.data_busy = true;

    assert(!hk6b_privacy_can_apply(&g));

    g.ptt = false;
    assert(!hk6b_privacy_can_apply(&g));

    g.data_busy = false;
    assert(hk6b_privacy_can_apply(&g));

    puts("15 DEFER UNTIL ALL CONDITIONS CLEAR: OK");

    puts("HK6B V05 PRIVACY GUARD: PASSED");
    return 0;
}