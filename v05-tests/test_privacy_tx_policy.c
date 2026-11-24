#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "radio/hk6b_privacy_effective.h"

static settings_t stored;
uint32_t settings_read_field(size_t offset, size_t size) {
    uint32_t v = 0;
    assert(offset + size <= sizeof stored);
    memcpy(&v, (const char *)&stored + offset, size);
    return v;
}
bool dmrPrivacyKeyIsValid(dmrPrivacyType_t type, uint16_t key) {
    if (type == DMR_PRIVACY_MOTOROLA_BP) return key >= 1 && key <= 255;
    if (type == DMR_PRIVACY_ANYTONE_BP) return true;
    return false;
}
static void expected(hk6b_privacy_cache_t *c, uint8_t serial,
                     uint16_t channel, int vfo, uint8_t gt,
                     uint16_t gk, int want, const char *msg) {
    assert(hk6b_privacy_tx_profile_ok(c,serial,channel,vfo,gt,gk)==(bool)want);
    puts(msg);
}
int main(void) {
    hk6b_privacy_cache_t c = {0};
    expected(&c,1,5,0,DMR_PRIVACY_OFF,0,0,"01 UNINITIALIZED REJECTED");
    c.initialized = true; c.serial = 1; c.valid = true;
    c.map.count=3;
    c.map.channel[0]=5; c.map.slot[0]=2;
    c.map.channel[1]=6; c.map.slot[1]=255;
    c.map.channel[2]=7; c.map.slot[2]=3;
    stored.privacy_type[1] = DMR_PRIVACY_MOTOROLA_BP;
    stored.privacy_key[1] = 150;
    expected(&c,1,5,0,DMR_PRIVACY_OFF,0,1,"02 VALID MAPPED KEY ACCEPTED");
    expected(&c,1,6,0,DMR_PRIVACY_MOTOROLA_BP,20,1,"03 EXPLICIT OFF ACCEPTED");
    expected(&c,1,8,0,DMR_PRIVACY_MOTOROLA_BP,20,1,"04 UNMAPPED GLOBAL KEY ACCEPTED");
    expected(&c,1,8,0,DMR_PRIVACY_OFF,0,1,"05 GLOBAL INTENTIONAL OFF ACCEPTED");
    expected(&c,1,8,0,DMR_PRIVACY_MOTOROLA_BP,0,0,"06 INVALID GLOBAL KEY REJECTED");
    expected(&c,1,7,0,DMR_PRIVACY_OFF,0,0,"07 INVALID SLOT REJECTED");
    expected(&c,2,5,0,DMR_PRIVACY_OFF,0,0,"08 STALE SERIAL REJECTED");
    c.malformed=true;
    expected(&c,1,6,0,DMR_PRIVACY_OFF,0,0,"09 MALFORMED EVEN EXPLICIT OFF REJECTED");
    c.malformed=false;
    expected(&c,1,5,1,DMR_PRIVACY_MOTOROLA_BP,55,1,"10 VFO GLOBAL ACCEPTED");
    puts("HK6B V05 TX POLICY: PASSED");
    return 0;
}
