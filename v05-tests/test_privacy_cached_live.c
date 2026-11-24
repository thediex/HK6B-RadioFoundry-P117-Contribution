#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "radio/hk6b_privacy_effective.h"

/* Controlled replacement for the settings storage backend. */
static settings_t stored;
static unsigned field_reads;

uint32_t settings_read_field(size_t offset, size_t size)
{
    uint32_t value = 0;

    assert(size == 1 || size == 2 || size == 4);
    assert(offset <= sizeof stored);
    assert(size <= sizeof stored - offset);

    ++field_reads;
    memcpy(&value, (const uint8_t *)&stored + offset, size);

    return value;
}

/* Controlled validator for Motorola BP: keys 1..255. */
bool dmrPrivacyKeyIsValid(dmrPrivacyType_t type, uint16_t key)
{
    if (type == DMR_PRIVACY_MOTOROLA_BP) {
        return key >= 1 && key <= 255;
    }

    if (type == DMR_PRIVACY_ANYTONE_BP) {
        return true;
    }

    return false;
}

static void expect(
    const hk6b_privacy_cache_t *cache,
    uint16_t channel,
    bool dmr,
    bool vfo,
    uint8_t global_type,
    uint16_t global_key,
    uint8_t expected_type,
    uint16_t expected_key)
{
    hk6b_privacy_effective_t effective =
        hk6b_privacy_resolve_cached_live(
            cache,
            channel,
            dmr,
            vfo,
            global_type,
            global_key
        );

    assert(effective.type == expected_type);
    assert(effective.key == expected_key);
}

int main(void)
{
    hk6b_privacy_cache_t cache = {0};

    cache.initialized = true;
    cache.valid = true;
    cache.map.count = 4;

    cache.map.channel[0] = 5;
    cache.map.slot[0] = 2;

    cache.map.channel[1] = 7;
    cache.map.slot[1] = 3;

    cache.map.channel[2] = 9;
    cache.map.slot[2] = 255;

    cache.map.channel[3] = 11;
    cache.map.slot[3] = 4;

    stored.privacy_type[0] = DMR_PRIVACY_MOTOROLA_BP;
    stored.privacy_key[0] = 45;

    stored.privacy_type[1] = DMR_PRIVACY_MOTOROLA_BP;
    stored.privacy_key[1] = 150;

    stored.privacy_type[2] = DMR_PRIVACY_MOTOROLA_BP;
    stored.privacy_key[2] = 120;

    stored.privacy_type[3] = DMR_PRIVACY_MOTOROLA_BP;
    stored.privacy_key[3] = 80;

    expect(&cache, 5, true, false,
           DMR_PRIVACY_OFF, 0,
           DMR_PRIVACY_MOTOROLA_BP, 150);

    puts("01 CHANNEL 5 USES SLOT 2: OK");

    expect(&cache, 7, true, false,
           DMR_PRIVACY_OFF, 0,
           DMR_PRIVACY_MOTOROLA_BP, 120);

    puts("02 CHANNEL 7 USES SLOT 3: OK");

    expect(&cache, 11, true, false,
           DMR_PRIVACY_OFF, 0,
           DMR_PRIVACY_MOTOROLA_BP, 80);

    puts("03 CHANNEL 11 USES SLOT 4: OK");

    expect(&cache, 9, true, false,
           DMR_PRIVACY_MOTOROLA_BP, 45,
           DMR_PRIVACY_OFF, 0);

    puts("04 EXPLICIT PRIVACY OFF: OK");

    unsigned before = field_reads;

    expect(&cache, 20, true, false,
           DMR_PRIVACY_MOTOROLA_BP, 45,
           DMR_PRIVACY_MOTOROLA_BP, 45);

    assert(field_reads == before);

    puts("05 UNMAPPED CHANNEL USES GLOBAL: OK");

    before = field_reads;

    expect(&cache, 5, true, true,
           DMR_PRIVACY_MOTOROLA_BP, 45,
           DMR_PRIVACY_MOTOROLA_BP, 45);

    assert(field_reads == before);

    puts("06 VFO PRESERVES GLOBAL: OK");

    expect(&cache, 5, false, false,
           DMR_PRIVACY_MOTOROLA_BP, 45,
           DMR_PRIVACY_MOTOROLA_BP, 45);

    puts("07 NON-DMR DOES NOT USE OVERRIDE: OK");

    expect(&cache, 0, true, false,
           DMR_PRIVACY_MOTOROLA_BP, 45,
           DMR_PRIVACY_MOTOROLA_BP, 45);

    puts("08 INVALID CHANNEL NUMBER: OK");

    cache.malformed = true;

    expect(&cache, 5, true, false,
           DMR_PRIVACY_MOTOROLA_BP, 45,
           DMR_PRIVACY_OFF, 0);

    puts("09 MALFORMED MAP DISABLES PRIVACY: OK");

    cache.malformed = false;
    cache.valid = false;

    expect(&cache, 5, true, false,
           DMR_PRIVACY_MOTOROLA_BP, 45,
           DMR_PRIVACY_MOTOROLA_BP, 45);

    puts("10 ABSENT MAP USES GLOBAL: OK");

    cache.valid = true;

    stored.privacy_key[1] = 300;

    expect(&cache, 5, true, false,
           DMR_PRIVACY_MOTOROLA_BP, 45,
           DMR_PRIVACY_OFF, 0);

    puts("11 INVALID MOTOROLA KEY REJECTED: OK");

    stored.privacy_key[1] = 70000;

    expect(&cache, 5, true, false,
           DMR_PRIVACY_MOTOROLA_BP, 45,
           DMR_PRIVACY_OFF, 0);

    puts("12 OVERSIZED KEY REJECTED: OK");

    stored.privacy_key[1] = 150;
    stored.privacy_type[1] = DMR_PRIVACY_OFF;

    expect(&cache, 5, true, false,
           DMR_PRIVACY_MOTOROLA_BP, 45,
           DMR_PRIVACY_OFF, 0);

    puts("13 EMPTY SLOT REJECTED: OK");

    stored.privacy_type[1] = DMR_PRIVACY_MOTOROLA_BP;

    cache.map.channel[0] = 6;
    cache.map.slot[0] = 1;

    expect(&cache, 6, true, false,
           DMR_PRIVACY_OFF, 0,
           DMR_PRIVACY_MOTOROLA_BP, 45);

    puts("14 EXPLICIT SLOT 1: OK");

    assert(field_reads > 0);

    puts("HK6B V05 CACHED LIVE RESOLVER: PASSED");
    return 0;
}