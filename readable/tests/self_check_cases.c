/* SPDX-License-Identifier: MIT */
/* The readable self-check against the faithful port's, case by case (make
 * self-check-cases), both missions in one program.
 *
 * Built with the readable tables (src/binding/self_check_routines.c, each
 * mission's objects with only its table names left global) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them). Every key of the idle table and the rest runs
 * on CASES states each: the port's own case images
 * (c64-port/tests/self_check_cases.c, self_check_input(), which keeps
 * ERASCHK's pair and ROPECHK's bank in the program's domain), entered at
 * the key, and then, case by case, some of: SFAIL or SKEEP1 any word the
 * file goes on at (or an idle-loop word, a word inside the file, outside
 * it, or erasable); SMODE any option; NEWJOB, SKEEP5, SKEEP6 and SKEEP7
 * any sign; A any 16 bits; a random word anywhere; the interrupt latch or
 * SUPERBNK or FB out of range. Where either port runs, the other must
 * too, with the same memory and io record, word for word; where the
 * readable one refuses, it must leave everything as it found it. The
 * faithful routines are certified against the original programs
 * (c64-port/tests/test_self_check.c), as the readable ones are by make
 * self-check-cases' oracle step. argv: the LM and CM ropes. */
#include <stdio.h>
#include <string.h>
#include "self_check_cases.h"
#include "rope_image.h"

extern const agc_routine_table faithful_lm_self_check_routines,
    faithful_lm_self_check_idle_routines;
extern const agc_routine_table faithful_lm_self_check_idle_hot_routines;
extern const agc_routine_table faithful_cm_self_check_routines,
    faithful_cm_self_check_idle_routines;

#define CASES 16384u

/* The erasable words (both missions). */
#define NEWJOB_AT 0067u
#define SFAIL_AT  01357u
#define SMODE_AT  01362u
#define SKEEP1_AT 01371u
#define SKEEP5_AT 01375u
#define SKEEP6_AT 01376u
#define SKEEP7_AT 01377u

static uint32_t seed = 314159u;
static unsigned rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (unsigned)(seed >> 12) & 0xFFFFu;
}

/* A Comanche055 address of the file in the mission's numbering. */
static unsigned mission;
#define K(z) ((uint16_t)((z) - (mission ? 0u : 2u)))

/* The words a TC through SFAIL or SKEEP1 goes on at (Comanche055
 * numbering), but TCALARM2 +2 for SFAIL (a -0 SMODE would loop there
 * for ever, in both ports). */
static const uint16_t resumes[] = {03270u, 03341u, 03413u, 03430u, 03443u, 03512u, 03520u,
                                   03522u, 03524u, 03542u, 03543u, 03564u, 03565u, 03625u,
                                   03627u, 03635u, 03675u, 03713u, 03724u};

static uint16_t tc_word(int sfail)
{
    unsigned m = rnd() % 8u, at;

    switch (m) {
    case 0: return (uint16_t)(04000u + rnd() % 04000u); /* fixed-fixed */
    case 1: return (uint16_t)(02000u + rnd() % 02000u); /* bank 43, anywhere */
    case 2: return rnd() % 2u ? K(03306u) : K(03340u);  /* the idle loop */
    case 3:
        return (uint16_t)(rnd() % 2u ? rnd() % 02000u : rnd() & 077777u); /* erasable, anything */
    default:
        do at = resumes[rnd() % (sizeof resumes / sizeof resumes[0])];
        while (sfail && at == 03270u);
        return K(at);
    }
}

static uint16_t any_sign(void)
{
    static const uint16_t v[6] = {1u, 0u, 077776u, 077777u, 2u, 077775u};
    return rnd() % 4u ? v[rnd() % 6u] : (uint16_t)(rnd() & 077777u);
}

