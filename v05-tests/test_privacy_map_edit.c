#include <assert.h>
#include <stdio.h>
#include "radio/hk6b_privacy_map_edit.h"
int main(void)
{
    hk6b_privacy_kv_map_t m = {0};
    assert(hk6b_privacy_map_change(&m, 7, 1) && m.count == 1 && hk6b_privacy_kv_slot(&m, 7) == 1);
    assert(hk6b_privacy_map_change(&m, 8, 2) && m.count == 2 && hk6b_privacy_kv_slot(&m, 8) == 2);
    assert(hk6b_privacy_map_change(&m, 7, 3) && m.count == 2 && hk6b_privacy_kv_slot(&m, 7) == 3);
    assert(hk6b_privacy_map_change(&m, 9, 255) && hk6b_privacy_kv_slot(&m, 9) == 255);
    assert(hk6b_privacy_map_change(&m, 10, 4) && m.count == 4);
    hk6b_privacy_kv_map_t before = m;
    assert(!hk6b_privacy_map_change(&m, 11, 1));
    assert(m.count == before.count && hk6b_privacy_kv_slot(&m, 7) == 3);
    assert(!hk6b_privacy_map_change(&m, 0, 1) && !hk6b_privacy_map_change(&m, 1025, 1));
    assert(!hk6b_privacy_map_change(&m, 7, 5) && !hk6b_privacy_map_change(&m, 7, 254));
    assert(hk6b_privacy_map_change(&m, 8, 0) && hk6b_privacy_kv_slot(&m, 8) == 0 && m.count == 3);
    assert(hk6b_privacy_map_change(&m, 8, 0) && m.count == 3);
    assert(hk6b_privacy_map_change(&m, 11, 1) && m.count == 4);
    puts("P116 map edit tests PASSED");
    return 0;
}
