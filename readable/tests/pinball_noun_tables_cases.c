/* SPDX-License-Identifier: MIT */
/* The readable noun tables and their readers against the rope and the
 * faithful port's (make pinball-noun-tables-cases, its third step).
 *
 * First every word of both missions' six tables against the rope, word by
 * word (the ropes are the arguments, LM then CM). Then the readers side by
 * side with the faithful port's (c64-port/src/noun_reader.c, built as
 * faithful_apollo_noun_reader): every one of the port's 58,368 cases
 * (c64-port/tests/noun_reader_cases.c: every noun and scale factor of both
 * missions, under eight E-banks, six callers and four superbank settings),
 * as prepared and stirred 15 more ways: the entry, the bank registers,
 * ZERO, the latches, the return 2CADR in A and L, the noun or scale factor
 * index, the words the readers park in, any other erasable word, and a
 * word they do not read holding a 16th bit. Where
 * either reader runs the other must too, with the same memory and
 * latches, word for word; where they refuse, both leave everything as it
 * was. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "noun_reader_cases.h"
#include "fixed_data_symbols.h"
#include "rope_image.h"

bool faithful_apollo_noun_reader(apollo_overlay_state *, apollo_noun_reader_io *,
                                 enum apollo_fixed_mission, enum apollo_noun_reader_entry);
bool readable_lm_noun_reader(apollo_overlay_state *, apollo_noun_reader_io *,
                             enum apollo_fixed_mission, enum apollo_noun_reader_entry);
bool readable_cm_noun_reader(apollo_overlay_state *, apollo_noun_reader_io *,
                             enum apollo_fixed_mission, enum apollo_noun_reader_entry);
uint16_t readable_lm_noun_table_word(unsigned physical);
uint16_t readable_cm_noun_table_word(unsigned physical);

#define VARIANTS   16u
#define NOUNREG_AT APOLLO_LM_ERASABLE_WORD_NOUNREG
#define SFTEMP1_AT APOLLO_LM_ERASABLE_WORD_SFTEMP1

static uint32_t seed = 20690720u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* A register's 16 bits for a 15-bit word, sign-extended. */
static uint16_t reg(uint16_t w)
{
    return (uint16_t)((w & 077777u) | ((w & 040000u) << 1));
}

static void stir(apollo_overlay_state *s, apollo_noun_reader_io *io, unsigned v)
{
    static const uint16_t indexes[] = {0,      2, 39, 40, 99, 100, 101, 077777,
                                       077776, 1, 56, 57, 58, 38,  40,  076543};
    switch (v) {
    case 0: break;
    case 1: s->words[5] = (uint16_t)(s->words[5] + (rnd() % 2u ? 1u : 077777u)); break; /* Z */
    case 2:
        s->words[4] = (uint16_t)((rnd() % 040u) << 10);
        s->words[6] = (uint16_t)(s->words[4] | (s->words[3] >> 8));
        break;
    case 3:
        s->words[3] = (uint16_t)(rnd() % 4u ? (rnd() % 8u) << 8 : rnd() & 03777u);
        break;                                                          /* EB */
    case 4: s->words[6] ^= (uint16_t)(rnd() % 2u ? 1u : 02000u); break; /* BB */
    case 5: s->words[7] = (uint16_t)(1u + rnd() % 077777u); break;      /* ZERO */
    case 6: io->superbank = (uint16_t)(rnd() % 4u ? (rnd() % 8u) * 020u : rnd() % 0400u); break;
    case 7: io->interrupt_enabled = (uint16_t)(rnd() % 3u); break;
    case 8:
        s->words[0] = rnd() % 2u ? (uint16_t)(rnd() % 010000u) : (uint16_t)(rnd() | rnd() << 1);
        break;                                                                             /* A */
    case 9: s->words[1] = rnd() % 2u ? reg(rnd()) : (uint16_t)(rnd() | rnd() << 1); break; /* L */
    case 10:
        s->words[NOUNREG_AT] = rnd() % 2u ? indexes[rnd() % 16u] : (uint16_t)(rnd() & 077777u);
        break;
    case 11:
        s->words[SFTEMP1_AT] = rnd() % 2u ? indexes[rnd() % 16u] : (uint16_t)(rnd() & 077777u);
        break;
    case 12: /* the words the readers park the return in, and the next */
        s->words[APOLLO_LM_ERASABLE_WORD_IDAD2TEM] = (uint16_t)(rnd() & 077777u);
        s->words[APOLLO_LM_ERASABLE_WORD_IDAD3TEM] = (uint16_t)(rnd() & 077777u);
        s->words[APOLLO_LM_ERASABLE_WORD_SFTEMP2] = (uint16_t)(rnd() & 077777u);
        break;
    case 13: s->words[2] = rnd() % 2u ? reg(rnd()) : (uint16_t)(rnd() | rnd() << 1); break; /* Q */
    case 14: s->words[8u + rnd() % (APOLLO_SHARED_WORDS - 8u)] = (uint16_t)(rnd() & 077777u); break;
    default: { /* a word the readers do not read with its 16th bit set */
        unsigned w;
        do w = 0200u + rnd() % (APOLLO_SHARED_WORDS - 0200u);
        while (w == NOUNREG_AT);
        s->words[w] |= 0100000u;
        break;
    }
    }
}

