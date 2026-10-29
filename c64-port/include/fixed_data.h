/* SPDX-License-Identifier: MIT */
#ifndef APOLLO_FIXED_DATA_H
#define APOLLO_FIXED_DATA_H
#include <stdbool.h>
#include <stdint.h>

enum apollo_fixed_mission { APOLLO_FIXED_LM, APOLLO_FIXED_CM };
enum apollo_noun_table {
    APOLLO_NNADTAB, APOLLO_NNTYPTAB, APOLLO_SFINTAB,
    APOLLO_SFOUTAB, APOLLO_IDADDTAB, APOLLO_RUTMXTAB,
    APOLLO_NOUN_TABLE_COUNT
};
typedef struct apollo_fixed_table {
    uint16_t physical_fixed_word; /* Historical bank * 1024 + within-bank offset. */
    uint16_t word_count;
} apollo_fixed_table;

/* Immutable original data, returned as raw 15-bit source words. No RAM input
 * is read or cached. Unsupported missions/tables/addresses leave *out intact.
 * Only certified data spans are accessible; reader instruction words are not.
 * Define APOLLO_FIXED_LM_ONLY or APOLLO_FIXED_CM_ONLY to link one mission.
 * The data arrays may be linked inside native ROM overlays; their pointers
 * and this API remain valid only while that native module is resident. */
bool apollo_fixed_data_read(enum apollo_fixed_mission, uint16_t address,
                           uint16_t *out);
bool apollo_noun_table_info(enum apollo_fixed_mission, enum apollo_noun_table,
                           apollo_fixed_table *out);
bool apollo_noun_table_read(enum apollo_fixed_mission, enum apollo_noun_table,
                           uint16_t index, uint16_t *out);
#endif
