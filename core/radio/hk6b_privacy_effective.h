#ifndef HK6B_PRIVACY_EFFECTIVE_H
#define HK6B_PRIVACY_EFFECTIVE_H

#include <stdbool.h>
#include <stdint.h>

#include "settings/settings.h"
#include "radio/hk6b_privacy_slots.h"
#include "radio/hk6b_privacy_cache.h"
#include "dmr/dmrPrivacy.h"

typedef struct {
    uint8_t type;
    uint16_t key;
} hk6b_privacy_effective_t;

static inline hk6b_privacy_effective_t
hk6b_privacy_resolve(
    const settings_t *settings,
    uint16_t channel_number,
    bool is_dmr,
    bool is_vfo)
{
    hk6b_privacy_effective_t result = {0, 0};

    if (settings == NULL || !is_dmr) {
        return result;
    }

    uint8_t slot = hk6b_privacy_channel_slot(
        settings,
        channel_number,
        is_dmr,
        is_vfo
    );

    if (slot == 255) {
        return result;
    }

    if (slot == 0) {
        slot = settings->privacy_active;
    }

    if (slot < 1 || slot > SETTINGS_PRIVACY_KEYS) {
        return result;
    }

    unsigned index = slot - 1;

    uint8_t type = settings->privacy_type[index];
    uint16_t key = (uint16_t)settings->privacy_key[index];

    if (type == DMR_PRIVACY_OFF ||
        type >= DMR_PRIVACY_NUM_TYPES ||
        !dmrPrivacyKeyIsValid((dmrPrivacyType_t)type, key)) {
        return result;
    }

    result.type = type;
    result.key = key;

    return result;
}


// HK6B V05: live resolver for the radio task.
// Reads only the necessary settings fields, avoiding a 420-byte stack copy.
// Global privacy remains separate from the channel override.
static inline hk6b_privacy_effective_t
hk6b_privacy_resolve_live(
    uint16_t channel_number,
    bool is_dmr,
    bool is_vfo,
    uint8_t global_type,
    uint16_t global_key)
{
    hk6b_privacy_effective_t result = {
        global_type, global_key
    };

    if (!is_dmr || is_vfo || channel_number == 0) {
        return result;
    }

    uint8_t slot = 0;

    for (unsigned i = 0; i < SETTINGS_PRIVACY_KEYS; ++i) {
        size_t offset =
            offsetof(settings_t, hk6b_privacy_channel) +
            i * sizeof(uint16_t);

        uint16_t mapped_channel =
            (uint16_t)settings_read_field(offset, sizeof(uint16_t));

        if (mapped_channel != channel_number) {
            continue;
        }

        offset = offsetof(settings_t, hk6b_privacy_slot) + i;

        uint8_t candidate =
            (uint8_t)settings_read_field(offset, sizeof(uint8_t));

        if ((candidate >= 1 && candidate <= SETTINGS_PRIVACY_KEYS) ||
            candidate == 255) {
            slot = candidate;
            break;
        }
    }

    if (slot == 0) {
        return result;
    }

    result.type = DMR_PRIVACY_OFF;
    result.key = 0;

    if (slot == 255) {
        return result;
    }

    unsigned index = slot - 1;

    uint8_t type = (uint8_t)settings_read_field(
        offsetof(settings_t, privacy_type) + index,
        sizeof(uint8_t)
    );

    uint32_t stored_key = settings_read_field(
        offsetof(settings_t, privacy_key) +
            index * sizeof(uint32_t),
        sizeof(uint32_t)
    );

    if (stored_key > UINT16_MAX ||
        type == DMR_PRIVACY_OFF ||
        type >= DMR_PRIVACY_NUM_TYPES ||
        !dmrPrivacyKeyIsValid((dmrPrivacyType_t)type,
                              (uint16_t)stored_key)) {
        return result;
    }

    result.type = type;
    result.key = (uint16_t)stored_key;

    return result;
}

// HK6B V05: resolve DMR privacy using the CPS KV cache.
// No flash read for the channel-to-slot association.
// Global privacy remains the fallback for unmapped channels.
static inline hk6b_privacy_effective_t
hk6b_privacy_resolve_cached_live(
    const hk6b_privacy_cache_t *cache,
    uint16_t channel_number,
    bool is_dmr,
    bool is_vfo,
    uint8_t global_type,
    uint16_t global_key)
{
    hk6b_privacy_effective_t result = {
        global_type, global_key
    };

    if (!is_dmr || is_vfo || channel_number == 0) {
        return result;
    }

    uint8_t slot = hk6b_privacy_cache_slot(
        cache, channel_number
    );

    if (slot == 0) {
        return result;
    }

    result.type = DMR_PRIVACY_OFF;
    result.key = 0;

    if (slot == 255) {
        return result;
    }

    if (slot < 1 || slot > SETTINGS_PRIVACY_KEYS) {
        return result;
    }

    unsigned index = slot - 1u;

    uint8_t type = (uint8_t)settings_read_field(
        offsetof(settings_t, privacy_type) + index,
        sizeof(uint8_t)
    );

    uint32_t stored_key = settings_read_field(
        offsetof(settings_t, privacy_key) +
            index * sizeof(uint32_t),
        sizeof(uint32_t)
    );

    if (stored_key > UINT16_MAX ||
        type == DMR_PRIVACY_OFF ||
        type >= DMR_PRIVACY_NUM_TYPES ||
        !dmrPrivacyKeyIsValid(
            (dmrPrivacyType_t)type,
            (uint16_t)stored_key)) {
        return result;
    }

    result.type = type;
    result.key = (uint16_t)stored_key;

    return result;
}

/* P105: a configured profile must not silently fall back to clear TX.
 * A deliberate explicit OFF (slot 255) and an absent mapping are distinct.
 * The serial must match the KV snapshot before any new TX starts.
 * Read errors are treated as "absent" by the current KV API: a full
 * storage health/error distinction remains future work.
 */
static inline bool hk6b_privacy_tx_profile_ok(
    const hk6b_privacy_cache_t *cache,
    uint8_t current_serial,
    uint16_t channel_number,
    bool is_vfo,
    uint8_t global_type,
    uint16_t global_key)
{
    if (cache == NULL || !cache->initialized ||
        cache->serial != current_serial || cache->malformed) {
        return false;
    }

    uint8_t slot = is_vfo ? 0u :
        hk6b_privacy_cache_slot(cache, channel_number);

    if (slot == 255u) {
        return true; /* intentionally clear */
    }

    hk6b_privacy_effective_t effective =
        hk6b_privacy_resolve_cached_live(cache, channel_number,
            true, is_vfo, global_type, global_key);

    if (slot >= 1u && slot <= SETTINGS_PRIVACY_KEYS) {
        return effective.type != DMR_PRIVACY_OFF;
    }

    if (slot != 0u) {
        return false;
    }

    if (global_type == DMR_PRIVACY_OFF) {
        return true; /* intentionally unconfigured global */
    }

    return effective.type != DMR_PRIVACY_OFF &&
           effective.type < DMR_PRIVACY_NUM_TYPES &&
           dmrPrivacyKeyIsValid((dmrPrivacyType_t)effective.type,
                                effective.key);
}

#endif