#ifndef HK6B_PRIVACY_MAP_EDIT_H
#define HK6B_PRIVACY_MAP_EDIT_H

#include "radio/hk6b_privacy_kv.h"

// Modify one association in a copy. Slot 0 removes the explicit entry.
// False on invalid input or a full record; input is unchanged on failure.
static inline bool hk6b_privacy_map_change(hk6b_privacy_kv_map_t *map, uint16_t ch, uint8_t slot)
{
    if (map == NULL || ch < 1u || ch > 1024u ||
        !(slot == 0u || slot == 255u || (slot >= 1u && slot <= 4u)) ||
        map->count > HK6B_PRIVACY_KV_MAX) return false;
    hk6b_privacy_kv_map_t next = *map;
    unsigned pos = 0;
    while (pos < next.count && next.channel[pos] != ch) pos++;
    if (slot == 0u) {
        if (pos == next.count) return true;
        for (unsigned j = pos + 1u; j < next.count; ++j) {
            next.channel[j - 1u] = next.channel[j];
            next.slot[j - 1u] = next.slot[j];
        }
        next.count--;
    } else if (pos < next.count) {
        next.slot[pos] = slot;
    } else {
        if (next.count >= HK6B_PRIVACY_KV_MAX) return false;
        next.channel[next.count] = ch;
        next.slot[next.count++] = slot;
    }
    *map = next;
    return true;
}

#endif
