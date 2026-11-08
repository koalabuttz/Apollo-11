/* SPDX-License-Identifier: MIT */
/* The readable IMU_CALIBRATION_AND_ALIGNMENT of the CM against the
 * faithful port's, key by key (make cm-imu-cal-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_imu_cal_routines.c) and
 * the faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the keys, which must be the faithful tables', and every basic key
 * must run readable code (the order pairs not yet written, the faithful
 * port's). Then the runs: each
 * readable key from 4,096 states (each other key, which runs the faithful
 * routine through the readable table, from 16), each one of the port's
 * frozen test states (c64-port/tests/cm_imu_calibration_vectors.h) entered
 * at the key, most stirred further: the returns the file keeps (QPLACE,
 * QPLACES, MPAC and Q: mostly a word the file's callers return to, now and
 * then 3, a word below 010 or any), NDXCTR (mostly 0, 1, -0 or -1, now and
 * then one that sends INDEX NDXCTR; TC +1 out of memory), PIPINDEX,
 * POSITON and RESULTCT (mostly within their tables, now and then one that
 * lands an INDEX at Z, below 010 or out of erasable memory), the counts and switches
 * (LENGTHOT, GEOCOMP1, TORQNDX, ZERONDX), NEWJOB, MODREG (often P07),
 * FLAGWRD3 and FLAGWRD5, the scalers (LOSCALAR often 37777, where FINETIME
 * waits) and channel 30, A and L (now and then an overflow), any erasable
 * word, the latches, EBANK, FB, or a state the routines refuse (BBANK not
 * the two banks, ZERO not +0, a word of 16 bits, the superbank bit set).
 * An order pair's state is at NEWOPS with LOC the key (pair_state()).
 * FINETIME's keys keep EBANK 5 (but now and then EBANK 0 to 3 with Q 3,
 * a return to an erasable word): the faithful port reads the file's
 * switched words in EBANK 5 whatever EBANK holds, which the readable code
 * does not model (it refuses such a run when it comes back into bank 33).
 * Each runs once on each side, as the program's trampoline would call the
 * key's routine, with a hardware record or (now and then) none. Where
 * either port runs (or stops where the AGC waits, EXEC_IDLE), the other
 * must too, with the same status, memory and io record, word for word;
 * where the readable one refuses, it must leave everything as it found
 * it; and the faithful routine must run in at least 1 of 16 of each
 * readable key's states. A timer stops a faithful run that goes on past
 * 10 ms (a return that comes back to itself for ever); the readable one
 * must refuse such a state (its run limit). */
#define _POSIX_C_SOURCE 200809L /* sigsetjmp, setitimer */
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_native_image.h"
#include "agc_transfer.h"
#include "agc_input_channels.h"
#include "cm_imu_calibration_vectors.h"
#include "../src/agc/names.h"

extern const agc_routine_table cm_imu_calibration_basic, cm_imu_calibration_interpretive;
extern const agc_routine_table faithful_cm_imu_calibration_basic,
    faithful_cm_imu_calibration_interpretive;
bool cm_imu_cal_readable(uint16_t key, bool order_pair);

/* A word's place in the state: E5's words at 02400 + (the address -
 * 01400), unswitched ones at their address. */
#define AT(name) ((unsigned)K_##name >= 01400u ? (unsigned)K_##name + 01000u : (unsigned)K_##name)

/* The interpreter's words, and the work area's (from FIXLOC). */
#define NEWOPS_AT   06037u
#define EDOP_AT     0023u
#define INTBIT15_AT 0115u
#define FIXLOC_AT   0120u
#define MPAC_AT     0154u
#define MODE_AT     0163u
#define LOC_AT      0164u
#define BANKSET_AT  0165u
#define PUSHLOC_AT  0166u
#define X1_OFF      0046u
#define S2_OFF      0051u
#define QPRET_OFF   0052u

#define CASES       4096u
#define OTHER_CASES 16u

static uint32_t seed = 1471u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned pick(unsigned n)
{
    return (unsigned)((rnd() >> 6) % n);
}

/* A run that loops for ever: what the timer stops it with. */
#define LOOPS ((enum exec_status) - 1)

static sigjmp_buf stopped;

