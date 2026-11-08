/* SPDX-License-Identifier: MIT */
/* The readable TVCINITIALIZE against the faithful port's, key by key (make
 * cm-tvc-init-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable table (src/binding/cm_tvc_init_routines.c) and
 * the faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the keys, which must be the faithful table's, and every key must
 * run readable code. Then the runs: each key from 4,096 states, each one
 * of the port's frozen test states (c64-port/tests/cm_tvc_init_vectors.h)
 * entered at the key, most stirred further: the counters the file indexes
 * by (CNTR, often +0 or 1 or -0; TTMP1; COEFFADR, often one of the
 * coefficient tables' addresses), Q (often one of the file's keys LOADCOEF
 * and S40.15 run on at, or 3), A and L, EBANK (often E6, the file's;
 * now and then any of the eight), the bank and superbank latch at RTRNLOC,
 * DAPDATR1, channel 11 (DSALMOUT's engine bit), the masses, IXX and the
 * gains (often at an edge where DDOUBL or AD overflows), RTRNLOC (often
 * one of SWICHOVR's returns), any erasable bank's words, the latches, or
 * a state the routines refuse (BBANK not the two banks, a word of 16
 * bits). Each runs once on each side, as the program's trampoline would
 * call the key's routine, with a hardware record or (now and then) none.
 * Where either port runs, the other must too, with the same memory and io
 * record, word for word; where the readable one refuses, it must leave
 * everything as it found it; and the faithful routine must run in at
 * least 1 of 16 of each key's states. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_native_image.h"
#include "agc_transfer.h"
#include "cm_tvc_init_vectors.h"

extern const agc_routine_table cm_tvc_init_basic, cm_tvc_init_interpretive;
extern const agc_routine_table faithful_cm_tvc_init_basic, faithful_cm_tvc_init_interpretive;
bool cm_tvc_init_readable(uint16_t key);

/* The addresses of the words the routines use, in E6 (03000 + the
 * address in the bank) or unswitched. */
#define CNTR_AT     03047u /* 01447 */
#define TTMP1_AT    03211u /* 01611 */
#define RTRNLOC_AT  03213u /* 01613 */
#define COEFFADR_AT 03322u /* 01722 */
#define DAPDATR1_AT 03066u /* 01466 */
#define IXX_AT      03070u /* 01470 */
#define CSMMASS_AT  03074u /* 01474 */
#define EMDOT_AT    0110u
#define MASSTMP_AT  03252u /* 01652 */
#define KTLX_I_AT   03236u /* 01636 */
#define FLAGWRD9_AT 0105u
/* The coefficient tables' addresses: CSMN10, LBN10 (fixed) and HBN10 (E6). */
static const uint16_t coefficients[] = {02500u, 02512u, 01425u};
/* The words LOADCOEF and S40.15 go on at after a TC Q, and their returns
 * from the file. */
static const uint16_t returns[] = {02436u, 02441u, 02445u, 02462u, 02471u,
                                   02216u, 02222u, 02402u, 02417u, 3u};

#define CASES 4096u

static uint32_t seed = 937u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned tables_differ(const agc_routine_table *readable, const agc_routine_table *faithful,
                              const char *what)
{
    unsigned i, failures = 0;

    if (readable->count != faithful->count) {
        printf("FAIL: %s has %u keys, faithful %u\n", what, readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < readable->count; ++i)
        if (readable->entries[i].physical != faithful->entries[i].physical) {
            printf("FAIL: %s key %u is %u, faithful %u\n", what, i, readable->entries[i].physical,
                   faithful->entries[i].physical);
            ++failures;
        }
    return failures;
}

/* Every key runs a readable routine. */
static unsigned keys_unwritten(const agc_routine_table *table)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (!cm_tvc_init_readable(key)) {
            printf("FAIL: key %u has no readable routine\n", key);
            ++failures;
        }
    }
    return failures;
}

static uint16_t any_word(void)
{
    return (uint16_t)(rnd() & 077777u);
}

/* A word of a number: often an edge value. */
static uint16_t value_word(void)
{
    static const uint16_t edges[] = {0u,      077777u, 1u,      077776u, 020000u,
                                     057777u, 037777u, 040000u, 010000u, 067777u};
    return rnd() % 4u ? any_word() : edges[rnd() % (sizeof edges / sizeof edges[0])];
}

/* The frozen cases. */
static unsigned mission_cases[CM_TVC_INIT_VECTORS], mission_count;

/* A return address: one of the file's, a TC's, or any. */
static uint16_t return_word(void)
{
    unsigned k = rnd() % 8u;

    if (k < 3u) return returns[rnd() % (sizeof returns / sizeof returns[0])];
    if (k < 6u) return (uint16_t)(010u + rnd() % 07770u);
    return rnd() % 2u ? any_word() : rnd();
}

