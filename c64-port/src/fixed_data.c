/* SPDX-License-Identifier: MIT */
#include "fixed_data.h"
#include <stddef.h>

#if defined(APOLLO_FIXED_LM_ONLY) && defined(APOLLO_FIXED_CM_ONLY)
#error "Select at most one fixed-data mission"
#endif
typedef struct fixed_span {
    uint16_t address, count;
    const uint16_t *words;
} fixed_span;
#if !defined(APOLLO_FIXED_CM_ONLY)
#include "fixed_data_lm.inc"
#endif
#if !defined(APOLLO_FIXED_LM_ONLY)
#include "fixed_data_cm.inc"
#endif

static const fixed_span *spans(enum apollo_fixed_mission mission, size_t *count)
{
    switch (mission) {
#if !defined(APOLLO_FIXED_CM_ONLY)
    case APOLLO_FIXED_LM:
        *count = sizeof(fixed_data_lm_spans) / sizeof(fixed_data_lm_spans[0]);
        return fixed_data_lm_spans;
#endif
#if !defined(APOLLO_FIXED_LM_ONLY)
    case APOLLO_FIXED_CM:
        *count = sizeof(fixed_data_cm_spans) / sizeof(fixed_data_cm_spans[0]);
        return fixed_data_cm_spans;
#endif
    default: return NULL;
    }
}

bool apollo_fixed_data_read(enum apollo_fixed_mission mission, uint16_t address,
                           uint16_t *out)
{
    size_t count = 0, i;
    const fixed_span *list;
    if (out == NULL) return false;
    list = spans(mission, &count);
    for (i = 0; i < count; ++i) {
        if (address >= list[i].address &&
            (uint16_t)(address - list[i].address) < list[i].count) {
            *out = list[i].words[address - list[i].address];
            return true;
        }
    }
    return false;
}

bool apollo_noun_table_info(enum apollo_fixed_mission mission,
                           enum apollo_noun_table table, apollo_fixed_table *out)
{
    const apollo_fixed_table *list;
    if (out == NULL || (unsigned)table >= APOLLO_NOUN_TABLE_COUNT) return false;
    switch (mission) {
#if !defined(APOLLO_FIXED_CM_ONLY)
    case APOLLO_FIXED_LM: list = fixed_data_lm_nouns; break;
#endif
#if !defined(APOLLO_FIXED_LM_ONLY)
    case APOLLO_FIXED_CM: list = fixed_data_cm_nouns; break;
#endif
    default: return false;
    }
    *out = list[(unsigned)table];
    return true;
}

bool apollo_noun_table_read(enum apollo_fixed_mission mission,
                           enum apollo_noun_table table, uint16_t index,
                           uint16_t *out)
{
    apollo_fixed_table info;
    if (!apollo_noun_table_info(mission, table, &info) || index >= info.word_count)
        return false;
    return apollo_fixed_data_read(mission,
                                (uint16_t)(info.physical_fixed_word + index), out);
}
