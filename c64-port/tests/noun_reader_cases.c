/* SPDX-License-Identifier: MIT
 * Input-only constructors. No expected output or native numerical body. */
#include "noun_reader_cases.h"
#include "fixed_data_symbols.h"

enum apollo_fixed_mission noun_reader_mission(uint16_t n)
{
    return n < NOUN_READER_LM_CASE_COUNT ? APOLLO_FIXED_LM : APOLLO_FIXED_CM;
}

enum apollo_noun_reader_entry noun_reader_entry(uint16_t n)
{
    uint16_t local = n < NOUN_READER_LM_CASE_COUNT ? n :
                     (uint16_t)(n - NOUN_READER_LM_CASE_COUNT);
    uint16_t parameter = (uint16_t)(local / 192u);
    uint16_t factors = n < NOUN_READER_LM_CASE_COUNT ? 30u : 21u;
    return parameter < 101u ? APOLLO_LODNNTAB :
           parameter < (uint16_t)(101u + factors) ? APOLLO_GTSFOUT : APOLLO_GTSFIN;
}

static uint16_t register_input(uint16_t word)
{
    return (uint16_t)(word | ((word & 040000u) << 1));
}

void noun_reader_prepare(apollo_overlay_state *s, apollo_noun_reader_io *io,
                         uint16_t n)
{
    uint16_t local = n < NOUN_READER_LM_CASE_COUNT ? n :
                     (uint16_t)(n - NOUN_READER_LM_CASE_COUNT);
    uint16_t parameter = (uint16_t)(local / 192u);
    uint16_t factors = n < NOUN_READER_LM_CASE_COUNT ? 30u : 21u;
    uint16_t bank = (uint16_t)(local % 8u), variant = (uint16_t)((local / 8u) % 6u);
    uint16_t entry, value, i;
    static const uint16_t callers[6] = {0, 1, 01377, 01400, 02070, 07777};
    static const uint16_t banks[6] = {0, 02000, 064000, 050000, 04000, 06000};
    for (i = 0; i < APOLLO_SHARED_WORDS; ++i)
        s->words[i] = (uint16_t)((251u * i + (local % 16u) * 947u + 99u) & 077777u);
    for (i = 0; i < 3u; ++i) s->words[i] = register_input(s->words[i]);
    s->words[0] = callers[variant];
    s->words[1] = register_input((uint16_t)(banks[variant] | ((bank + variant + 3u) & 7u)));
    if (variant == 3u) {
        s->words[0] ^= 040000u; /* Original first DXCH corrects both overflow signs. */
        s->words[1] ^= 040000u;
    }
    s->words[3] = (uint16_t)(bank << 8);
    s->words[4] = 064000;
    s->words[6] = (uint16_t)(064000u | bank);
    s->words[7] = 0;
    if (noun_reader_entry(n) == APOLLO_LODNNTAB) {
        value = parameter == 100u ? 077777u : parameter;
        s->words[APOLLO_LM_ERASABLE_WORD_NOUNREG] = value;
        entry = n < NOUN_READER_LM_CASE_COUNT ? APOLLO_LM_FIXED_WORD_LODNNTAB : APOLLO_CM_FIXED_WORD_LODNNTAB;
    } else {
        uint16_t code = (uint16_t)(parameter - 101u);
        if (code >= factors) code = (uint16_t)(code - factors);
        value = code == (uint16_t)(factors - 1u) ? 077777u : (uint16_t)(2u * code);
        s->words[APOLLO_LM_ERASABLE_WORD_SFTEMP1] = value;
        if (noun_reader_entry(n) == APOLLO_GTSFOUT)
            entry = n < NOUN_READER_LM_CASE_COUNT ? APOLLO_LM_FIXED_WORD_GTSFOUT : APOLLO_CM_FIXED_WORD_GTSFOUT;
        else
            entry = n < NOUN_READER_LM_CASE_COUNT ? APOLLO_LM_FIXED_WORD_GTSFIN : APOLLO_CM_FIXED_WORD_GTSFIN;
    }
    s->words[5] = (uint16_t)(02000u + entry % 1024u);
    io->superbank = (uint16_t)(0100u + 020u * ((local / 48u) % 4u));
    io->interrupt_enabled = (uint16_t)((parameter + variant) & 1u);
}
