/* SPDX-License-Identifier: MIT */
/* The readable interrupt lead-ins, RESUME exits and (LM) T6 interrupt
 * against the faithful port's, case by case (make rupt-cases).
 *
 * Built once per mission, with the readable tables (src/binding/
 * rupt_routines.c) and the faithful port's under the same names with
 * faithful_ in front (the Makefile renames them). Each entry of each table
 * runs on 65,536 random states: random memory with the bank registers
 * coherent, mostly inside the interrupt, the 2CADRs the DTCB lead-ins read
 * mostly in fixed memory, TIME5 near its half-second mark, ZRUPT and
 * BANKRUPT mostly what an interrupt leaves (BRUPT the word at ZRUPT -1, or
 * not), and for the T6 routines TIME6 mostly -0 with NXT6ADR naming a
 * writer, the queue sometimes empty (POSMAX) and Q a fixed return address
 * or not. Where either port runs, the other must too, with the same
 * memory, io record and channels, word for word; where the readable one
 * refuses, it must leave everything as it found it. The faithful routines
 * are certified against the original program (c64-port/tests/
 * test_rupt_lead_ins.c, test_lm_t6rupt.c), as the readable ones are by make
 * rupt-cases' first step. */
#include <stdio.h>
#include <string.h>
#include "rupt_lead_ins.h"
#include "dntm_rupt_routines.h"
#if LUMINARY099
#include "lm_t6rupt_routines.h"
#include "p66_bias.h"
typedef p66_bias_io mission_io;
#define MISSION "LM"
extern const agc_routine_table faithful_lm_lead_in_routines, faithful_lm_lead_in_hot_routines,
    faithful_lm_t6rupt_routines, faithful_lm_t6rupt_hot_routines,
    faithful_lm_t6rupt_jets_hot_routines, faithful_lm_dntm_rupt_routines;
static const struct {
    const char *name;
    const agc_routine_table *readable, *faithful;
} tables[] = {
    {"lm_lead_in_routines", &lm_lead_in_routines, &faithful_lm_lead_in_routines},
    {"lm_lead_in_hot_routines", &lm_lead_in_hot_routines, &faithful_lm_lead_in_hot_routines},
    {"lm_t6rupt_routines", &lm_t6rupt_routines, &faithful_lm_t6rupt_routines},
    {"lm_t6rupt_hot_routines", &lm_t6rupt_hot_routines, &faithful_lm_t6rupt_hot_routines},
    {"lm_t6rupt_jets_hot_routines", &lm_t6rupt_jets_hot_routines,
     &faithful_lm_t6rupt_jets_hot_routines},
    {"lm_dntm_rupt_routines", &lm_dntm_rupt_routines, &faithful_lm_dntm_rupt_routines}};
#else
#include "cm_native_image.h"
typedef cm_native_io mission_io;
#define MISSION "CM"
extern const agc_routine_table faithful_cm_lead_in_routines, faithful_cm_dntm_rupt_routines;
static const struct {
    const char *name;
    const agc_routine_table *readable, *faithful;
} tables[] = {{"cm_lead_in_routines", &cm_lead_in_routines, &faithful_cm_lead_in_routines},
              {"cm_dntm_rupt_routines", &cm_dntm_rupt_routines, &faithful_cm_dntm_rupt_routines}};
#endif

#define CASES 65536u

static uint32_t seed = 173205u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static uint16_t sign_extended(uint16_t w)
{
    return (uint16_t)((w & 077777u) | ((w & 040000u) << 1));
}

/* A 16-bit register: mostly a sign-extended word, sometimes an overflow. */
static uint16_t reg(void)
{
    return rnd() % 16u ? sign_extended(rnd()) : (uint16_t)(rnd() | rnd() << 1);
}

/* A fixed address, mostly; sometimes erasable. */
static uint16_t cadr(void)
{
    return rnd() % 16u ? (uint16_t)(02000u + rnd() % 06000u) : (uint16_t)(rnd() % 02000u);
}

/* A BBANK word: mostly FBANK and EBANK bits only. */
static uint16_t bank_word(void)
{
    uint16_t bb = (uint16_t)((rnd() % 040u) << 10 | (rnd() % 8u));
    return rnd() % 16u ? bb : (uint16_t)(rnd() & 077777u);
}

/* The physical index of erasable address a under EBANK eb. */
static unsigned physical(uint16_t a, uint16_t eb)
{
    return a < 01400u ? a : ((eb >> 8) & 7u) * 0400u + (a - 01400u);
}

