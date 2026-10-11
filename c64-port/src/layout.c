/* SPDX-License-Identifier: MIT */
#include "layout.h"
#include <stddef.h>
#include "layout_data.inc"

uint16_t *apollo_layout_cell(apollo_overlay_state *image, uint16_t physical_word)
{
    if (image == NULL || physical_word >= APOLLO_SHARED_WORDS) return NULL;
    return &image->words[physical_word];
}

bool apollo_layout_erasable_index(uint16_t logical_word, uint16_t eb,
                                  uint16_t *physical_word)
{
    if (physical_word == NULL || logical_word > 01777u ||
        (eb & (uint16_t)~03400u) != 0) return false;
    *physical_word = logical_word < 01400u ? logical_word :
                     (uint16_t)(eb | (logical_word & 0377u));
    return true;
}

bool apollo_layout_fixed_read(enum apollo_fixed_mission mission,
                              uint16_t physical_word, uint16_t *value)
{
    const apollo_layout_fixed_word *data;
    uint16_t count, i;
    if (value == NULL || (unsigned)mission > APOLLO_FIXED_CM) return false;
    data = mission == APOLLO_FIXED_LM ? apollo_layout_lm_fixed : apollo_layout_cm_fixed;
    count = mission == APOLLO_FIXED_LM ? APOLLO_LAYOUT_LM_FIXED_WORDS : APOLLO_LAYOUT_CM_FIXED_WORDS;
    for (i = 0; i < count; ++i) {
        if (data[i].physical_word == physical_word) {
            *value = data[i].value;
            return true;
        }
    }
    return false;
}

bool apollo_layout_materialize_emissions(enum apollo_fixed_mission mission,
                                        apollo_overlay_state *image)
{
    if (image == NULL || (unsigned)mission > APOLLO_FIXED_CM) return false;
    if (mission == APOLLO_FIXED_CM)
        image->words[apollo_layout_cm_erasable_emission.physical_word] =
            apollo_layout_cm_erasable_emission.value;
    return true;
}
