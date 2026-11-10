/* SPDX-License-Identifier: MIT */
/* The readable update program against the faithful port's, case by case
 * (make update-program-cases ROPE_LM ROPE_CM).
 *
 * Built once per mission, with the readable tables (src/binding/
 * update_program_routines.c) and the faithful port's under the same names
 * with faithful_ in front (the Makefile renames them). First the entry
 * lists and the keys, which must be the faithful port's. Then every key
 * of both tables, basic and interpretive, on CASES states: the port's own
 * entry states (c64-port/tests/<m>_update_cases.c, <m>_up_input(), shaped
 * to each entry's domain), and in half the cases one or more of: A, L or
 * Q any word; UPVERB any of 0-4, -0, -1 or any word; UPTEMP one of TIMEDIDL's
 * returns, a word next to one, or any word; COMPNUMB, UPCOUNT, MPAC or a
 * word of UPBUFF any word; an update ECADR in EBANK 0 (the registers and
 * counters, or MPAC itself); MODREG and FLAGWRD5 bit 8; the interrupts,
 * SUPERBNK and the ZERO register out of range; no agc_hardware; a random
 * word anywhere (but the central registers and LOC); and for the
 * interpretive words EDOP, the work area, its push-down pointer, BANKSET
 * and NEWJOB. BBANK follows FB and EBANK, as the AGC keeps it. Where either port runs, the other
 * must too, with the same memory, io record and channel 11, word for word;
 * where the readable one refuses, it must leave everything as it found it.
 * The faithful routines are certified against the original program
 * (c64-port/tests/test_<m>_update.c), as the readable ones are by make
 * update-program-cases' first step. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#if LUMINARY099
#include "lm_update_cases.h"
typedef p66_bias_io mission_io;
#define MISSION          "LM"
#define TABLE            lm_update_routines
#define FAITHFUL         faithful_lm_update_routines
#define INTERPRETIVE     lm_update_interpretive
#define FAITHFUL_INTERP  faithful_lm_update_interpretive
#define ENTRIES          lm_up_entries
#define FAITHFUL_ENTRIES faithful_lm_up_entries
#define IENTRIES         lm_up_interp_entries
#define FAITHFUL_IKEYS   faithful_lm_up_interp_entries
#define ROUTINES         LM_UP_ROUTINES
#define INTERP           LM_UP_INTERPRETIVE
#define INPUT            lm_up_input
#define UPVERB_AT        LM_UP_UPVERB
#define UPTEMP_AT        LM_UP_UPTEMP
#define COMPNUMB_AT      LM_UP_COMPNUMB
#define UPCOUNT_AT       LM_UP_UPCOUNT
#define UPBUFF_AT        LM_UP_UPBUFF
typedef lm_up_entry entry;
/* TIMEDIDL's two error returns (UPEND73 +4, UPEND70 +4). */
static const uint16_t returns[2] = {03542u, 03727u};
#else
#include "cm_update_cases.h"
typedef cm_native_io mission_io;
#define MISSION          "CM"
#define TABLE            cm_update_routines
#define FAITHFUL         faithful_cm_update_routines
#define INTERPRETIVE     cm_update_interpretive
#define FAITHFUL_INTERP  faithful_cm_update_interpretive
#define ENTRIES          cm_up_entries
#define FAITHFUL_ENTRIES faithful_cm_up_entries
#define IENTRIES         cm_up_interp_entries
#define FAITHFUL_IKEYS   faithful_cm_up_interp_entries
#define ROUTINES         CM_UP_ROUTINES
#define INTERP           CM_UP_INTERPRETIVE
#define INPUT            cm_up_input
#define UPVERB_AT        CM_UP_UPVERB
#define UPTEMP_AT        CM_UP_UPTEMP
#define COMPNUMB_AT      CM_UP_COMPNUMB
#define UPCOUNT_AT       CM_UP_UPCOUNT
#define UPBUFF_AT        CM_UP_UPBUFF
typedef cm_up_entry entry;
static const uint16_t returns[2] = {03534u, 03721u};
#endif

extern const agc_routine_table FAITHFUL, FAITHFUL_INTERP;
extern const entry FAITHFUL_ENTRIES[ROUTINES];
extern const uint16_t FAITHFUL_IKEYS[INTERP];

