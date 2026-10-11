/* SPDX-License-Identifier: MIT */
#include "layout_check.h"
#include "layout_vectors.h"
#include <stddef.h>

static uint16_t input_word(uint16_t i)
{
    return (uint16_t)((47u * i + 0x3412u) & 077777u);
}

bool layout_check(apollo_overlay_state *image, volatile uint16_t *output)
{
    uint16_t row, word, physical, expected, old, probe, bank, logical;
    volatile uint16_t *cell;
    for (word = 0; word < APOLLO_SHARED_WORDS; ++word)
        image->words[word] = input_word(word);
    for (row = 0; row < LAYOUT_BINDINGS; ++row) {
        word = layout_native_bindings[row];
        expected = layout_expected[row];
        if (output != NULL) output[row] = word;
        if (word != expected) return false;
        if (layout_kind[row] == 1u || layout_kind[row] == 4u) {
            cell = apollo_layout_cell(image, word);
            if (cell == NULL || *cell != input_word(expected)) return false;
            old = *cell;
            probe = (uint16_t)((row * 17u + 0x4567u) & 077777u);
            *cell = probe; /* Volatile accesses require actual target RAM stores. */
            if (*(volatile uint16_t *)&image->words[expected] != probe) return false;
            *cell = old;
        }
    }
    for (bank = 0; bank < 8u; ++bank) {
        for (logical = 0; logical < 1024u; ++logical) {
            expected = logical < 768u ? logical : (uint16_t)(256u * bank + logical - 768u);
            physical = 0xdead;
            if (!apollo_layout_erasable_index(logical, (uint16_t)(bank * 256u), &physical) ||
                physical != expected || apollo_layout_cell(image, physical) != &image->words[expected])
                return false;
        }
    }
    for (row = 0; row < LAYOUT_METADATA_WORDS; ++row) {
        enum apollo_fixed_mission mission = row < 36u ? APOLLO_FIXED_LM : APOLLO_FIXED_CM;
        word = 0xbeef;
        if (!apollo_layout_fixed_read(mission, layout_metadata_address[row], &word) ||
            word != layout_metadata_value[row]) return false;
    }
    for (word = 0; word < 3u; ++word) {
        cell = apollo_layout_cell(image, word);
        *cell = (uint16_t)(0x8123u + word * 123u);
        if (*(volatile uint16_t *)&image->words[word] != (uint16_t)(0x8123u + word * 123u)) return false;
        *cell = input_word(word);
    }
    if (!apollo_layout_materialize_emissions(APOLLO_FIXED_LM, image)) return false;
    for (word = 0; word < APOLLO_SHARED_WORDS; ++word)
        if (image->words[word] != input_word(word)) return false;
    if (!apollo_layout_materialize_emissions(APOLLO_FIXED_CM, image)) return false;
    for (word = 0; word < APOLLO_SHARED_WORDS; ++word) {
        expected = word == 01356u ? 021356u : input_word(word);
        if (image->words[word] != expected) return false;
    }
    return true;
}