/* Every word of the mission's tables against the rope. */
static unsigned tables_differ(unsigned mission, uint16_t (*word_at)(unsigned), unsigned *words)
{
    unsigned t, i, differ = 0;

    for (t = 0; t < APOLLO_NOUN_TABLE_COUNT; ++t) {
        apollo_fixed_table table;
        if (!apollo_noun_table_info((enum apollo_fixed_mission)mission, (enum apollo_noun_table)t,
                                    &table)) {
            printf("FAIL %s table %u: no such table\n", mission ? "CM" : "LM", t);
            ++differ;
            continue;
        }
        for (i = 0; i < table.word_count; ++i) {
            unsigned physical = table.physical_fixed_word + i;
            agc_word rope;
            if (!apollo_rope_read(mission, (uint16_t)physical, &rope) ||
                word_at(physical) != rope) {
                if (++differ <= 8u)
                    printf("FAIL %s table %u word %u: %05o, the rope's %05o\n",
                           mission ? "CM" : "LM", t, i, word_at(physical), rope);
            }
            ++*words;
        }
    }
    return differ;
}

int main(int argc, char **argv)
{
    static apollo_overlay_state start, f, r;
    unsigned n, v, words = 0, runs = 0, ran = 0, failures = 0, by_variant[VARIANTS] = {0};

    if (argc != 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        fprintf(stderr, "usage: %s LM-rope CM-rope\n", argv[0]);
        return 2;
    }
    failures += tables_differ(0, readable_lm_noun_table_word, &words);
    failures += tables_differ(1, readable_cm_noun_table_word, &words);
    printf("pinball-noun-tables-cases: %u table words, %u not the rope's\n", words, failures);

    /* Each mission's reader refuses the other mission's calls (the flight
     * program of one mission links only its own), leaving the state. */
    for (n = 0; n < NOUN_READER_CASE_COUNT; n += 97u) {
        apollo_noun_reader_io io, rio;
        enum apollo_fixed_mission mission = noun_reader_mission((uint16_t)n);
        bool ran;

        noun_reader_prepare(&start, &io, (uint16_t)n);
        r = start;
        rio = io;
        ran = mission == APOLLO_FIXED_LM
                  ? readable_cm_noun_reader(&r, &rio, mission, noun_reader_entry((uint16_t)n))
                  : readable_lm_noun_reader(&r, &rio, mission, noun_reader_entry((uint16_t)n));
        if (ran || memcmp(&r, &start, sizeof r) || memcmp(&rio, &io, sizeof rio)) {
            if (++failures <= 8u) printf("FAIL case %u: the other mission's reader ran\n", n);
        }
    }

    for (n = 0; n < NOUN_READER_CASE_COUNT; ++n)
        for (v = 0; v < VARIANTS; ++v) {
            apollo_noun_reader_io io, fio, rio;
            enum apollo_fixed_mission mission = noun_reader_mission((uint16_t)n);
            enum apollo_noun_reader_entry entry = noun_reader_entry((uint16_t)n);
            bool fok, rok;
            const char *why = NULL;

            if (v && n % 4u != v % 4u) continue; /* (stirred: a quarter of the cases each way) */
            noun_reader_prepare(&start, &io, (uint16_t)n);
            stir(&start, &io, v);
            f = r = start;
            fio = rio = io;
            fok = faithful_apollo_noun_reader(&f, &fio, mission, entry);
            rok = apollo_noun_reader(&r, &rio, mission, entry);
            ++runs;
            ran += fok;
            if (fok != rok)
                why = fok ? "readable refused, faithful ran" : "readable ran, faithful refused";
            else if (memcmp(&f, &r, sizeof f) || memcmp(&fio, &rio, sizeof fio))
                why = "different memory or latches";
            else if (!rok && (memcmp(&r, &start, sizeof r) || memcmp(&rio, &io, sizeof rio)))
                why = "a refusal changed the state";
            if (why) {
                unsigned w;
                ++by_variant[v];
                if (++failures <= 8u) {
                    printf("FAIL case %u variant %u (%s, entry %u): %s\n", n, v,
                           mission ? "CM" : "LM", (unsigned)entry, why);
                    for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                        if (f.words[w] != r.words[w])
                            printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, r.words[w],
                                   f.words[w], start.words[w]);
                }
            }
        }
    for (v = 0; v < VARIANTS; ++v)
        if (by_variant[v]) printf("  variant %u: %u failures\n", v, by_variant[v]);
    printf("pinball-noun-tables-cases: %u runs, %u run by the faithful reader, %u failures\n", runs,
           ran, failures);
    return failures != 0;
}
