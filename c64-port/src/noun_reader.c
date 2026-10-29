/* SPDX-License-Identifier: MIT */
#include "noun_reader.h"
#include "fixed_data_symbols.h"
#include "agc_word.h"
#include <stddef.h>

/* Both original layouts give these shared scratch aliases the same physical
 * addresses. Guard this explicitly rather than silently assuming a CM copy. */
#define SAME_ADDRESS(name) typedef char noun_same_##name[ \
    APOLLO_LM_ERASABLE_WORD_##name == APOLLO_CM_ERASABLE_WORD_##name ? 1 : -1]
SAME_ADDRESS(NOUNREG);
SAME_ADDRESS(NNADTEM);
SAME_ADDRESS(NNTYPTEM);
SAME_ADDRESS(MIXBR);
SAME_ADDRESS(RUTMXTEM);
SAME_ADDRESS(IDAD1TEM);
SAME_ADDRESS(IDAD2TEM);
SAME_ADDRESS(IDAD3TEM);
SAME_ADDRESS(SFTEMP1);
SAME_ADDRESS(SFTEMP2);
#undef SAME_ADDRESS

static void source_return(apollo_overlay_state *state, uint16_t caller_pc,
                          uint16_t caller_bank, uint16_t source_next)
{
    uint16_t source_bank = state->words[6];
    state->words[0] = agc_extend(source_next);
    state->words[1] = agc_extend(source_bank);
    state->words[3] = (uint16_t)((caller_bank & 7u) << 8);
    state->words[4] = (uint16_t)(caller_bank & 076000u);
    state->words[5] = caller_pc;
    state->words[6] = (uint16_t)(caller_bank & 076007u);
}

bool apollo_noun_reader(apollo_overlay_state *state, apollo_noun_reader_io *io,
                       enum apollo_fixed_mission mission,
                       enum apollo_noun_reader_entry entry)
{
    uint16_t start, leave, noun, address, type, routine = 0, component[3];
    uint16_t factor[2], index, caller_pc, caller_bank;
    apollo_fixed_table table;
    if (state == NULL || io == NULL || (unsigned)entry > APOLLO_GTSFIN ||
        (unsigned)mission > APOLLO_FIXED_CM) return false;
    if (mission == APOLLO_FIXED_LM) {
        start = (uint16_t)(entry == APOLLO_LODNNTAB ? APOLLO_LM_FIXED_WORD_LODNNTAB :
                          entry == APOLLO_GTSFOUT ? APOLLO_LM_FIXED_WORD_GTSFOUT :
                                                   APOLLO_LM_FIXED_WORD_GTSFIN);
        leave = (uint16_t)(entry == APOLLO_LODNNTAB ? APOLLO_LM_FIXED_WORD_LODNLV :
                                                   APOLLO_LM_FIXED_WORD_SFCOM);
    } else {
        start = (uint16_t)(entry == APOLLO_LODNNTAB ? APOLLO_CM_FIXED_WORD_LODNNTAB :
                          entry == APOLLO_GTSFOUT ? APOLLO_CM_FIXED_WORD_GTSFOUT :
                                                   APOLLO_CM_FIXED_WORD_GTSFIN);
        leave = (uint16_t)(entry == APOLLO_LODNNTAB ? APOLLO_CM_FIXED_WORD_LODNLV :
                                                   APOLLO_CM_FIXED_WORD_SFCOM);
    }
    start = (uint16_t)(02000u + start % 1024u);
    leave = (uint16_t)(02000u + leave % 1024u + 2u);
    caller_pc = agc_correct(state->words[0]);
    caller_bank = agc_correct(state->words[1]);
    if (state->words[5] != start || state->words[4] != 064000u ||
        (state->words[3] & (uint16_t)~03400u) != 0 ||
        state->words[6] != (uint16_t)(064000u | (state->words[3] >> 8)) ||
        state->words[7] != 0 || (io->superbank & (uint16_t)~0160u) != 0 ||
        (io->superbank & 0100u) == 0 ||
        io->interrupt_enabled > 1u || caller_pc > 07777u)
        return false;
    if (entry == APOLLO_LODNNTAB) {
        uint16_t raw = state->words[APOLLO_LM_ERASABLE_WORD_NOUNREG];
        if (raw > AGC_MASK || (raw != AGC_NEG_ZERO && raw > 99u)) return false;
        noun = raw == AGC_NEG_ZERO ? 0 : raw;
        if (!apollo_noun_table_read(mission, APOLLO_NNADTAB, noun, &address) ||
            !apollo_noun_table_read(mission, APOLLO_NNTYPTAB, noun, &type))
            return false;
        if (noun >= 40u) {
            index = (uint16_t)(address & 01777u);
            if (!apollo_noun_table_read(mission, APOLLO_RUTMXTAB,
                                       (uint16_t)(noun - 40u), &routine) ||
                !apollo_noun_table_read(mission, APOLLO_IDADDTAB, index, &component[0]) ||
                !apollo_noun_table_read(mission, APOLLO_IDADDTAB,
                                       (uint16_t)(index + 1u), &component[1]) ||
                !apollo_noun_table_read(mission, APOLLO_IDADDTAB,
                                       (uint16_t)(index + 2u), &component[2])) return false;
        }
        state->words[APOLLO_LM_ERASABLE_WORD_NNADTEM] = address;
        state->words[APOLLO_LM_ERASABLE_WORD_NNTYPTEM] = type;
        state->words[APOLLO_LM_ERASABLE_WORD_MIXBR] = noun < 40u ? 1u : 2u;
        if (noun < 40u) {
            state->words[APOLLO_LM_ERASABLE_WORD_IDAD2TEM] = 1;
            state->words[2] = (uint16_t)(start + 14u);
        } else {
            state->words[APOLLO_LM_ERASABLE_WORD_RUTMXTEM] = routine;
            state->words[APOLLO_LM_ERASABLE_WORD_IDAD1TEM] = component[0];
            state->words[APOLLO_LM_ERASABLE_WORD_IDAD2TEM] = component[1];
            state->words[APOLLO_LM_ERASABLE_WORD_IDAD3TEM] = component[2];
            state->words[2] = (uint16_t)(address & 01777u);
        }
    } else {
        enum apollo_noun_table which = entry == APOLLO_GTSFOUT ? APOLLO_SFOUTAB :
                                                                 APOLLO_SFINTAB;
        uint16_t raw = state->words[APOLLO_LM_ERASABLE_WORD_SFTEMP1];
        if (raw > AGC_MASK || (raw != AGC_NEG_ZERO && (raw & 1u))) return false;
        index = raw == AGC_NEG_ZERO ? 0 : raw;
        if (!apollo_noun_table_info(mission, which, &table) ||
            index >= table.word_count || (uint16_t)(index + 1u) >= table.word_count ||
            !apollo_noun_table_read(mission, which, index, &factor[0]) ||
            !apollo_noun_table_read(mission, which, (uint16_t)(index + 1u), &factor[1]))
            return false;
        state->words[APOLLO_LM_ERASABLE_WORD_SFTEMP1] = factor[0];
        state->words[APOLLO_LM_ERASABLE_WORD_SFTEMP2] = factor[1];
    }
    source_return(state, caller_pc, caller_bank, leave);
    return true;
}
