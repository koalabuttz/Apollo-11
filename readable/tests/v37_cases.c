/* SPDX-License-Identifier: MIT */
/* The readable verb 37 and P00 against the faithful port's, case by case
 * (make v37-cases).
 *
 * Built once per mission, with the readable table (src/binding/
 * v37_routines.c) and the faithful port's under the same names with
 * faithful_ in front (the Makefile renames them). First V37's tables: the
 * readable PREMM1, FCADRMM1 and DNLADMM1 (and the CM's FLAGTABL), written
 * from the listing, must hold the words the port reads out of the
 * original program's rope (c64-port/src/<m>_v37_operands.inc), one per
 * program V37 knows (NOV37MM + 1). Then the routines: each case is one of
 * the port's test states (c64-port/tests/<m>_v37_cases.c: every entry, a
 * major mode PREMM1 lists, P00, P20 or any other, MODREG and MINDEX near
 * and far), half of them stirred further: any Q, A with an overflow,
 * interrupts, SUPERBNK, the ZERO register, a negative MMNUMBER, a MINDEX
 * past the tables, INITSUB's return word, the phase table. Each entry runs
 * once on each side. Where the faithful routine runs, the readable one
 * must too, with the same memory, registers, channels and traps, word for
 * word; where the readable one refuses, it must leave everything as it
 * found it. The faithful routines are certified against the original
 * program (c64-port/tests/test_<m>_v37.c), as the readable ones are by
 * make v37-cases' first step. */
#include <stdio.h>
#include <string.h>
#if LUMINARY099
#include "lm_v37_cases.h"
/* The words V37 reads, out of the original program's rope. */
#include "../src/lm_v37_operands.inc"
#define MISSION         "LM"
#define TABLE           lm_v37_routines
#define FAITHFUL        faithful_lm_v37_routines
#define FAITHFUL_KEYS   faithful_lm_v37_entries
#define ROUTINES        LM_V37_ROUTINES
#define MMNUMBER        LM_V37_MMNUMBER
#define MINDEX          LM_V37_MINDEX
#define NOV37MM         LM_V37_K_NOV37MM
#define LISTED_PREMM1   lm_v37_premm1
#define LISTED_FCADRMM1 lm_v37_fcadrmm1
#define LISTED_DNLADMM1 lm_v37_dnladmm1
typedef lm_fresh_io v37_io;
extern const uint16_t FAITHFUL_KEYS[ROUTINES];
#else
#include "cm_v37_cases.h"
/* The words V37 reads, out of the original program's rope. */
#include "../src/cm_v37_operands.inc"
#define MISSION         "CM"
#define TABLE           cm_v37_routines
#define FAITHFUL        faithful_cm_v37_routines
#define FAITHFUL_KEYS   faithful_cm_v37_entries
#define ROUTINES        CM_V37_ROUTINES
#define MMNUMBER        CM_V37_MMNUMBER
#define MINDEX          CM_V37_MINDEX
#define NOV37MM         CM_V37_K_NOV37MM
#define LISTED_PREMM1   cm_v37_premm1
#define LISTED_FCADRMM1 cm_v37_fcadrmm1
#define LISTED_DNLADMM1 cm_v37_dnladmm1
typedef cm_fresh_io v37_io;
extern const cm_v37_entry FAITHFUL_KEYS[ROUTINES];
extern const uint16_t FLAGTABL[12];
#endif

extern const agc_routine_table FAITHFUL;
/* The readable tables (src/fresh_start/v37.c). */
extern const uint16_t PREMM1[], FCADRMM1[], DNLADMM1[];
extern const unsigned V37_PROGRAMS;

#define CASES        65536u
#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static uint32_t seed = 271828u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* One case: the port's test state n, and for odd n more. */
static void state(apollo_overlay_state *s, v37_io *io, agc_hardware *hw, uint16_t n)
{
    unsigned g;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
#if LUMINARY099
    lm_v37_input(s, io, hw, n);
#else
    cm_v37_input(s, io, n);
#endif
    if (n % 2u == 0u) return;
    if (rnd() % 4u == 0u) s->words[2] = (uint16_t)(rnd() | (rnd() << 1)); /* any Q */
    if (rnd() % 8u == 0u)
        s->words[0] ^= (uint16_t)(rnd() % 2u ? 0100000u : 040000u); /* A overflows */
    if (rnd() % 16u == 0u) io->control.interrupt_enabled = (uint16_t)(rnd() % 3u);
    if (rnd() % 16u == 0u) io->control.superbank = (uint16_t)(rnd() & 0377u);
    if (rnd() % 32u == 0u) s->words[7] = 1u;
    if (rnd() % 16u == 0u) s->words[MMNUMBER] = (uint16_t)(rnd() & 077777u);
    if (rnd() % 16u == 0u)
        s->words[MINDEX] = (uint16_t)(rnd() % 2u ? rnd() & 077777u : 030u + rnd() % 020u);
    if (rnd() % 8u == 0u) s->words[0155] = (uint16_t)(rnd() & 077777u); /* MPAC +1 */
    for (g = 0; g < 12u; ++g)                                           /* -PHASE1 .. PHASE6 */
        if (rnd() % 16u == 0u) s->words[0752u + g] = (uint16_t)(rnd() & 077777u);
}