#define CASES 4096u
/* The port's entry states per key: case numbers fit 16 bits. */
#define STATES    (65536u / (ROUTINES + INTERP))
#define MPAC_AT   0154u
#define MODREG_AT 01011u
#define FLAGWRD5  0101u

static uint32_t seed = 314159u;
static unsigned rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (unsigned)(seed >> 12) & 0xFFFFu;
}

static uint16_t any_word(void)
{
    static const uint16_t v[8] = {0u, 077777u, 1u, 077776u, 2u, 3u, 4u, 037777u};
    return rnd() % 2u ? v[rnd() % 8u] : (uint16_t)(rnd() & 077777u);
}

/* A register: any word, sign-extended, or now and then with an overflow. */
static uint16_t any_register(void)
{
    uint16_t w = any_word();
    if (rnd() % 8u == 0u) return (uint16_t)(rnd() & 0177777u);
    return (uint16_t)((w & 040000u) ? (w | 0100000u) : w);
}

/* A word anywhere but the central registers and LOC, which the table's
 * dispatch reads to choose the key. */
static unsigned any_address(void)
{
    unsigned at;

    do at = 010u + rnd() % (2048u - 010u);
    while (at == 0164u);
    return at;
}

static void stir_basic(apollo_overlay_state *s, mission_io *io, int *no_hardware)
{
    unsigned m = rnd();

    if (m & 1u) s->words[0] = any_register();
    if (m & 2u) s->words[1] = any_register();
    if (m & 4u) s->words[2] = rnd() % 2u ? any_register() : (uint16_t)(02000u + rnd() % 02000u);
    if (m & 010u) {
        static const uint16_t verbs[7] = {0u, 1u, 2u, 3u, 4u, 077777u, 077776u};
        s->words[UPVERB_AT] = rnd() % 4u ? verbs[rnd() % 7u] : any_word();
    }
    if (m & 020u) {
        uint16_t r = returns[rnd() % 2u];
        unsigned k = rnd() % 4u;
        s->words[UPTEMP_AT] = k == 0u   ? r
                              : k == 1u ? (uint16_t)(r + 1u)
                              : k == 2u ? (uint16_t)(r - 1u)
                                        : any_word();
    }
    if (m & 040u) s->words[COMPNUMB_AT] = any_word();
    if (m & 0100u) s->words[UPCOUNT_AT] = any_word();
    if (m & 0200u) s->words[MPAC_AT + rnd() % 3u] = any_word();
    if (m & 0400u) s->words[UPBUFF_AT + rnd() % 22u] = any_word();
    if (m & 01000u)
        s->words[UPBUFF_AT + 1u + 2u * (rnd() % 10u)] = (uint16_t)(rnd() % 0400u); /* EB 0 */
    if (m & 02000u) s->words[MODREG_AT] = rnd() % 2u ? (uint16_t)(rnd() % 4u) : any_word();
    if (m & 04000u) s->words[FLAGWRD5] ^= 0200u;
    if (m & 010000u) s->words[any_address()] = (uint16_t)(rnd() & 077777u);
    if (m & 020000u) s->words[3] = (uint16_t)((rnd() % 8u) << 8);
    if ((m & 0140000u) == 0140000u) {
        switch (rnd() % 4u) {
        case 0: io->interrupt_enabled = (uint16_t)(rnd() % 3u); break;
        case 1: io->superbank = (uint16_t)(rnd() & 0377u); break;
        case 2: s->words[7] = 1u; break;
        default: *no_hardware = 1; break;
        }
    }
    s->words[6] = (uint16_t)((s->words[4] & 076000u) | (s->words[3] >> 8)); /* BBANK */
}

