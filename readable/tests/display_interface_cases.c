/* SPDX-License-Identifier: MIT */
/* The readable display interface routines against the faithful port's six
 * modules (c64-port/src/lanec_di_<part>.c), side by side (make
 * display-interface-cases).
 *
 * The faithful modules are built with their dispatches renamed
 * faithful_lanec_di_<part>_dispatch; the readable ones of both missions
 * are lanec_di_<part>_dispatch (display_interface_missions.c). First every
 * case of the port's six suites and of its refusal suite
 * (c64-port/tests/lanec_di_<part>_cases.c, lanec_di_refuse_cases.c). Then
 * each key of the port's routine tables (lanec_di_routines.inc, both
 * missions) from 4,096 states of the port's random fill, entered there and
 * stirred: A +0, -0, a display kind, an overflow or anything; Q a return
 * into fixed memory, another key, a word of erasable memory or anything;
 * L one of the returns SAVELOCR saves or any sign-extended word; COPINDEX
 * a kind, just past one or up to 0300; FACEREG, TEMPOR2 and COPMPAC a
 * kind, -0, just past one or anything; LOCCTR a job's register set, -1,
 * the last that fits, one past it or anything; MPAC +5 a register set
 * offset or anything; DSPLOCK either sign; FLAGWRD4 as the
 * fill leaves it or with no display bits; now and then the bank registers
 * or ZERO wrong, the superbank bit off, another bank, Z one word off, or
 * the other mission. Where either runs, the other must too, with the same
 * memory and channels word for word; where the readable one refuses, it
 * must leave everything as it found it; and the faithful module must run
 * from some state of every key. argv: the LM and CM ropes. */
#include <stdio.h>
#include <string.h>
#include "lanec_suites.h"
#include "lanec_di_dsp.h"
#include "lanec_di_idle.h"
#include "lanec_di_mark.h"
#include "lanec_di_nvdsp.h"
#include "lanec_di_play.h"
#include "lanec_di_routines.h"
#include "lanec_di_sleep.h"
#include "lanec_symbols.h"
#include "rope_image.h"

typedef enum lanec_status (*dispatch)(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#define FAITHFUL(part)                                                                             \
    enum lanec_status faithful_lanec_di_##part##_dispatch(apollo_overlay_state *, lanec_io *,      \
                                                          enum lanec_mission);
FAITHFUL(dsp)
FAITHFUL(mark)
FAITHFUL(play)
FAITHFUL(sleep)
FAITHFUL(idle)
FAITHFUL(nvdsp)

/* The routine tables' keys: each part's stub marks its keys. */
#define STUB(part, m)                                                                              \
    static enum exec_status part##_##m(const struct agc_native_program *p,                         \
                                       apollo_overlay_state *s, void *v)                           \
    {                                                                                              \
        (void)p;                                                                                   \
        (void)s;                                                                                   \
        (void)v;                                                                                   \
        return EXEC_OUT_OF_SCOPE;                                                                  \
    }
#define STUBS(part) STUB(part, lm) STUB(part, cm)
STUBS(dsp)
STUBS(mark)
STUBS(play)
STUBS(sleep)
STUBS(idle)
STUBS(nvdsp)
#include "lanec_di_routines.inc"

static const struct {
    agc_routine lm, cm;
    dispatch faithful, readable;
    const char *name;
} parts[] = {
    {dsp_lm, dsp_cm, faithful_lanec_di_dsp_dispatch, lanec_di_dsp_dispatch, "dsp"},
    {mark_lm, mark_cm, faithful_lanec_di_mark_dispatch, lanec_di_mark_dispatch, "mark"},
    {play_lm, play_cm, faithful_lanec_di_play_dispatch, lanec_di_play_dispatch, "play"},
    {sleep_lm, sleep_cm, faithful_lanec_di_sleep_dispatch, lanec_di_sleep_dispatch, "sleep"},
    {idle_lm, idle_cm, faithful_lanec_di_idle_dispatch, lanec_di_idle_dispatch, "idle"},
    {nvdsp_lm, nvdsp_cm, faithful_lanec_di_nvdsp_dispatch, lanec_di_nvdsp_dispatch, "nvdsp"},
};
#define PARTS (sizeof parts / sizeof parts[0])

/* The part a suite runs, by its dispatch (the refusal suite: by the
 * module its case was made from, found by trying each). */
static size_t part_of(lanec_run run)
{
    size_t k;

    for (k = 0; k < PARTS; ++k)
        if (run == parts[k].readable) return k;
    return PARTS;
}

#define A_AT     0u
#define L_AT     1u
#define Q_AT     2u
#define EB_AT    3u
#define FB_AT    4u
#define Z_AT     5u
#define BB_AT    6u
#define LOCCTR   064u
#define FLAGWRD4 0100u
#define MPAC     0154u
#define LOC      0164u
#define DSPLOCK  01012u
#define STATES   4096u

