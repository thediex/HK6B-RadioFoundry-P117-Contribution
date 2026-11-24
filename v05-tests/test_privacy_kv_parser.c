#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "radio/hk6b_privacy_kv.h"

static void reject(const uint8_t *bytes, size_t len)
{
    hk6b_privacy_kv_map_t map;
    memset(&map, 0xA5, sizeof map);

    assert(!hk6b_privacy_kv_parse(&map, bytes, len));
    assert(map.count == 0);
    assert(hk6b_privacy_kv_slot(&map, 5) == 0);
}

int main(void)
{
    hk6b_privacy_kv_map_t map = {0};

    const uint8_t good[] = {
        1, 2,
        5, 0, 2,
        7, 0, 3
    };

    assert(HK6B_PRIVACY_KV_KEY == 0x1E00u);
    assert(HK6B_PRIVACY_KV_SIZE == 14u);

    assert(hk6b_privacy_kv_parse(
        &map, good, sizeof good));

    assert(map.count == 2);
    assert(hk6b_privacy_kv_slot(&map, 5) == 2);
    assert(hk6b_privacy_kv_slot(&map, 7) == 3);
    assert(hk6b_privacy_kv_slot(&map, 9) == 0);

    puts("01 CPS BINARY FORMAT: OK");

    const uint8_t disabled[] = {
        1, 1, 9, 0, 255
    };

    assert(hk6b_privacy_kv_parse(
        &map, disabled, sizeof disabled));
    assert(hk6b_privacy_kv_slot(&map, 9) == 255);

    puts("02 EXPLICITLY DISABLED: OK");

    const uint8_t empty[] = {1, 0};

    assert(hk6b_privacy_kv_parse(
        &map, empty, sizeof empty));
    assert(map.count == 0);

    puts("03 EMPTY MAP: OK");

    const uint8_t duplicate[] = {
        1, 2, 5, 0, 2, 5, 0, 3
    };

    reject(duplicate, sizeof duplicate);
    puts("04 DUPLICATE CHANNEL REJECTED: OK");

    const uint8_t wrongVersion[] = {2, 0};
    reject(wrongVersion, sizeof wrongVersion);
    puts("05 UNKNOWN VERSION REJECTED: OK");

    reject(good, sizeof good - 1);
    puts("06 TRUNCATED RECORD REJECTED: OK");

    const uint8_t badSlot[] = {1, 1, 5, 0, 0};
    reject(badSlot, sizeof badSlot);
    puts("07 INVALID SLOT REJECTED: OK");

    const uint8_t badChannel[] = {1, 1, 0, 0, 2};
    reject(badChannel, sizeof badChannel);
    puts("08 INVALID CHANNEL REJECTED: OK");

    const uint8_t tooMany[] = {1, 5};
    reject(tooMany, sizeof tooMany);
    puts("09 EXCESSIVE COUNT REJECTED: OK");

    const uint8_t trailing[] = {1, 0, 99};
    reject(trailing, sizeof trailing);
    puts("10 EXTRA BYTES REJECTED: OK");

    puts("HK6B V05 C KV PARSER: PASSED");
    return 0;
}