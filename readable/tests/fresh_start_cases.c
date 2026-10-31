/* SPDX-License-Identifier: MIT */
/* The readable fresh start and restart against the faithful port's, case
 * by case (make fresh-start-cases).
 *
 * Built once per mission, with the readable table (src/binding/
 * fresh_start_routines.c) and the faithful port's under the same names
 * with faithful_ in front (the Makefile renames them). Each case is one of
 * the port's test states (c64-port/tests/<m>_fresh_start_cases.c: every
 * entry, restart groups active, inactive or corrupt, ERESTORE set or not),
 * half of them stirred further: any Q, interrupts, SUPERBNK, the ZERO
 * register, the keys and channel 33, the phase table. Each entry runs once
 * on each side. Where the faithful routine runs, the readable one must too,
 * with the same memory, registers, channels and traps, word for word;
 * where the readable one refuses, it must leave everything as it found it.
 *
 * Two kinds of state are not counted:
 *   - MR.KLEAN, P00KLEAN or V37KLEAN returning by TC Q with an overflow in
 *     Q, a state no program makes: the faithful routine goes to the
 *     overflow-corrected address, where the AGC (yaAGC) runs the 16-bit Q
 *     as some other instruction; the readable binding refuses;
 *   - the same with Q at one of the file's own return points (as when
 *     DOFSTRT1 calls MR.KLEAN): the faithful routine carries on in the
 *     file, the readable binding refuses, because there a readable routine
 *     returns from the C call instead.
 * The faithful routines are certified against the original program
 * (c64-port/tests/test_<m>_fresh_start.c), as the readable ones are by make
 * fresh-start-cases' first step. */
#include <stdio.h>
#include <string.h>
#if LUMINARY099
#include "lm_fresh_start_cases.h"
#define MISSION       "LM"
#define TABLE         lm_fresh_start_routines
#define ENTRIES       lm_fresh_entries
#define FAITHFUL      faithful_lm_fresh_start_routines
#define FAITHFUL_KEYS faithful_lm_fresh_entries
#define ROUTINES      LM_FS_ROUTINES
#define KLEAN         1 /* MR.KLEAN's place in ENTRIES, then P00KLEAN, V37KLEAN */
typedef lm_fresh_io fresh_io;
#else
#include "cm_fresh_start_cases.h"
#define MISSION       "CM"
#define TABLE         cm_fresh_start_routines
#define ENTRIES       cm_fresh_entries
#define FAITHFUL      faithful_cm_fresh_start_routines
#define FAITHFUL_KEYS faithful_cm_fresh_entries
#define ROUTINES      CM_FS_ROUTINES
#define KLEAN         3
typedef cm_fresh_io fresh_io;
#endif

extern const agc_routine_table FAITHFUL;
extern const uint16_t FAITHFUL_KEYS[ROUTINES];

#define CASES 65536u

static uint32_t seed = 314159u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* One case: the port's test state n, and for odd n more. */
static void state(apollo_overlay_state *s, fresh_io *io, agc_hardware *hw, uint16_t n)
{
    unsigned g;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
#if LUMINARY099
    lm_fs_input(s, io, hw, n);
#else
    cm_fs_input(s, io, n);
    hw->channel11 = rnd();
    io->warning = rnd() % 2u;
#endif
    if (n % 2u == 0u) return;
    if (rnd() % 4u == 0u) s->words[2] = (uint16_t)(rnd() | (rnd() << 1)); /* any Q */
    if (rnd() % 16u == 0u) io->control.interrupt_enabled = (uint16_t)(rnd() % 3u);
    if (rnd() % 16u == 0u) io->control.superbank = (uint16_t)(rnd() & 0377u);
    if (rnd() % 32u == 0u) s->words[7] = 1u;
#if LUMINARY099
    if (rnd() % 4u == 0u) io->channel15 = (agc_word)(rnd() & 077777u);
    if (rnd() % 4u == 0u) io->channel16 = (agc_word)(rnd() & 077777u);
#else
    if (rnd() % 4u == 0u) io->control.main_keys = (agc_word)(rnd() & 077777u);
    if (rnd() % 4u == 0u) io->control.navigation_keys = (agc_word)(rnd() & 077777u);
#endif
    if (rnd() % 4u == 0u) io->channel33 = (agc_word)(rnd() & 077777u);
    for (g = 0; g < 12u; ++g) /* -PHASE1 .. PHASE6 */
        if (rnd() % 16u == 0u) s->words[0752u + g] = (uint16_t)(rnd() & 077777u);
}