static unsigned key_of(unsigned i)
{
#if LUMINARY099
    return 4u * 1024u + FAITHFUL_KEYS[i] - 02000u;
#else
    return FAITHFUL_KEYS[i].bank * 1024u + FAITHFUL_KEYS[i].z - 02000u;
#endif
}

static unsigned tables(void)
{
    unsigned i, failures = 0;

    if (V37_PROGRAMS != NOV37MM + 1u || V37_PROGRAMS != COUNT(LISTED_PREMM1)) {
        printf("FAIL: V37 knows %u programs, NOV37MM + 1 = %u, the rope %u\n", V37_PROGRAMS,
               (unsigned)NOV37MM + 1u, (unsigned)COUNT(LISTED_PREMM1));
        return 1;
    }
    for (i = 0; i < V37_PROGRAMS; ++i) {
        if (PREMM1[i] != LISTED_PREMM1[i]) printf("FAIL: PREMM1 +%u\n", i), ++failures;
        if (FCADRMM1[i] != LISTED_FCADRMM1[i]) printf("FAIL: FCADRMM1 +%u\n", i), ++failures;
        if (DNLADMM1[i] != LISTED_DNLADMM1[i]) printf("FAIL: DNLADMM1 +%u\n", i), ++failures;
    }
#if COMANCHE055
    for (i = 0; i < 12u; ++i)
        if (FLAGTABL[i] != cm_v37_flagtabl[i]) printf("FAIL: FLAGTABL +%u\n", i), ++failures;
#endif
    return failures;
}

int main(void)
{
    unsigned i, n, checks = 0, failures = tables(), ran[64] = {0};

    if (TABLE.count != FAITHFUL.count || TABLE.count != ROUTINES) {
        printf("FAIL: %u entries, faithful %u\n", TABLE.count, FAITHFUL.count);
        return 1;
    }
    for (i = 0; i < ROUTINES; ++i)
        if (TABLE.entries[i].physical != FAITHFUL.entries[i].physical ||
            TABLE.entries[i].physical != key_of(i)) {
            printf("FAIL: entry %u at key %u, faithful %u\n", i, TABLE.entries[i].physical,
                   FAITHFUL.entries[i].physical);
            ++failures;
        }
    for (n = 0; n < CASES; ++n) {
        apollo_overlay_state start, fa, ra;
        v37_io io_start, fio, rio;
        agc_hardware hw_start, fhw, rhw;
        agc_native_program fp, rp;
        enum exec_status sf, sr;
        const char *why = 0;

        state(&start, &io_start, &hw_start, (uint16_t)n);
        i = n % ROUTINES;
        fa = ra = start;
        fio = rio = io_start;
        fhw = rhw = hw_start;
        memset(&fp, 0, sizeof fp);
        memset(&rp, 0, sizeof rp);
        fp.hardware = &fhw;
        rp.hardware = &rhw;
        sf = FAITHFUL.entries[i].call(&fp, &fa, &fio);
        sr = TABLE.entries[i].call(&rp, &ra, &rio);
        ++checks;
        if (sf == EXEC_OK) ++ran[i];
        if (sr != EXEC_OK &&
            (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io_start, sizeof rio)))
            why = "readable refusal changed the state";
        else if (sf == EXEC_OK && sr != EXEC_OK)
            why = "readable refused, faithful ran";
        else if (sf != EXEC_OK && sr == EXEC_OK)
            why = "readable ran, faithful refused";
        else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
            why = "different memory";
        else if (sf == EXEC_OK && memcmp(&fio, &rio, sizeof fio))
            why = "different channels";
        if (why && ++failures <= 30) {
            unsigned w;
            printf("FAIL case %u, entry %05o: %s\n", n, start.words[5], why);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
        }
    }
    for (i = 0; i < ROUTINES; ++i)
        printf("%s key %05u: run by the faithful routine %u times\n", MISSION,
               TABLE.entries[i].physical, ran[i]);
    printf("v37-cases %s: %u programs' tables, %u checks, %u failures\n", MISSION, V37_PROGRAMS,
           checks, failures);
    return failures != 0;
}