static void stir_interpretive(apollo_overlay_state *s, mission_io *io)
{
    switch (rnd() % 8u) {
    case 0: s->words[023] = (uint16_t)(rnd() % 4u); break; /* EDOP */
    case 1:
        s->words[0120] = (uint16_t)(0401u + 054u * (rnd() % 6u) - rnd() % 2u);
        break;                                                                 /* FIXLOC */
    case 2: s->words[0166] = (uint16_t)(s->words[0120] + rnd() % 050u); break; /* PUSHLOC */
    case 3: s->words[0165] = (uint16_t)(rnd() & 077777u); break;               /* BANKSET */
    case 4:
        s->words[067] = (uint16_t)(rnd() % 2u ? rnd() & 03777u : rnd() & 077777u);
        break; /* NEWJOB */
    case 5: io->interrupt_enabled = (uint16_t)(rnd() % 3u); break;
    default: s->words[any_address()] = (uint16_t)(rnd() & 077777u); break;
    }
    s->words[6] = (uint16_t)((s->words[4] & 076000u) | (s->words[3] >> 8)); /* BBANK */
}

static unsigned compare(const agc_routine_table *readable, const agc_routine_table *faithful,
                        unsigned first, int interpretive)
{
    unsigned i, n, failures = 0;

    if (readable->count != faithful->count) {
        printf("FAIL %s: %u keys, faithful %u\n", MISSION, readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical;
        agc_routine f = faithful->entries[i].call, r = readable->entries[i].call;
        unsigned ran = 0, bad = 0;

        if (readable->entries[i].physical != key) {
            printf("FAIL %s key %u at %u, faithful %u\n", MISSION, i, readable->entries[i].physical,
                   key);
            ++failures;
            continue;
        }
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            mission_io io, fio, rio;
            agc_hardware hw, fhw, rhw;
            agc_native_program fp, rp;
            enum exec_status sf, sr;
            int no_hardware = 0;
            const char *why = 0;

            memset(&start, 0, sizeof start);
            INPUT(&start, &io, &hw, (uint16_t)(first + i + (ROUTINES + INTERP) * (n % STATES)));
            if (n % 2u) {
                if (interpretive)
                    stir_interpretive(&start, &io);
                else
                    stir_basic(&start, &io, &no_hardware);
            }
            fa = ra = start;
            fio = rio = io;
            fhw = rhw = hw;
            memset(&fp, 0, sizeof fp);
            memset(&rp, 0, sizeof rp);
            fp.hardware = no_hardware ? NULL : &fhw;
            rp.hardware = no_hardware ? NULL : &rhw;
            sf = f(&fp, &fa, &fio);
            sr = r(&rp, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof io) ||
                                  rhw.channel11 != hw.channel11))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
            else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if (sf == EXEC_OK &&
                     (memcmp(&fio, &rio, sizeof fio) || fhw.channel11 != rhw.channel11))
                why = "different io record";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL %s %05o case %u: %s\n", MISSION, key, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran == 0u && bad == 0u) {
            printf("FAIL %s %05o: the faithful routine never ran\n", MISSION, key);
            ++bad;
        }
        failures += bad;
        printf("%s %s %05o: %u cases, %u run by the faithful routine, %u failures\n", MISSION,
               interpretive ? "interpretive" : "key", key, CASES, ran, bad);
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned i, failures = 0;

    if (argc != 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        fprintf(stderr, "usage: %s LM-rope CM-rope\n", argv[0]);
        return 2;
    }
    for (i = 0; i < ROUTINES; ++i)
        if (ENTRIES[i].bank != FAITHFUL_ENTRIES[i].bank || ENTRIES[i].z != FAITHFUL_ENTRIES[i].z) {
            printf("FAIL %s entry %u: %02o,%04o, faithful %02o,%04o\n", MISSION, i, ENTRIES[i].bank,
                   ENTRIES[i].z, FAITHFUL_ENTRIES[i].bank, FAITHFUL_ENTRIES[i].z);
            ++failures;
        }
    for (i = 0; i < INTERP; ++i)
        if (IENTRIES[i] != FAITHFUL_IKEYS[i]) {
            printf("FAIL %s interpretive entry %u: %04o, faithful %04o\n", MISSION, i, IENTRIES[i],
                   FAITHFUL_IKEYS[i]);
            ++failures;
        }
    failures += compare(&TABLE, &FAITHFUL, 0u, 0);
    failures += compare(&INTERPRETIVE, &FAITHFUL_INTERP, ROUTINES, 1);
    printf("update-program-cases %s: %u failures\n", MISSION, failures);
    return failures != 0;
}