static uint32_t seed = 271828u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* A 16-bit register: a sign-extended word, or now and then an overflow. */
static uint16_t reg16(void)
{
    uint16_t w = (uint16_t)(rnd() & 077777u);
    if (rnd() % 4u == 0u) return (uint16_t)(rnd() | (rnd() << 1));
    return (uint16_t)(w | ((w << 1) & 0100000u));
}

/* A sign-extended word, as L is after every instruction. */
static uint16_t sign_extended(void)
{
    uint16_t w = (uint16_t)(rnd() & 077777u);
    return (uint16_t)(w | ((w << 1) & 0100000u));
}

/* A display kind (0-2), -0, one past, or anything. */
static uint16_t kind(void)
{
    switch (rnd() % 8u) {
    case 0:
    case 1: return 0u;
    case 2:
    case 3: return 1u;
    case 4: return 2u;
    case 5: return 077777u;
    case 6: return 3u;
    default: return (uint16_t)(rnd() & 077777u);
    }
}

/* Z and the bank of a key's physical address. */
static uint16_t key_z(uint16_t physical, uint16_t *bank)
{
    *bank = (uint16_t)(physical / 1024u);
    if (*bank == 2u || *bank == 3u) return (uint16_t)(physical - 2048u + 04000u);
    return (uint16_t)(02000u + physical % 1024u);
}

static const agc_routine_entry *keys(enum lanec_mission m, size_t *count)
{
    *count = m == LANEC_LM ? LANEC_DI_LM_ROUTINES : LANEC_DI_CM_ROUTINES;
    return m == LANEC_LM ? lm_entries : cm_entries;
}

/* One state at key z (in bank) of mission m, from the port's fill for case n. */
static enum lanec_mission state(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m,
                                uint16_t bank, uint16_t z, unsigned n)
{
    uint16_t *w = s->words, x, b;
    size_t count;
    const agc_routine_entry *list = keys(m, &count);
    uint16_t goflash2 = m == LANEC_LM ? LANEC_LM_GOFLASH2 : LANEC_CM_GOFLASH2;
    uint16_t bothjobs = m == LANEC_LM ? LANEC_LM_BOTHJOBS : LANEC_CM_BOTHJOBS;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    lanec_fill(s, io, n * 2654435761u + z);
    if (bank == 2u || bank == 3u) bank = rnd() % 4u ? 010u : (uint16_t)(rnd() % 030u);
    lanec_enter(s, io, bank, z);
    x = (uint16_t)(rnd() % 8u);
    w[A_AT] = x == 0u   ? 0u
              : x == 1u ? 0177777u
              : x < 4u  ? (uint16_t)(rnd() % 3u)
              : x == 4u ? (uint16_t)((rnd() & 1u ? 040000u : 0100000u) | (rnd() & 037777u))
                        : reg16();
    x = (uint16_t)(rnd() % 16u);
    w[Q_AT] = x < 8u    ? (uint16_t)(02000u + rnd() % 06000u)
              : x < 11u ? key_z(list[rnd() % count].physical, &b)
              : x < 13u ? (uint16_t)(rnd() % 02000u)
                        : reg16();
    x = (uint16_t)(rnd() % 8u);
    w[L_AT] = x == 0u   ? (uint16_t)(goflash2 + 2u)
              : x == 1u ? (uint16_t)(bothjobs + 1u)
              : x == 2u ? (uint16_t)(bothjobs + 014u)
                        : sign_extended();
    /* COPINDEX at most 0300: the faithful module's IFLEGAL +4 indexes with
     * it unchecked (IFLEGAL has just set it to 2). */
    x = kind();
    w[LOC] = x <= 3u ? x : (uint16_t)(rnd() % 0301u);
    if (rnd() % 2u) w[MPAC] = kind();
    if (rnd() % 2u) w[MPAC + 3u] = kind();
    if (rnd() % 2u) w[MPAC + 4u] = kind();
    x = (uint16_t)(rnd() % 8u);
    w[LOCCTR] = x < 3u    ? (uint16_t)(014u * (rnd() % 7u))
                : x == 3u ? 077776u
                : x == 4u ? (uint16_t)(01376u - LOC + rnd() % 2u)
                : x == 5u ? (uint16_t)(01376u - (MPAC + 4u) + rnd() % 2u)
                          : (uint16_t)(rnd() & 077777u);
    if (rnd() % 2u) w[MPAC + 5u] = (uint16_t)(014u * (rnd() % 7u));
    w[DSPLOCK] = rnd() % 2u ? (uint16_t)(rnd() & 037777u) : (uint16_t)(040000u | rnd());
    w[DSPLOCK] &= 077777u;
    if (rnd() % 4u == 0u) w[FLAGWRD4] = 0u;
    /* Now and then a state the module does not take. */
    switch (rnd() % 64u) {
    case 0: w[FB_AT] = (uint16_t)(rnd() & 076000u); break;
    case 1: io->superbank &= (uint16_t)~0100u; break;
    case 2: w[BB_AT] ^= 02000u; break;
    case 3: w[EB_AT] |= 1u; break;
    case 4: w[7] = 1u; break;
    case 5: m = m == LANEC_LM ? LANEC_CM : LANEC_LM; break;
    case 6: w[Z_AT] ^= 1u; break;
    default: break;
    }
    return m;
}

