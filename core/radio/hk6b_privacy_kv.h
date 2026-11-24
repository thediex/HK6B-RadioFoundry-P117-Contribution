#ifndef HK6B_PRIVACY_KV_H
#define HK6B_PRIVACY_KV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * HK6B V05: channel-to-privacy-slot mapping.
 *
 * KV key: 0x1E00
 * Payload: version(1), count(1), entries(count * 3)
 * Entry: channel number LE16, slot U8
 *
 * Slot 1..4: stored privacy key slot
 * Slot 255: explicitly disable privacy
 * No entry: inherit global privacy
 *
 * No privacy key material is stored here.
 */

#define HK6B_PRIVACY_KV_KEY      0x1E00u
#define HK6B_PRIVACY_KV_VERSION  1u
#define HK6B_PRIVACY_KV_MAX      4u
#define HK6B_PRIVACY_KV_SIZE     (2u + HK6B_PRIVACY_KV_MAX * 3u)

typedef struct {
    uint16_t channel[HK6B_PRIVACY_KV_MAX];
    uint8_t slot[HK6B_PRIVACY_KV_MAX];
    uint8_t count;
} hk6b_privacy_kv_map_t;

static inline bool hk6b_privacy_kv_parse(
    hk6b_privacy_kv_map_t *out,
    const uint8_t *data,
    size_t len)
{
    if (out == NULL) {
        return false;
    }

    hk6b_privacy_kv_map_t parsed = {0};

    if (data == NULL || len < 2u ||
        data[0] != HK6B_PRIVACY_KV_VERSION) {
        *out = parsed;
        return false;
    }

    uint8_t count = data[1];

    if (count > HK6B_PRIVACY_KV_MAX ||
        len != 2u + (size_t)count * 3u) {
        *out = parsed;
        return false;
    }

    for (unsigned i = 0; i < count; ++i) {
        size_t at = 2u + i * 3u;

        uint16_t channel =
            (uint16_t)((uint16_t)data[at] |
            ((uint16_t)data[at + 1u] << 8));

        uint8_t slot = data[at + 2u];

        if (channel < 1u || channel > 1024u ||
            !((slot >= 1u && slot <= 4u) ||
              slot == 255u)) {
            *out = (hk6b_privacy_kv_map_t){0};
            return false;
        }

        for (unsigned j = 0; j < i; ++j) {
            if (parsed.channel[j] == channel) {
                *out = (hk6b_privacy_kv_map_t){0};
                return false;
            }
        }

        parsed.channel[i] = channel;
        parsed.slot[i] = slot;
    }

    parsed.count = count;
    *out = parsed;
    return true;
}

static inline uint8_t hk6b_privacy_kv_slot(
    const hk6b_privacy_kv_map_t *map,
    uint16_t channel)
{
    if (map == NULL || channel == 0u) {
        return 0u;
    }

    for (unsigned i = 0; i < map->count &&
         i < HK6B_PRIVACY_KV_MAX; ++i) {
        if (map->channel[i] == channel) {
            return map->slot[i];
        }
    }

    return 0u;
}

#endif