static void perturb(apollo_overlay_state *s, self_check_io *io)
{
    unsigned m = rnd() % 16u;

    if (m & 1u) s->words[SFAIL_AT] = tc_word(1);
    if (m & 2u) s->words[SKEEP1_AT] = tc_word(0);
    if (m & 4u) s->words[SMODE_AT] = any_sign();
    if (m & 8u) s->words[NEWJOB_AT] = any_sign();
    if (rnd() % 4u == 0u) s->words[SKEEP5_AT] = any_sign();
    if (rnd() % 4u == 0u) s->words[SKEEP6_AT] = any_sign();
    if (rnd() % 8u == 0u) s->words[SKEEP7_AT] = any_sign();
    if (rnd() % 8u == 0u) s->words[0] = (uint16_t)(rnd() | rnd() << 1);
    if (rnd() % 8u == 0u) s->words[rnd() % 2048u] = (uint16_t)(rnd() & 077777u);
    if (rnd() % 64u == 0u) *self_check_interrupts(mission, io) = 2u;
    if (rnd() % 64u == 0u) *self_check_superbank(mission, io) = (uint16_t)(rnd() % 8u * 020u);
    if (rnd() % 64u == 0u) s->words[4] = (uint16_t)(rnd() % 040u << 10);
}

/* The port's case for entry z: a case of the entry it lists, or of
 * SMODECHK +3 (3306) for the three words the CCS SMODE goes on at. */
static void state(apollo_overlay_state *s, self_check_io *io, uint16_t z, unsigned n)
{
    const uint16_t *entries = self_check_entry_list(mission);
    uint16_t at = z == K(03307u) || z == K(03311u) || z == K(03312u) ? K(03306u) : z;
    unsigned k = 0;

    while (k < SELF_CHECK_ROUTINES && entries[k] != at) ++k;
    if (k == SELF_CHECK_ROUTINES) k = 0;
    self_check_input(mission, s, io, (uint16_t)(k + SELF_CHECK_ROUTINES * (n % 4096u)));
    s->words[5] = z;
    if (n % 2u) perturb(s, io);
}

static unsigned compare(const agc_routine_table *readable, const agc_routine_table *faithful,
                        const char *name)
{
    unsigned i, n, failures = 0;

    if (readable->count != faithful->count) {
        printf("FAIL %s: %u keys, faithful %u\n", name, readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        agc_routine f = faithful->entries[i].call, r = readable->entries[i].call;
        unsigned ran = 0, bad = 0;

        if (readable->entries[i].physical != key) {
            printf("FAIL %s key %u at %u, faithful %u\n", name, i, readable->entries[i].physical,
                   key);
            ++failures;
            continue;
        }
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            self_check_io io, fio, rio;
            agc_native_program p;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, z, n);
            fa = ra = start;
            fio = rio = io;
            memset(&p, 0, sizeof p);
            sf = f(&p, &fa, &fio);
            sr = r(&p, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
            else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if (sf == EXEC_OK && memcmp(&fio, &rio, sizeof fio))
                why = "different io record";
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
        printf("%s %05o: %u cases, %u run by the faithful routine, %u failures\n", name, z, CASES,
               ran, bad);
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned failures = 0;

    if (argc != 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        fprintf(stderr, "usage: sc-cases LM-rope CM-rope\n");
        return 2;
    }
    mission = 0;
    failures += compare(&lm_self_check_idle_routines, &faithful_lm_self_check_idle_routines,
                        "LM self-check idle");
    failures += compare(&lm_self_check_idle_hot_routines, &faithful_lm_self_check_idle_hot_routines,
                        "LM self-check idle (T4RUPT)");
    failures += compare(&lm_self_check_routines, &faithful_lm_self_check_routines, "LM self-check");
    mission = 1;
    failures += compare(&cm_self_check_idle_routines, &faithful_cm_self_check_idle_routines,
                        "CM self-check idle");
    failures += compare(&cm_self_check_routines, &faithful_cm_self_check_routines, "CM self-check");
    printf("self-check-cases: %u failures\n", failures);
    return failures != 0;
}