static void stop(int signal)
{
    (void)signal;
    siglongjmp(stopped, 1);
}

static enum exec_status run(const agc_routine_entry *entry, agc_native_program *program,
                            apollo_overlay_state *state, cm_channel_io *io)
{
    static const struct itimerval limit = {{0, 0}, {0, 10000}}, off = {{0, 0}, {0, 0}};
    enum exec_status status;

    if (sigsetjmp(stopped, 1)) return LOOPS;
    setitimer(ITIMER_REAL, &limit, NULL);
    status = entry->call(program, state, io);
    setitimer(ITIMER_REAL, &off, NULL);
    return status;
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

static uint16_t any_word(void)
{
    return (uint16_t)(rnd() & 077777u);
}

/* A word of a number: often an edge value. */
static uint16_t value_word(void)
{
    static const uint16_t edges[] = {0u,      077777u, 1u,      077776u, 020000u,
                                     057777u, 037777u, 040000u, 010000u, 067777u};
    return pick(4u) ? any_word() : edges[pick(sizeof edges / sizeof edges[0])];
}

/* A sign CCS branches on: often +0, -0, 1 or -1. */
static uint16_t sign_word(void)
{
    unsigned k = pick(8u);

    return k < 2u ? 0u : k == 2u ? 077777u : k == 3u ? 1u : k == 4u ? 077776u : value_word();
}

/* A small signed number as a word. */
static uint16_t number(int v)
{
    return (uint16_t)(v < 0 ? 077777u - (unsigned)-v : (unsigned)v);
}

/* A 16-bit register: mostly a number sign-extended, now and then an
 * overflow. */
static uint16_t register_word(uint16_t v)
{
    v = (uint16_t)(v & 040000u ? v | 0100000u : v);
    return pick(8u) ? v : (uint16_t)(v ^ 0100000u);
}

/* The words the file's callers return to (their TCs' next words), but
 * those that go back through the same word (TC QPLACE, TC QPLACES, TC
 * MPAC), which would come back to themselves for ever. */
static const uint16_t returns[] = {
    02012u, 02033u, 02043u, 02102u, 02107u, 02134u, 02146u, 02173u, 02202u, 02226u, 02251u,
    02262u, 02267u, 02350u, 02472u, 02527u, 02530u, 02531u, 02633u, 02637u, 02643u, 02314u,
    02436u, 02440u, 03067u, 03375u, 03107u, 02063u, 02324u, 02522u, 02552u, 03406u, 04554u,
    03070u, 03166u, 03167u, 03320u, 03356u, 03443u, 03545u, 03572u, 03272u, 03466u, 03635u,
};

/* A return: one of the above, 3, a word below 010, or any. */
static uint16_t return_word(void)
{
    unsigned k = pick(16u);

    return k < 11u    ? returns[pick(sizeof returns / sizeof returns[0])]
           : k == 11u ? 3u
           : k == 12u ? (uint16_t)pick(010u)
           : k == 13u ? (uint16_t)(02000u + pick(06000u))
                      : value_word();
}

/* An index for an INDEX into a table at base: mostly within top words,
 * now and then one that lands at Z, or out of erasable memory, or any. */
static uint16_t table_index(unsigned base, unsigned top)
{
    unsigned k = pick(16u);

    return k < 12u    ? (uint16_t)pick(top)
           : k == 12u ? number(5 - (int)base)
           : k == 13u ? number(pick(2u) ? -(int)base - 1 : (int)(02000u - base))
                      : sign_word();
}

/* A CADR a GOTO or RVQ goes to: mostly one in fixed memory, now and then
 * any word. */
static uint16_t cadr_word(void)
{
    return pick(8u) ? (uint16_t)(04000u + pick(074000u)) : value_word();
}

/* An index register as the filter's loops set it: a count (0 to 12) or
 * its negative, and in ALCGKK to ALKCG2's SXA,1 (the gains' load) X1 also
 * ALX1S's index into the gains (0 to 144D, even). (An index that takes an
 * operand out of its E-bank, or into fixed memory, is a state the program
 * never reaches, where the two ports differ: src/binding/
 * cm_imu_cal_routines.c.) */
static uint16_t index_word(bool gains)
{
    unsigned k = pick(4u);

    return k < 2u              ? (uint16_t)pick(13u)
           : k == 2u || !gains ? number(-(int)pick(13u))
                               : (uint16_t)(2u * pick(0111u));
}

/* An order pair's state: at NEWOPS with LOC the key, EDOP +0 (as DANZIG
 * leaves it before every NEWOPS), INTBIT15 the bank's bit 15 (now and then
 * not), a VAC area as the work area (now and then not), PUSHLOC in it, and
 * for most n MPAC, MODE (double or triple precision, or a vector) and the
 * work area's words stirred: X1 and X2, S2 (where EARTHR's ITA keeps its
 * return) and QPRET (RVQ's) mostly a CADR in fixed memory. */
static void pair_state(uint16_t *w, uint16_t key, unsigned n)
{
    unsigned k, fixloc = w[FIXLOC_AT];

    w[5] = NEWOPS_AT;
    w[LOC_AT] = (uint16_t)(02000u + key % 1024u);
    w[EDOP_AT] = 0u;
    if (n % 4u != 0u || fixloc < 0401u || fixloc > 0661u || (fixloc - 0401u) % 054u)
        fixloc = w[FIXLOC_AT] = (uint16_t)(0401u + 054u * pick(5u));
    if (n % 4u != 0u) {
        for (k = 0; k < 7u; ++k) w[MPAC_AT + k] = value_word();
        k = pick(3u);
        w[MODE_AT] = k == 0u ? 0u : k == 1u ? 1u : 077776u;
        for (k = 0; k < 053u; ++k)
            if (pick(2u)) w[fixloc + k] = value_word();
        w[fixloc + X1_OFF] = index_word(key % 1024u >= 0670u && key % 1024u <= 0702u);
        w[fixloc + X1_OFF + 1u] = index_word(false);
        /* At ALKLP +021 (DLOAD* ALSK +1,1) X1 mostly as LXC,1 CMPX1 sets
         * it: +1 or -1. */
        if (key % 1024u == 0760u && pick(4u)) w[fixloc + X1_OFF] = pick(2u) ? 1u : 077776u;
        w[fixloc + S2_OFF] = cadr_word();
        w[fixloc + QPRET_OFF] = cadr_word();
        /* Now and then a work area the routines do not take. */
        if (pick(64u) == 0u) fixloc = w[FIXLOC_AT] = (uint16_t)pick(02000u);
    }
    if (w[PUSHLOC_AT] < fixloc || w[PUSHLOC_AT] > fixloc + 037u || n % 4u != 0u)
        w[PUSHLOC_AT] = (uint16_t)(fixloc + pick(040u));
    w[INTBIT15_AT] = (uint16_t)(w[4] & 040000u);
    if (n % 4u != 0u && pick(32u) == 0u) w[INTBIT15_AT] = any_word();
}

/* The frozen cases. */
static unsigned mission_cases[CM_IMU_CALIBRATION_VECTORS], mission_count;

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_channel_io *io, uint16_t key, bool interpretive,
                  unsigned n)
{
    const irc_case *c = &cm_imu_calibration_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb;
    bool fixed_fixed = key / 1024u == 02u;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    irc_inputs(c, &io->in);
    io->io.superbank = c->superbank;
    io->io.interrupt_enabled = c->interrupt_enabled;
    w[3] = 02400u; /* E5, the file's */
    w[4] = (uint16_t)((key / 1024u) << 10);
    w[5] = (uint16_t)(fixed_fixed ? 04000u + key % 1024u : 02000u + key % 1024u);
    /* (FINETIME's caller, CHECKG, is in bank 33; now and then any bank.) */
    if (fixed_fixed) w[4] = (uint16_t)((pick(4u) ? 033u : pick(040u)) << 10);
    eb = (uint16_t)(w[3] >> 8);
    if (n % 4u != 0u) {
        /* The returns. */
        w[AT(QPLACE)] = return_word();
        w[AT(QPLACES)] = return_word();
        w[K_MPAC] = return_word();
        w[2] = register_word(fixed_fixed && pick(2u) ? 02350u /* CHECKG1 +024 */
                                                     : return_word());
        /* (FINETIME's keys now and then under EBANK 0 to 3, back through
         * Q 3: TC 3 runs EBANK as TC, to an erasable word.) */
        if (fixed_fixed && pick(8u) == 0u) {
            w[3] = (uint16_t)(pick(4u) << 8);
            eb = (uint16_t)(w[3] >> 8);
            w[2] = 3u;
        }
        /* The indexes. */
        w[AT(NDXCTR)] = pick(4u) ? sign_word() : table_index(02133u, 06000u);
        w[AT(PIPINDEX)] = table_index(K_PIPAX, 3u);
        w[AT(POSITON)] = table_index(K_SOUTHDR - 2u, 020u);
        w[AT(RESULTCT)] = pick(2u)   ? (uint16_t)(pick(2u) ? 1u : 5u)
                          : pick(8u) ? table_index(K_DATAPL, 6u)
                                     : number((int)pick(4u) - (int)K_DATAPL - 1);
        /* The counts and switches. */
        w[AT(LENGTHOT)] = pick(2u) ? number((int)pick(8u) - 2) : sign_word();
        w[AT(GEOCOMP1)] = sign_word();
        w[AT(TORQNDX)] = sign_word();
        w[AT(ZERONDX)] = value_word();
        w[AT(GTSWTLT1)] = value_word();
        w[K_NEWJOB] = pick(2u) ? 0u : pick(2u) ? (uint16_t)(rnd() & 037777u) : value_word();
        w[K_MODREG] = pick(2u) ? 7u : sign_word();
        w[AT(PREMTRXC)] = sign_word();
        w[AT(GEOSAVE1)] = sign_word();
        w[K_LGYRO] = sign_word();
        w[K_FLAGWRD1] = value_word();
        w[AT(RUN)] = pick(2u)   ? (uint16_t)(pick(2u) ? 0u : 2u)
                     : pick(8u) ? table_index(K_TAZEL1 + 1u, 4u)
                                : number(-(int)K_TAZEL1 - 1); /* (a DCA of words -1 and 0) */
        w[AT(QPLAC)] = return_word();
        w[K_MARKSTAT] = pick(4u) ? table_index(K_QPRET, 0400u) : value_word();
        w[K_FLAGWRD3] = value_word();
        w[K_FLAGWRD5] = value_word();
        /* The PIPAs, often as they were a moment ago. */
        w[K_PIPAX + pick(3u)] = pick(2u) ? (uint16_t)(rnd() & 7u) : value_word();
        io->in.channel30 = any_word();
        if (pick(4u) == 0u) io->in.channel4 = 037777u;
        /* Any erasable word. */
        for (k = 010u; k < 04000u; ++k)
            if (pick(64u) == 0u) w[k] = value_word();
        w[0] = register_word(value_word());
        w[1] = register_word(value_word());
        if (pick(16u) == 0u) io->io.superbank = (uint16_t)(rnd() & (pick(2u) ? 0160u : 0177u));
        if (pick(32u) == 0u) io->io.interrupt_enabled = (uint16_t)pick(3u);
        if (pick(32u) == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (!fixed_fixed && pick(16u) == 0u) {
            w[3] = pick(4u) ? (uint16_t)(pick(8u) << 8) : any_word();
            if (w[3] <= 03400u && !(w[3] & 0377u)) eb = (uint16_t)(w[3] >> 8);
        }
    }
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, ZERO
     * not +0, or a word of 16 bits. */
    if (n % 4u != 0u && pick(64u) == 0u) w[6] = any_word();
    if (n % 4u != 0u && pick(64u) == 0u) w[7] = any_word();
    if (n % 4u != 0u && pick(64u) == 0u) w[3u + pick(03775u)] |= 0100000u;
    if (interpretive) {
        pair_state(w, key, n);
        w[BANKSET_AT] = pick(32u) ? w[6] : any_word();
    }
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     bool order_pairs, unsigned *runs, unsigned *written, unsigned *loops)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical;
        bool readable_key = cm_imu_cal_readable(key, order_pairs);
        unsigned ran = 0, bad = 0, cases = readable_key ? CASES : OTHER_CASES;

        *written += readable_key;
        for (n = 0; n < cases; ++n) {
            apollo_overlay_state start, fa, ra;
            cm_channel_io io, fio, rio;
            agc_hardware fhw, rhw;
            agc_native_program fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, order_pairs, n);
            fa = ra = start;
            fio = rio = io;
            memset(&fhw, 0, sizeof fhw);
            rhw = fhw;
            memset(&fp, 0, sizeof fp);
            rp = fp;
            if (n % 64u != 63u) { /* now and then none */
                fp.hardware = &fhw;
                rp.hardware = &rhw;
            }
            sf = run(&faithful->entries[i], &fp, &fa, &fio);
            sr = run(&readable->entries[i], &rp, &ra, &rio);
            ran += sf == EXEC_OK || sf == EXEC_IDLE;
            *loops += sf == LOOPS;
            if (!readable_key) { /* the faithful routine on both sides */
                if (sf != sr || (sf != LOOPS &&
                                 (memcmp(&fa, &ra, sizeof fa) || memcmp(&fio, &rio, sizeof fio))))
                    why = "the table does not run the faithful routine";
            } else if (sr == LOOPS) {
                why = "readable loops";
            } else if (sr != EXEC_OK && sr != EXEC_IDLE &&
                       (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio))) {
                why = "readable refusal changed the state";
            } else if (sf == LOOPS)
                why = sr == EXEC_OUT_OF_SCOPE ? NULL : "faithful loops, readable does not refuse";
            else if ((sf == EXEC_OK || sf == EXEC_IDLE) && sr != sf)
                why = sr == EXEC_OK || sr == EXEC_IDLE ? "different status"
                                                       : "readable refused, faithful ran";
            else if (sf != EXEC_OK && sf != EXEC_IDLE && (sr == EXEC_OK || sr == EXEC_IDLE))
                why = "readable ran, faithful refused";
            else if ((sf == EXEC_OK || sf == EXEC_IDLE) && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if ((sf == EXEC_OK || sf == EXEC_IDLE) &&
                     (memcmp(&fio, &rio, sizeof fio) || memcmp(&fhw, &rhw, sizeof fhw)))
                why = "different io record";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL %02o,%04o case %u: %s (status %d, faithful %d)\n", key / 1024u,
                       02000u + key % 1024u, n, why, (int)sr, (int)sf);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (readable_key && ran < cases / 16u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine ran in %u of %u states\n", key / 1024u,
                   02000u + key % 1024u, ran, cases);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned i, failures = 0, runs = 0, basic = 0, pairs = 0, loops = 0;
    struct sigaction on_alarm;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_IMU_CALIBRATION_VECTORS; ++i)
        if (cm_imu_calibration_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    failures +=
        tables_differ(&cm_imu_calibration_basic, &faithful_cm_imu_calibration_basic, "basic");
    failures += tables_differ(&cm_imu_calibration_interpretive,
                              &faithful_cm_imu_calibration_interpretive, "interpretive");
    for (i = 0; i < faithful_cm_imu_calibration_basic.count; ++i)
        basic += cm_imu_cal_readable(faithful_cm_imu_calibration_basic.entries[i].physical, false);
    if (basic != faithful_cm_imu_calibration_basic.count) { /* (every basic key) */
        printf("FAIL: %u basic keys readable of %u\n", basic,
               faithful_cm_imu_calibration_basic.count);
        ++failures;
    }
    if (failures) {
        printf("cm-imu-cal-cases: %u failures\n", failures);
        return 1;
    }
    basic = 0;
    memset(&on_alarm, 0, sizeof on_alarm);
    on_alarm.sa_handler = stop;
    sigaction(SIGALRM, &on_alarm, NULL);
    failures += keys(&cm_imu_calibration_basic, &faithful_cm_imu_calibration_basic, false, &runs,
                     &basic, &loops);
    failures += keys(&cm_imu_calibration_interpretive, &faithful_cm_imu_calibration_interpretive,
                     true, &runs, &pairs, &loops);
    printf("cm-imu-cal-cases: %u of %u keys readable (%u basic, %u order pairs) from %u states "
           "each, the others from %u, %u run by the faithful routines, %u looping, %u failures\n",
           basic + pairs, cm_imu_calibration_basic.count + cm_imu_calibration_interpretive.count,
           basic, pairs, CASES, OTHER_CASES, runs, loops, failures);
    return failures != 0;
}
