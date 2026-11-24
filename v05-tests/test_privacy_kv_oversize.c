#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "radio/hk6b_privacy_cache.h"

static const uint8_t valid_record[14] = {
    1, 4,
    5, 0, 2,
    7, 0, 3,
    9, 0, 255,
    11, 0, 4
};

static uint8_t stored[15];
static int stored_length;
static unsigned reads;
static uint16_t last_capacity;

static int reader(uint16_t key, void *buf, uint16_t capacity)
{
    assert(key == HK6B_PRIVACY_KV_KEY);
    ++reads;
    last_capacity = capacity;

    if (stored_length < 0) {
        return -1;
    }

    unsigned n = (unsigned)stored_length;

    if (n > capacity) {
        n = capacity;
    }

    memcpy(buf, stored, n);
    return (int)n;
}

int main(void)
{
    hk6b_privacy_cache_t cache = {0};

    memcpy(stored, valid_record, sizeof valid_record);

    stored_length = 14;

    assert(hk6b_privacy_cache_refresh(&cache, 1, reader));
    assert(cache.valid);
    assert(!cache.malformed);
    assert(hk6b_privacy_cache_slot(&cache, 5) == 2);
    assert(last_capacity == 15);

    puts("01 EXACT 14-BYTE RECORD ACCEPTED: OK");

    stored[14] = 0xA5;
    stored_length = 15;

    assert(!hk6b_privacy_cache_refresh(&cache, 2, reader));
    assert(!cache.valid);
    assert(cache.malformed);
    assert(hk6b_privacy_cache_slot(&cache, 5) == 255);

    puts("02 EXTRA BYTE REJECTED: OK");

    unsigned previous_reads = reads;

    assert(!hk6b_privacy_cache_refresh(&cache, 2, reader));
    assert(reads == previous_reads);
    assert(cache.malformed);

    puts("03 NO DUPLICATE READ: OK");

    stored_length = -1;

    assert(!hk6b_privacy_cache_refresh(&cache, 3, reader));
    assert(!cache.valid);
    assert(!cache.malformed);
    assert(hk6b_privacy_cache_slot(&cache, 5) == 0);

    puts("04 ABSENT RECORD INHERITS GLOBAL: OK");

    stored_length = 14;

    assert(hk6b_privacy_cache_refresh(&cache, 4, reader));
    assert(cache.valid);
    assert(!cache.malformed);
    assert(hk6b_privacy_cache_slot(&cache, 7) == 3);

    puts("05 RECOVERY AFTER INVALID RECORD: OK");

    puts("HK6B V05 KV OVERSIZE: PASSED");
    return 0;
}