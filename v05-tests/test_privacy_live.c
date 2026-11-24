#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "radio/hk6b_privacy_effective.h"

static settings_t memoria;

uint32_t settings_read_field(size_t offset, size_t size)
{
    uint32_t value = 0;

    if ((size != 1 && size != 2 && size != 4) ||
        offset > sizeof(memoria) ||
        size > sizeof(memoria) - offset) {
        return 0;
    }

    memcpy(&value, ((const unsigned char *)&memoria) + offset, size);
    return value;
}

/* Validador aislado: no sustituye el motor DMR real. */
bool dmrPrivacyKeyIsValid(dmrPrivacyType_t type, uint16_t key)
{
    return type == DMR_PRIVACY_MOTOROLA_BP &&
           key >= 1 && key <= 255;
}

int main(void)
{
    memset(&memoria, 0, sizeof(memoria));

    memoria.privacy_type[0] = DMR_PRIVACY_MOTOROLA_BP;
    memoria.privacy_key[0] = 150;

    memoria.privacy_type[1] = DMR_PRIVACY_MOTOROLA_BP;
    memoria.privacy_key[1] = 120;

    memoria.hk6b_privacy_channel[0] = 5;
    memoria.hk6b_privacy_slot[0] = 1;

    memoria.hk6b_privacy_channel[1] = 7;
    memoria.hk6b_privacy_slot[1] = 2;

    settings_t original = memoria;

    hk6b_privacy_effective_t a =
        hk6b_privacy_resolve_live(5, true, false, 0, 0);

    hk6b_privacy_effective_t b =
        hk6b_privacy_resolve_live(7, true, false, 0, 0);

    hk6b_privacy_effective_t c =
        hk6b_privacy_resolve_live(8, true, false, 0, 0);

    assert(a.type == DMR_PRIVACY_MOTOROLA_BP && a.key == 150);
    assert(b.type == DMR_PRIVACY_MOTOROLA_BP && b.key == 120);
    assert(c.type == DMR_PRIVACY_OFF && c.key == 0);

    /* Verificar que los canales sin asociacion conservan
       la configuracion global. */
    hk6b_privacy_effective_t global =
        hk6b_privacy_resolve_live(
            8, true, false, DMR_PRIVACY_MOTOROLA_BP, 55);

    assert(global.type == DMR_PRIVACY_MOTOROLA_BP);
    assert(global.key == 55);

    /* Comprobar que el VFO conserva su configuracion global. */
    hk6b_privacy_effective_t vfo =
        hk6b_privacy_resolve_live(
            5, true, true, DMR_PRIVACY_MOTOROLA_BP, 55);

    assert(vfo.key == 55);

    /* Verificar que no se modifico la configuracion. */
    assert(memcmp(&memoria, &original, sizeof(memoria)) == 0);

    printf("CANAL 5: MOTOROLA BP 150 OK\n");
    printf("CANAL 7: MOTOROLA BP 120 OK\n");
    printf("CANAL 8: PRIVACIDAD GLOBAL OK\n");
    printf("VFO: PRIVACIDAD GLOBAL OK\n");
    printf("ALMACENAMIENTO: SIN MODIFICACIONES\n");
    printf("HK6B V05 LIVE TEST: OK\n");

    return 0;
}