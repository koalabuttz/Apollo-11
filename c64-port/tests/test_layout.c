/* SPDX-License-Identifier: MIT */
#include "layout_check.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static apollo_overlay_state image, before;

int main(void)
{
    uint16_t word, value;
    unsigned bad;
    assert(layout_check(&image, NULL));
    assert(APOLLO_CM_ERASABLE_WORD_ALTVAR == 01357);
    assert(APOLLO_CM_ERASABLE_WORD_ALTVAR == APOLLO_CM_ERASABLE_WORD_SELFERAS);
    assert(APOLLO_CM_ERASABLE_WORD_ALTVAR == APOLLO_CM_ERASABLE_WORD_SFAIL);
    assert(APOLLO_CM_FIXED_WORD_R53FLBIT == 05010);
    assert(APOLLO_LAYOUT_SOURCE_FIXED_BANKS == 36u);
    assert(APOLLO_CM_LAYOUT_UNAVAILABLE_PSEUDO_FIRST == 0120000u);
    assert(APOLLO_CM_LAYOUT_UNAVAILABLE_PSEUDO_LAST == 0167777u);
    assert(APOLLO_CM_LAYOUT_UNAVAILABLE_BANK_FIRST == 36u);
    assert(APOLLO_CM_LAYOUT_UNAVAILABLE_BANK_LAST == 55u);
    for (word = 0; word < 2048u; ++word)
        assert(apollo_layout_cell(&image, word) == &image.words[word]);
    for (bad = 2048u; bad < 65536u; ++bad)
        assert(apollo_layout_cell(&image, (uint16_t)bad) == NULL);
    assert(apollo_layout_cell(NULL, 0) == NULL);
    before = image;
    assert(!apollo_layout_materialize_emissions((enum apollo_fixed_mission)-1, &image));
    assert(!apollo_layout_materialize_emissions((enum apollo_fixed_mission)2, &image));
    assert(memcmp(&image, &before, sizeof image) == 0);
    assert(!apollo_layout_materialize_emissions(APOLLO_FIXED_CM, NULL));
    for (bad = 0; bad < 65536u; ++bad) {
        if (bad > 01777u) {
            value = 0xbeef;
            assert(!apollo_layout_erasable_index((uint16_t)bad, 0, &value));
            assert(value == 0xbeef);
        }
        if (bad & (unsigned)~03400u) {
            value = 0xbeef;
            assert(!apollo_layout_erasable_index(01400, (uint16_t)bad, &value));
            assert(value == 0xbeef);
        }
    }
    assert(!apollo_layout_erasable_index(0, 0, NULL));
    value = 0xbeef;
    assert(!apollo_layout_fixed_read((enum apollo_fixed_mission)-1, 0, &value));
    assert(!apollo_layout_fixed_read((enum apollo_fixed_mission)2, 0, &value));
    assert(value == 0xbeef);
    assert(!apollo_layout_fixed_read(APOLLO_FIXED_LM, 0, NULL));
    puts("PASS: 4232 original bindings, 8192 bank contexts, 79 fixed metadata words; raw single-image aliases and original CM emission");
    return 0;
}
