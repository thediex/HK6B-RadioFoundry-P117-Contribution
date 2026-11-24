#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "radio/hk6b_privacy_effective.h"

/* Stub exclusivo para la prueba aislada.
   El firmware real utiliza dmrPrivacyKeyIsValid(). */
bool dmrPrivacyKeyIsValid(dmrPrivacyType_t type, uint16_t key)
{
    if (type == DMR_PRIVACY_MOTOROLA_BP)
        return key >= 1 && key <= 255;
    return false;
}

int main(void)
{
    settings_t s;
    memset(&s, 0, sizeof(s));

    s.privacy_active = 0;

    s.privacy_type[0] = DMR_PRIVACY_MOTOROLA_BP;
    s.privacy_key[0] = 150;

    s.privacy_type[1] = DMR_PRIVACY_MOTOROLA_BP;
    s.privacy_key[1] = 120;

    s.hk6b_privacy_channel[0] = 5;
    s.hk6b_privacy_slot[0] = 1;

    s.hk6b_privacy_channel[1] = 7;
    s.hk6b_privacy_slot[1] = 2;

    settings_t original = s;

    hk6b_privacy_effective_t a =
        hk6b_privacy_resolve(&s, 5, true, false);

    hk6b_privacy_effective_t b =
        hk6b_privacy_resolve(&s, 7, true, false);

    hk6b_privacy_effective_t c =
        hk6b_privacy_resolve(&s, 8, true, false);

    assert(a.type == DMR_PRIVACY_MOTOROLA_BP);
    assert(a.key == 150);

    assert(b.type == DMR_PRIVACY_MOTOROLA_BP);
    assert(b.key == 120);

    assert(c.type == DMR_PRIVACY_OFF);
    assert(c.key == 0);

    /* Ninguna ranura ni asociacion debe modificarse. */
    assert(memcmp(&s, &original, sizeof(s)) == 0);

    printf("CANAL 5: BP %u OK\n", a.key);
    printf("CANAL 7: BP %u OK\n", b.key);
    printf("CANAL 8: PRIVACIDAD OFF OK\n");
    printf("CLAVES ALMACENADAS: SIN MODIFICACIONES\n");
    printf("HK6B V05 FUNCTIONAL TEST: OK\n");

    return 0;
}