/* The physical erasable key of a key at or above the transfer base. */
static bool erasable_key(uint16_t key)
{
    return key >= AGC_TRANSFER_ERASABLE_BASE;
}

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, uint16_t key, unsigned n)
{
    const irc_case *c = &cm_tvc_init_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    if (rnd() % 4u) w[3] = rnd() % 4u ? 03000u : 03400u; /* E6, the file's, or E7 */
    if (erasable_key(key)) {
        w[3] = rnd() % 16u ? 03000u : 03400u;
        w[5] = RTRNLOC_AT - 03000u + 01400u; /* 01613 */
        w[4] = rnd() % 4u ? 036000u : (uint16_t)(rnd() & 076000u);
        /* any bank, now and then with the superbank bit set */
        if (rnd() % 4u == 0u) io->superbank = rnd() % 2u ? 0100u : 0u;
    } else {
        w[4] = (uint16_t)((key / 1024u) << 10);
        w[5] = (uint16_t)(02000u + key % 1024u);
    }
    eb = (uint16_t)(w[3] >> 8);
    if (n % 4u != 0u) {
        /* The counters the file indexes by. */
        k = rnd() % 8u;
        w[CNTR_AT] = k < 3u ? 0u : k < 6u ? 1u : k == 6u ? 077777u : any_word();
        w[TTMP1_AT] = rnd() % 4u ? (uint16_t)(rnd() % 2u) : any_word();
        k = rnd() % 4u;
        w[COEFFADR_AT] = k < 3u ? coefficients[rnd() % 3u] : any_word();
        if (rnd() % 2u) w[RTRNLOC_AT] = rnd() % 4u ? (rnd() % 2u ? 02352u : 04753u) : any_word();
        /* The masses, gains and flags, often at an edge. */
        w[DAPDATR1_AT] = rnd() % 2u ? any_word() : (uint16_t)(rnd() % 2u ? 020000u : 010000u);
        w[IXX_AT] = value_word();
        w[CSMMASS_AT] = value_word();
        w[EMDOT_AT] = value_word();
        w[MASSTMP_AT] = value_word();
        w[KTLX_I_AT] = value_word();
        if (rnd() % 4u == 0u) w[FLAGWRD9_AT] = any_word();
        io->channel11 = rnd() % 2u ? (uint16_t)(rnd() % 2u ? 010000u : 0u) : any_word();
        /* Any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A, L and Q. */
        w[0] = rnd() % 2u ? any_word() : rnd();
        w[1] = rnd() % 2u ? any_word() : rnd();
        if (rnd() % 2u) w[2] = return_word();
        if (rnd() % 8u == 0u) {
            k = 010u + rnd() % 03770u;
            w[k] = any_word();
        }
        if (rnd() % 16u == 0u) io->superbank = (uint16_t)(rnd() & (rnd() % 2u ? 0160u : 0177u));
        if (rnd() % 32u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 3u);
        if (rnd() % 32u == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (rnd() % 64u == 0u) w[3] = any_word();
        /* now and then another EBANK, any of the eight (TC Q with Q 3
         * runs EBANK's word as TC <EBANK>) */
        if (!erasable_key(key) && rnd() % 8u == 0u) w[3] = (uint16_t)((rnd() % 8u) << 8);
        if (w[2] == 3u && rnd() % 4u == 0u) w[3] = rnd() % 2u ? 02000u : 01400u; /* at the edge */
        if (w[3] <= 03400u && !(w[3] & 0377u)) eb = (uint16_t)(w[3] >> 8);
    }
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     unsigned *runs)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical;
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            cm_native_io io, fio, rio;
            agc_hardware hardware, fhw, rhw;
            agc_native_program p, fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, n);
            fa = ra = start;
            fio = rio = io;
            memset(&hardware, 0, sizeof hardware);
            fhw = rhw = hardware;
            memset(&p, 0, sizeof p);
            fp = rp = p;
            if (n % 64u != 63u) { /* now and then none */
                fp.hardware = &fhw;
                rp.hardware = &rhw;
            }
            sf = faithful->entries[i].call(&fp, &fa, &fio);
            sr = readable->entries[i].call(&rp, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
            else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if (sf == EXEC_OK &&
                     (memcmp(&fio, &rio, sizeof fio) || memcmp(&fhw, &rhw, sizeof fhw)))
                why = "different io record";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL key %u case %u: %s\n", key, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran < CASES / 16u && bad == 0u) {
            printf("FAIL key %u: the faithful routine ran in %u of %u states\n", key, ran, CASES);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned i, failures = 0, runs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_TVC_INIT_VECTORS; ++i)
        if (cm_tvc_init_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    failures += tables_differ(&cm_tvc_init_basic, &faithful_cm_tvc_init_basic, "basic");
    failures += tables_differ(&cm_tvc_init_interpretive, &faithful_cm_tvc_init_interpretive,
                              "interpretive");
    failures += keys_unwritten(&faithful_cm_tvc_init_basic);
    if (failures) {
        printf("cm-tvc-init-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&cm_tvc_init_basic, &faithful_cm_tvc_init_basic, &runs);
    printf("cm-tvc-init-cases: %u of %u keys readable (all basic) from %u states each, %u run by "
           "the faithful routines, %u failures\n",
           cm_tvc_init_basic.count, faithful_cm_tvc_init_basic.count, CASES, runs, failures);
    return failures != 0;
}