/* Run both on one state; a description of the difference, or NULL. */
static const char *compare(size_t k, const apollo_overlay_state *start, const lanec_io *io,
                           enum lanec_mission m, int *ran, apollo_overlay_state *fa,
                           apollo_overlay_state *ra)
{
    lanec_io fio = *io, rio = *io;
    enum lanec_status sf, sr;

    *fa = *ra = *start;
    sf = parts[k].faithful(fa, &fio, m);
    sr = parts[k].readable(ra, &rio, m);
    *ran = sf == LANEC_OK;
    if (sr != LANEC_OK && (memcmp(ra, start, sizeof *ra) || memcmp(&rio, io, sizeof rio)))
        return "readable refusal changed the state";
    if (sf == LANEC_OK && sr != LANEC_OK) return "readable refused, faithful ran";
    if (sf != LANEC_OK && sr == LANEC_OK) return "readable ran, faithful refused";
    if (sf == LANEC_OK && memcmp(fa, ra, sizeof *fa)) return "different memory";
    if (sf == LANEC_OK && memcmp(&fio, &rio, sizeof fio)) return "different channels";
    return NULL;
}

static void show(const apollo_overlay_state *start, const apollo_overlay_state *fa,
                 const apollo_overlay_state *ra)
{
    unsigned w;
    for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
        if (fa->words[w] != ra->words[w])
            printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra->words[w], fa->words[w],
                   start->words[w]);
}

/* A suite's cases, each on its part (every part, for the refusal suite). */
static unsigned suite(const lanec_suite *cases, unsigned *ran_total)
{
    unsigned n, failures = 0;

    for (n = 0; n < cases->count; ++n) {
        apollo_overlay_state start, fa, ra;
        lanec_io io;
        lanec_meta meta;
        size_t k = part_of(cases->run), first = k, last = k;

        memset(&start, 0, sizeof start);
        memset(&io, 0, sizeof io);
        memset(&meta, 0, sizeof meta);
        cases->make(n, &start, &io, &meta);
        if (k == PARTS) first = 0, last = PARTS - 1u;
        for (k = first; k <= last; ++k) {
            int ran;
            const char *why = compare(k, &start, &io, meta.mission, &ran, &fa, &ra);

            *ran_total += (unsigned)ran;
            if (why && ++failures <= 3) {
                printf("FAIL %s case %u (%s %s, Z %04o): %s\n", cases->name, n, parts[k].name,
                       meta.mission == LANEC_LM ? "LM" : "CM", start.words[Z_AT], why);
                show(&start, &fa, &ra);
            }
        }
    }
    return failures;
}

int main(int argc, char **argv)
{
    static const lanec_suite *const suites[] = {
        &lanec_suite_di_dsp,  &lanec_suite_di_mark,  &lanec_suite_di_play,   &lanec_suite_di_sleep,
        &lanec_suite_di_idle, &lanec_suite_di_nvdsp, &lanec_suite_di_refuse,
    };
    static const enum lanec_mission missions[] = {LANEC_LM, LANEC_CM};
    unsigned i, k, n, cases = 0, suite_ran = 0, keys_run = 0, failures = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < sizeof suites / sizeof suites[0]; ++i) {
        failures += suite(suites[i], &suite_ran);
        cases += suites[i]->count;
    }
    printf("di-cases: the port's suites, %u cases (%u runs), %u failures\n", cases, suite_ran,
           failures);

    for (k = 0; k < 2u; ++k) {
        size_t count, part;
        const agc_routine_entry *list = keys(missions[k], &count);

        for (i = 0; i < count; ++i, ++keys_run) {
            unsigned ran_total = 0, bad = 0;
            uint16_t bank, z = key_z(list[i].physical, &bank);

            for (part = 0; part < PARTS; ++part)
                if (list[i].call == (k ? parts[part].cm : parts[part].lm)) break;
            for (n = 0; n < STATES; ++n) {
                apollo_overlay_state start, fa, ra;
                lanec_io io;
                enum lanec_mission m = state(&start, &io, missions[k], bank, z, n);
                int ran;
                const char *why = compare(part, &start, &io, m, &ran, &fa, &ra);

                ran_total += (unsigned)ran;
                if (why && ++bad <= 3) {
                    printf("FAIL %s %s %o,%04o state %u: %s\n", k ? "CM" : "LM", parts[part].name,
                           bank, z, n, why);
                    show(&start, &fa, &ra);
                }
            }
            if (ran_total == 0u && bad == 0u) {
                printf("FAIL %s %s %o,%04o: the faithful routine never ran\n", k ? "CM" : "LM",
                       parts[part].name, bank, z);
                ++bad;
            }
            failures += bad;
        }
    }
    printf("di-cases: %u keys from %u states each, %u failures\n", keys_run, STATES, failures);
    return failures != 0;
}
