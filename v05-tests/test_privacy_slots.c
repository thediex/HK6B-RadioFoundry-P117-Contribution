#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "radio/hk6b_privacy_slots.h"

int main(void)
{
    settings_t s;
    memset(&s, 0, sizeof(s));

    // Existing channels keep the global configuration.
    assert(hk6b_privacy_channel_slot(&s, 5, true, false) == 0);

    // Laboratory example: channel 5 -> slot 1.
    s.hk6b_privacy_channel[0] = 5;
    s.hk6b_privacy_slot[0] = 1;

    // Laboratory example: channel 7 -> slot 2.
    s.hk6b_privacy_channel[1] = 7;
    s.hk6b_privacy_slot[1] = 2;

    assert(hk6b_privacy_channel_slot(&s, 5, true, false) == 1);
    assert(hk6b_privacy_channel_slot(&s, 7, true, false) == 2);
    assert(hk6b_privacy_channel_slot(&s, 8, true, false) == 0);

    // No per-channel override on FM or VFO.
    assert(hk6b_privacy_channel_slot(&s, 5, false, false) == 0);
    assert(hk6b_privacy_channel_slot(&s, 5, true, true) == 0);

    // Explicitly disabled privacy.
    s.hk6b_privacy_slot[1] = 255;
    assert(hk6b_privacy_channel_slot(&s, 7, true, false) == 255);

    // Invalid mapping must not select an arbitrary slot.
    s.hk6b_privacy_slot[1] = 99;
    assert(hk6b_privacy_channel_slot(&s, 7, true, false) == 0);

    puts("HK6B V05 PRIVACY SLOT TEST: OK");
    return 0;
}