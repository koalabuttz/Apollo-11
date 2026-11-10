/* SPDX-License-Identifier: MIT */
/* The readable SPS_BACK-UP_RCS_CONTROL against the faithful port's
 * (make sps-backup-cases), the LM's only.
 *
 * Built with the readable table (src/binding/sps_backup_routines.c) and
 * the faithful port's under the same name with faithful_ in front (the
 * Makefile renames it), and the ropes loaded (tests/with_ropes.c: the
 * readable SPSRCS reads UTIME's words from the rope). First the key, which
 * must be the faithful table's. Then 65,536 runs from random states:
 * random memory, FB bank 21 and mostly EBANK 6 with the bank registers
 * agreeing, A and L any word; AXISCTR -1, +0, -0 or +1; the jet times
 * mostly +0, -0, half a unit either way or small, now and then any word;
 * the inhibition counters, E, EDOT and DKDB +0, -0, small or any word,
 * EDOT now and then about the rate limits; DAPBOOLS's DRIFTBIT either
 * way; a return in Q, mostly from 010 to 07777; and now and then a bank,
 * a ZERO, NEWJOB, a latch or a word the faithful routine refuses. A Q of
 * 16 bits with bit 15 set, which the faithful routine takes as its low
 * twelve, the readable one refuses, and the program never makes. Where
 * either port runs, the other must too, with the same memory and io
 * record, word for word; where the readable one refuses, it must leave
 * everything as it found it. */
#include <stdio.h>
#include <string.h>
#include "lm_sps_backup.h"
#include "rope_image.h"
#define LUMINARY099 1
#include "../src/agc/lm_names.inc"

extern const agc_routine_table faithful_lm_sps_backup_basic;

#define RUNS 65536u

/* An erasable address's word under EBANK eb. */
#define EB(a) ((a) < 01400u ? (a) : eb * 0400u + ((a) - 01400u))

static uint32_t seed = 1969u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned pick(unsigned n)
{
    return (unsigned)((rnd() >> 3) % n);
}

static uint16_t sign_extended(uint16_t v)
{
    return (uint16_t)((v & 077777u) | ((v & 040000u) << 1));
}

/* +0, -0, a small number of either sign, or any word. */
static uint16_t stirred(void)
{
    uint16_t v = (uint16_t)pick(64u);

    switch (pick(5u)) {
    case 0: return 0u;
    case 1: return 077777u;
    case 2: return v;
    case 3: return (uint16_t)(~v & 077777u);
    default: return (uint16_t)(rnd() & 077777u);
    }
}

/* A jet time: +0, -0, half a unit either way, or stirred. */
static uint16_t jet_time(void)
{
    switch (pick(5u)) {
    case 0: return 0u;
    case 1: return 077777u;
    case 2: return 020000u;
    case 3: return 057777u;
    default: return stirred();
    }
}

/* A rate near a limit: RATELIM2 (632) or the one MP RATELIM1 finds (about
 * 01200), either sign. */
static uint16_t rate(void)
{
    uint16_t v = (uint16_t)((pick(2u) ? 0632u : 01200u) + pick(64u) - 32u);

    return pick(2u) ? v : (uint16_t)(~v & 077777u);
}

static void state(apollo_overlay_state *s, p66_bias_io *io)
{
    static const uint16_t axes[] = {077776u, 0u, 077777u, 1u};
    uint16_t *w = s->words, eb = pick(16u) ? 6u : (uint16_t)pick(8u);
    unsigned i;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) w[i] = (uint16_t)(rnd() & 077777u);
    w[0] = sign_extended(rnd());
    w[1] = sign_extended(rnd());
    w[2] = pick(8u) ? (uint16_t)(010u + pick(07770u)) : (uint16_t)(rnd() & 077777u);
    w[3] = (uint16_t)(eb << 8);
    w[4] = pick(64u) ? 042000u : (uint16_t)(pick(040u) << 10);
    w[5] = faithful_lm_sps_backup_basic.entries[0].physical % 1024u + 02000u;
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    if (pick(256u) == 0u) w[3] |= (uint16_t)(1u + pick(0377u)); /* an EB of no bank */
    if (pick(128u) == 0u) w[7] = 1u;
    if (pick(256u) == 0u) w[010u + pick(APOLLO_SHARED_WORDS - 010u)] = 0100000u;
    w[K_NEWJOB] = pick(2u) ? 0u : pick(2u) ? 077777u : (uint16_t)pick(037777u);
    if (pick(128u) == 0u) w[K_NEWJOB] = (uint16_t)(040000u + pick(037776u));
    io->superbank = (uint16_t)(pick(8u) * 020u);
    io->interrupt_enabled = (uint16_t)pick(2u);
    if (pick(128u) == 0u) io->interrupt_enabled = 2u;

    w[EB(K_AXISCTR)] = axes[pick(4u)];
    for (i = 0; i < 3u; ++i) w[EB(K_TJP) + i] = jet_time();
    for (i = 0; i < 3u; ++i) w[EB(K_UJETCTR) - 1u + i] = stirred();
    w[EB(K_EDOT)] = pick(3u) ? stirred() : rate();
    w[EB(K_E)] = stirred();
    w[EB(K_DKDB)] = pick(2u) ? stirred() : (uint16_t)(rnd() & 037777u);
    if (pick(2u))
        w[K_DAPBOOLS] |= KW_DRIFTBIT;
    else
        w[K_DAPBOOLS] &= (uint16_t)~KW_DRIFTBIT;
}

static unsigned same_key(void)
{
    if (lm_sps_backup_basic.count != 1u || faithful_lm_sps_backup_basic.count != 1u ||
        lm_sps_backup_interpretive.count != 0u ||
        lm_sps_backup_basic.entries[0].physical !=
            faithful_lm_sps_backup_basic.entries[0].physical) {
        printf("FAIL: the key is not the faithful table's\n");
        return 1u;
    }
    return 0u;
}

int port_main(void)
{
    unsigned n, ran = 0, failures = same_key();

    if (failures) return 1;
    for (n = 0; n < RUNS; ++n) {
        apollo_overlay_state start, fa, ra;
        p66_bias_io io, fio, rio;
        agc_native_program fp, rp;
        enum exec_status sf, sr;
        const char *why = 0;
        int no_io = pick(64u) == 0u;

        state(&start, &io);
        fa = ra = start;
        fio = rio = io;
        memset(&fp, 0, sizeof fp);
        memset(&rp, 0, sizeof rp);
        sf = faithful_lm_sps_backup_basic.entries[0].call(&fp, &fa, no_io ? 0 : &fio);
        sr = lm_sps_backup_basic.entries[0].call(&rp, &ra, no_io ? 0 : &rio);
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
        if (why && ++failures <= 5u) {
            unsigned w;
            printf("FAIL case %u: %s (status %d, faithful %d)\n", n, why, (int)sr, (int)sf);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
        }
    }
    if (ran < RUNS / 2u) {
        printf("FAIL: the faithful routine ran in %u runs\n", ran);
        ++failures;
    }
    printf("sps-backup-cases: %u runs, %u faithful runs, %u failures\n", RUNS, ran, failures);
    return failures != 0u;
}
