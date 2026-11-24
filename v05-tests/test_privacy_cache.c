#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "radio/hk6b_privacy_cache.h"

static unsigned reads;
static int record_length = -1;
static uint8_t record[32];

static int mock_reader(
    uint16_t key,
    void *buffer,
    uint16_t capacity)
{
    assert(key == HK6B_PRIVACY_KV_KEY);

    ++reads;

    if (record_length < 0) {
        return -1;
    }

    unsigned copy = (unsigned)record_length;

    if (copy > capacity) {
        copy = capacity;
    }

    memcpy(buffer, record, copy);
    return record_length;
}

static void set_record(
    const uint8_t *data,
    unsigned length)
{
    assert(length <= sizeof record);
    memcpy(record, data, length);
    record_length = (int)length;
}

int main(void)
{
    hk6b_privacy_cache_t cache = {0};

    const uint8_t initial[] = {
        1, 2, 5, 0, 2, 7, 0, 3
    };

    set_record(initial, sizeof initial);

    assert(hk6b_privacy_cache_refresh(
        &cache, 10, mock_reader));

    assert(reads == 1);
    assert(hk6b_privacy_cache_slot(&cache, 5) == 2);
    assert(hk6b_privacy_cache_slot(&cache, 7) == 3);

    puts("01 INITIAL KV LOAD: OK");

    assert(hk6b_privacy_cache_refresh(
        &cache, 10, mock_reader));

    assert(reads == 1);

    puts("02 NO REPEATED FLASH READ: OK");

    const uint8_t updated[] = {
        1, 2, 5, 0, 3, 7, 0, 255
    };

    set_record(updated, sizeof updated);

    assert(hk6b_privacy_cache_refresh(
        &cache, 11, mock_reader));

    assert(reads == 2);
    assert(hk6b_privacy_cache_slot(&cache, 5) == 3);
    assert(hk6b_privacy_cache_slot(&cache, 7) == 255);

    puts("03 SERIAL CHANGE REFRESH: OK");

    const uint8_t invalid[] = {
        1, 2, 5, 0, 2
    };

    set_record(invalid, sizeof invalid);

    assert(!hk6b_privacy_cache_refresh(
        &cache, 12, mock_reader));

    assert(reads == 3);
    assert(!cache.valid);
    assert(cache.malformed);
    assert(cache.map.count == 0);
    assert(hk6b_privacy_cache_slot(&cache, 5) == 255);

    puts("04 INVALID RECORD CLEARS CACHE: OK");

    record_length = -1;

    assert(!hk6b_privacy_cache_refresh(
        &cache, 13, mock_reader));

    assert(reads == 4);
    assert(!cache.malformed);
    assert(hk6b_privacy_cache_slot(&cache, 7) == 0);

    puts("05 REMOVED RECORD CLEARS CACHE: OK");

    set_record(initial, sizeof initial);

    assert(hk6b_privacy_cache_refresh(
        &cache, 14, mock_reader));

    assert(reads == 5);
    assert(hk6b_privacy_cache_slot(&cache, 5) == 2);

    puts("06 VALID RECORD RECOVERY: OK");

    hk6b_privacy_cache_reset(&cache);

    assert(!cache.initialized);
    assert(!cache.valid);
    assert(hk6b_privacy_cache_slot(&cache, 5) == 0);

    puts("07 EXPLICIT CACHE RESET: OK");

    assert(hk6b_privacy_cache_refresh(
        &cache, 14, mock_reader));

    assert(reads == 6);

    puts("08 RESET FORCES RELOAD: OK");

    const uint8_t incompatible[] = {2, 0};
    set_record(incompatible, sizeof incompatible);

    assert(!hk6b_privacy_cache_refresh(
        &cache, 15, mock_reader));
    assert(cache.malformed);
    assert(hk6b_privacy_cache_slot(&cache, 5) == 255);

    puts("09 INCOMPATIBLE VERSION FAILS CLOSED: OK");

    record_length = -1;

    assert(!hk6b_privacy_cache_refresh(
        &cache, 16, mock_reader));
    assert(!cache.malformed);
    assert(hk6b_privacy_cache_slot(&cache, 5) == 0);

    puts("10 ABSENT RECORD USES GLOBAL: OK");
    puts("HK6B V05 PRIVACY CACHE: PASSED");

    return 0;
}