static bool klean(uint16_t z)
{
    return z == ENTRIES[KLEAN] || z == ENTRIES[KLEAN + 1] || z == ENTRIES[KLEAN + 2];
}

static bool not_counted(uint16_t z, const apollo_overlay_state *s)
{
    uint16_t q = s->words[2];
    bool overflow = ((q >> 15) & 1u) != ((q >> 14) & 1u);
    return klean(z) && (overflow || q < 04000u);
}

static bool same_io(const fresh_io *a, const fresh_io *b, const agc_hardware *ha,
                    const agc_hardware *hb)
{
    return memcmp(a, b, sizeof *a) == 0 && ha->channel11 == hb->channel11;
}

int main(void)
{
    unsigned i, n, checks = 0, failures = 0, skipped = 0, ran[64] = {0};

    if (TABLE.count != FAITHFUL.count || TABLE.count != ROUTINES) {
        printf("FAIL: %u entries, faithful %u\n", TABLE.count, FAITHFUL.count);
        return 1;
    }
    for (i = 0; i < ROUTINES; ++i) {
        if (TABLE.entries[i].physical != FAITHFUL.entries[i].physical) {
            printf("FAIL: entry %u at key %u, faithful %u\n", i, TABLE.entries[i].physical,
                   FAITHFUL.entries[i].physical);
            ++failures;
        }
        if (ENTRIES[i] != FAITHFUL_KEYS[i]) {
            printf("FAIL: entry address %u is %05o, faithful %05o\n", i, ENTRIES[i],
                   FAITHFUL_KEYS[i]);
            ++failures;
        }
    }
    for (n = 0; n < CASES; ++n) {
        apollo_overlay_state start, fa, ra;
        fresh_io io_start, fio, rio;
        agc_hardware hw_start, fhw, rhw;
        agc_native_program fp, rp;
        enum exec_status sf, sr;
        const char *why = 0;
        uint16_t z;

        state(&start, &io_start, &hw_start, (uint16_t)n);
        z = start.words[5];
        for (i = 0; i < ROUTINES; ++i) {
            uint16_t key = TABLE.entries[i].physical;
            if (key == (z >= 04000u ? 2048u + z - 04000u : 5u * 1024u + z - 02000u)) break;
        }
        if (i == ROUTINES) {
            printf("FAIL case %u: entry %05o not in the table\n", n, z);
            ++failures;
            continue;
        }
        fa = ra = start;
        fio = rio = io_start;
        fhw = rhw = hw_start;
        memset(&fp, 0, sizeof fp);
        memset(&rp, 0, sizeof rp);
        fp.hardware = &fhw;
        rp.hardware = &rhw;
        if (n % 64u == 63u) fp.hardware = rp.hardware = NULL; /* no channel 11 */
        sf = FAITHFUL.entries[i].call(&fp, &fa, &fio);
        sr = TABLE.entries[i].call(&rp, &ra, &rio);
        ++checks;
        if (sf == EXEC_OK) ++ran[i];
        if (sr != EXEC_OK &&
            (memcmp(&ra, &start, sizeof ra) || !same_io(&rio, &io_start, &rhw, &hw_start)))
            why = "readable refusal changed the state";
        else if (sf == EXEC_OK && sr != EXEC_OK && not_counted(z, &start))
            ++skipped;
        else if (sf == EXEC_OK && sr != EXEC_OK)
            why = "readable refused, faithful ran";
        else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
            why = "different memory";
        else if (sf == EXEC_OK && !same_io(&fio, &rio, &fhw, &rhw))
            why = "different channels";
        if (why && ++failures <= 30) {
            unsigned w;
            printf("FAIL case %u, entry %05o, Q %06o: %s\n", n, z, start.words[2], why);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
        }
    }
    for (i = 0; i < ROUTINES; ++i) {
        unsigned key = TABLE.entries[i].physical;
        printf("%s %05o: run by the faithful routine %u times\n", MISSION,
               key < 4096u ? 04000u + key - 2048u : 02000u + key % 1024u, ran[i]);
    }
    printf("fresh-start-cases %s: %u checks (%u not counted: KLEAN with Q overflowed or in the "
           "file), %u failures\n",
           MISSION, checks, skipped, failures);
    return failures != 0;
}
