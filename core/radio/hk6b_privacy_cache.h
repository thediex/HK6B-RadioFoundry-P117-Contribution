#ifndef HK6B_PRIVACY_CACHE_H
#define HK6B_PRIVACY_CACHE_H

#include <stdbool.h>
#include <stdint.h>

#include "radio/hk6b_privacy_kv.h"

/*
 * HK6B V05: cached channel privacy associations.
 *
 * Reader signature matches codeplug_kv_get().
 * A serial change invalidates the previous snapshot.
 * Missing or malformed records produce an empty map.
 *
 * All access must be serialized by the caller.
 */

typedef int (*hk6b_privacy_read_fn)(
    uint16_t key,
    void *buffer,
    uint16_t capacity
);

typedef struct {
    hk6b_privacy_kv_map_t map;
    uint8_t serial;
    bool initialized;
    bool valid;
    bool malformed;
} hk6b_privacy_cache_t;

static inline void hk6b_privacy_cache_reset(
    hk6b_privacy_cache_t *cache)
{
    if (cache == NULL) {
        return;
    }

    *cache = (hk6b_privacy_cache_t){0};
}

static inline bool hk6b_privacy_cache_refresh(
    hk6b_privacy_cache_t *cache,
    uint8_t serial,
    hk6b_privacy_read_fn reader)
{
    if (cache == NULL) {
        return false;
    }

    if (cache->initialized && cache->serial == serial) {
        return cache->valid;
    }

    cache->initialized = true;
    cache->serial = serial;
    cache->valid = false;
    cache->malformed = false;
    cache->map = (hk6b_privacy_kv_map_t){0};

    if (reader == NULL) {
        return false;
    }

    uint8_t bytes[HK6B_PRIVACY_KV_SIZE + 1u] = {0};

    int len = reader(
        HK6B_PRIVACY_KV_KEY,
        bytes,
        (uint16_t)sizeof bytes
    );

    if (len < 0) {
        return false;
    }

    if ((unsigned)len > HK6B_PRIVACY_KV_SIZE) {
        cache->malformed = true;
        return false;
    }

    cache->valid = hk6b_privacy_kv_parse(
        &cache->map,
        bytes,
        (size_t)len
    );

    cache->malformed = !cache->valid;
    return cache->valid;
}

static inline uint8_t hk6b_privacy_cache_slot(
    const hk6b_privacy_cache_t *cache,
    uint16_t channel)
{
    if (cache == NULL) {
        return 0u;
    }

    if (cache->malformed) {
        return 255u;
    }

    if (!cache->valid) {
        return 0u;
    }

    return hk6b_privacy_kv_slot(&cache->map, channel);
}

#endif