static void state(apollo_overlay_state *s, mission_io *io, agc_hardware *hw, uint16_t z,
                  uint16_t fb)
{
    unsigned i;
    uint16_t eb = (uint16_t)(rnd() % 4u ? 6u : rnd() % 8u), zrupt;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg();
    s->words[1] = sign_extended(rnd());
    s->words[2] = rnd() % 8u ? (uint16_t)(02000u + rnd() % 06000u) : reg(); /* Q */
    s->words[3] = (uint16_t)(eb << 8);
    s->words[4] = rnd() % 16u ? fb : (uint16_t)((rnd() % 040u) << 10);
    s->words[5] = z;
    s->words[6] = (uint16_t)(s->words[4] | eb);
    if (rnd() % 64u == 0u) s->words[7] = 1u;
    /* The 2CADRs: LM T5ADR, CM T6LOC and T5LOC. */
    s->words[01274] = cadr();
    s->words[01310] = cadr();
    s->words[01312] = cadr();
    /* TIME5 near half a second (062) from overflow, either side. */
    if (rnd() % 2u) s->words[030] = (uint16_t)(rnd() % 0144u);
    /* TIME6: -0 (due), +0, counting, or negative. */
    switch (rnd() % 8u) {
    case 0:
    case 1:
    case 2:
    case 3: s->words[031] = 077777u; break;
    case 4: s->words[031] = 0u; break;
    case 5:
    case 6: s->words[031] = (uint16_t)(rnd() % 040000u); break;
    default: s->words[031] = (uint16_t)(040000u | rnd()); break;
    }
    /* The T6 queue in EBANK 6: NXT6ADR a writer, T6NEXT/T6FURTHA times
     * or POSMAX. */
    {
        static const uint16_t writer[3] = {0u, 4u, 13u};
        s->words[03000u + 065u] = rnd() % 16u ? writer[rnd() % 3u] : (uint16_t)(rnd() % 16u);
        s->words[03000u + 066u] = rnd() % 4u ? (uint16_t)(rnd() % 040000u) : 037777u;
        s->words[03000u + 070u] = rnd() % 4u ? (uint16_t)(rnd() % 040000u) : 037777u;
    }
    /* ZRUPT, BANKRUPT and BRUPT as the interrupt left them, mostly. */
    zrupt = rnd() % 4u ? (uint16_t)(1u + rnd() % 01777u) : (uint16_t)(02001u + rnd() % 06000u);
    if (rnd() % 32u == 0u) zrupt = rnd() % 2u ? 0u : (uint16_t)(010001u + rnd() % 0100u);
    s->words[015] = zrupt;
    s->words[016] = bank_word();
    if (zrupt >= 2u && zrupt <= 02000u && rnd() % 4u) {
        uint16_t bb = rnd() % 2u ? s->words[016] : s->words[6];
        s->words[017] = s->words[physical((uint16_t)(zrupt - 1u), (uint16_t)((bb & 7u) << 8))];
    }
    io->superbank = (uint16_t)((rnd() % 8u) * 020u);
    io->interrupt_enabled = (uint16_t)(rnd() % 2u);
    hw->in_interrupt = (uint16_t)(rnd() % 32u ? 1u : rnd() % 3u);
    hw->channel5 = (uint16_t)(rnd() & 077777u);
    hw->channel6 = (uint16_t)(rnd() & 077777u);
    hw->channel13 = (uint16_t)(rnd() & 043777u);
}

static unsigned compare(const char *name, const agc_routine_table *readable,
                        const agc_routine_table *faithful)
{
    unsigned i, n, failures = 0;

    if (readable->count != faithful->count) {
        printf("FAIL %s: %u entries, faithful %u\n", name, readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z, fb;
        agc_routine f = faithful->entries[i].call, r = readable->entries[i].call;
        unsigned ran = 0, bad = 0;

        if (readable->entries[i].physical != key) {
            printf("FAIL %s: entry %u at key %u, faithful %u\n", name, i,
                   readable->entries[i].physical, key);
            ++failures;
            continue;
        }
        if (key >= 2048u && key < 4096u) { /* fixed-fixed */
            z = (uint16_t)(04000u + (key - 2048u));
            fb = (uint16_t)((rnd() % 040u) << 10);
        } else {
            z = (uint16_t)(02000u + key % 1024u);
            fb = (uint16_t)((key / 1024u) << 10);
        }
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            mission_io io, fio, rio;
            agc_hardware hw, fhw, rhw;
            agc_native_program fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, &hw, z, fb);
            fa = ra = start;
            fio = rio = io;
            fhw = rhw = hw;
            memset(&fp, 0, sizeof fp);
            memset(&rp, 0, sizeof rp);
            fp.hardware = &fhw;
            rp.hardware = &rhw;
            if (n % 64u == 63u) fp.hardware = rp.hardware = 0; /* no hardware record */
            sf = f(&fp, &fa, &fio);
            sr = r(&rp, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio) ||
                                  memcmp(&rhw, &hw, sizeof rhw)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
            else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if (sf == EXEC_OK &&
                     (memcmp(&fio, &rio, sizeof fio) || memcmp(&fhw, &rhw, sizeof fhw)))
                why = "different io or channels";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL %s %05o case %u: %s\n", name, z, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran == 0u && bad == 0u) {
            printf("FAIL %s %05o: the faithful routine never ran\n", name, z);
            ++bad;
        }
        failures += bad;
        printf("%s %s %05o: %u cases, %u run by the faithful routine, %u failures\n", MISSION, name,
               z, CASES, ran, bad);
    }
    return failures;
}

int main(void)
{
    unsigned t, failures = 0;

    for (t = 0; t < sizeof tables / sizeof tables[0]; ++t)
        failures += compare(tables[t].name, tables[t].readable, tables[t].faithful);
    printf("rupt-cases %s: %u failures\n", MISSION, failures);
    return failures != 0;
}
