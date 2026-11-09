/* SPDX-License-Identifier: MIT */
/* The readable CM_ENTRY_DIGITAL_AUTOPILOT against the faithful port's, key
 * by key (make cm-entry-dap-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_entry_dap_routines.c)
 * and the faithful port's under the same names with faithful_ in front
 * (the Makefile renames them), which also run the keys no part has
 * written yet.
 *
 * First the keys, which must be the faithful table's, and the hot table's,
 * which must be the faithful one's with the same routines as the basic
 * table; the ones the parts written so far cover must run readable code
 * (parts A to C: READGYMB to the dead zone tests, FLUSHJET, CM/DAPON
 * and CM/DAPIC). Then
 * the runs: each readable key from 4,096 states (the others from 16),
 * each one of the port's frozen test states
 * (c64-port/tests/cm_entry_dap_vectors.h) entered at the key, most
 * stirred further: CM/FLAGS (mostly its switches), IMODES33's fine align
 * bit, the CDUs and the last gimbal angles, JETEM (often +0, -0, 1 or
 * -1), CM/GYMDT, PIPTIME and TIME1, ALFA/180 and ROLL/180, FLAGWRD2,
 * DAPDATR1, the gimbal angles' changes and their sines and cosines, the
 * rates and their last changes, GAMDOT (often +0, -0, 1 or -1), the
 * commanded angles, SR, A now and then a jet code tag (or an index that
 * lands on LIMITS or in erasable memory), A and L about a dead zone's
 * edge, JETAG, JETEM2, CALFA, SALFA, CMDAPMOD and
 * P63FLAG (often +0, -0, 1 or -1), the jets' channels 5 and 6, any erasable bank's words, A
 * (often an overflow), L, Q (often 3, or a key of the file's to return
 * to), FB (any for FLUSHJET), EBANK (any for CM/DAPON and CM/DAPIC), the
 * latches, or a state the routines refuse (BBANK not the two banks, ZERO
 * not +0, a word of 16 bits). Each runs once on each side, as the
 * program's trampoline would call the key's routine, with a hardware
 * record or (now and then) none. Where either port runs, the other must
 * too, with the same memory, io and hardware records, word for word;
 * where the readable one refuses, it must leave everything as it found
 * it; and the faithful routine must run in at least 1 of 16 of each
 * readable key's states. A timer stops a run that goes on past a tenth
 * of a second; such a state must loop on both sides. */
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
#include "cm_entry_dap_vectors.h"
#include "../src/agc/names.h"

extern const agc_routine_table cm_entry_dap_basic, cm_entry_dap_interpretive,
    cm_entry_dap_hot_routines;
extern const agc_routine_table faithful_cm_entry_dap_basic, faithful_cm_entry_dap_interpretive,
    faithful_cm_entry_dap_hot_routines;
bool cm_entry_dap_readable(uint16_t key);

/* A word's place in the state: E6's words at 03000 + (the address -
 * 01400), unswitched ones at their address. */
#define AT(name) ((unsigned)K_##name >= 01400u ? (unsigned)K_##name + 01400u : (unsigned)K_##name)

#define CASES       4096u
#define OTHER_CASES 16u

static uint32_t seed = 1039u;
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

/* A key's routine, run until it returns, or stopped by the timer. */
static enum exec_status run(const agc_routine_entry *entry, agc_native_program *program,
                            apollo_overlay_state *state, cm_native_io *io)
{
    static const struct itimerval tenth = {{0, 0}, {0, 100000}}, off = {{0, 0}, {0, 0}};
    enum exec_status status;

    if (sigsetjmp(stopped, 1)) return LOOPS;
    setitimer(ITIMER_REAL, &tenth, NULL);
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

/* Each hot key runs the basic table's routine for it. */
static unsigned hot_differs(void)
{
    unsigned i, j, failures = 0;

    for (i = 0; i < cm_entry_dap_hot_routines.count; ++i) {
        const agc_routine_entry *hot = &cm_entry_dap_hot_routines.entries[i];
        for (j = 0; j < cm_entry_dap_basic.count; ++j)
            if (cm_entry_dap_basic.entries[j].physical == hot->physical) break;
        if (j == cm_entry_dap_basic.count || cm_entry_dap_basic.entries[j].call != hot->call) {
            printf("FAIL: hot key %u does not run the basic table's routine\n", hot->physical);
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
    return pick(4u) ? any_word() : edges[pick(sizeof edges / sizeof edges[0])];
}

/* A sign CCS branches on: often +0, -0, 1 or -1. */
static uint16_t sign_word(void)
{
    unsigned k = pick(8u);

    return k < 2u ? 0u : k == 2u ? 077777u : k == 3u ? 1u : k == 4u ? 077776u : value_word();
}

/* A word in a 16-bit register, its sign copied into bit 16. */
static uint16_t sign_extended(uint16_t v)
{
    return (uint16_t)(v & 040000u ? v | 0100000u : v);
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

/* The frozen cases. */
static unsigned mission_cases[CM_ENTRY_DAP_VECTORS], mission_count;

/* The keys parts A to C cover: READGYMB to the dead zones and FLUSHJET
 * (indexes 0 to 44), CM/DAPON and CM/DAPIC (120 to 137). */
static bool written_part(unsigned index)
{
    return index <= 44u || (index >= 120u && index <= 137u);
}

/* The file's words a TC Q returns to: after FLUSHJET at READGYMB +012,
 * DOBRATE? +010 and CM/DAPON +015, DOBRATE? +4 to +7, after RATEAVG
 * and ANGOVCOR, and after the dead zone tests. */
static const uint16_t returns[] = {02446u, 02476u, 03627u, 02472u, 02473u, 02474u, 02475u,
                                   02570u, 02604u, 02623u, 02676u, 02730u, 02734u, 02741u,
                                   02763u, 02770u, 03016u, 03112u, 03123u};

/* A Q to return to: 3, one of the file's words, a word of fixed memory,
 * or any. */
static uint16_t return_word(void)
{
    unsigned k = pick(8u);

    return k < 2u     ? 3u
           : k < 5u   ? returns[pick(sizeof returns / sizeof returns[0])]
           : k == 5u  ? (uint16_t)(02000u + pick(06000u))
           : pick(2u) ? any_word()
                      : rnd();
}

/* A bank: mostly the file's (15 and 20), or any. */
static uint16_t bank_word(void)
{
    unsigned k = pick(4u);

    return k == 0u ? 032000u : k == 1u ? 040000u : (uint16_t)(rnd() & 076000u);
}

/* A value about the edge of a dead zone, either sign: an error about
 * 3DDZ's (329), or 3DDZ's moved by CM/BIAS (55); a rate about 2D/SDZ's
 * (18) or 4D/SDZ's (36). */
static uint16_t edge_word(bool rate)
{
    static const int errors[] = {329, 329 + 55, 329 - 55}, rates[] = {18, 36};
    int v = (rate ? rates[pick(2u)] : errors[pick(3u)]) + (int)pick(9u) - 4;

    return number(pick(2u) ? v : -v);
}

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, agc_hardware *hw, uint16_t key,
                  unsigned n)
{
    const irc_case *c = &cm_entry_dap_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb;
    unsigned bank = key / 1024u, k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    w[3] = 03000u;   /* E6, the file's */
    if (bank < 4u) { /* fixed-fixed: FLUSHJET, under any FB */
        w[4] = bank_word();
        w[5] = (uint16_t)(04000u + (bank - 2u) * 02000u + key % 1024u);
    } else {
        w[4] = (uint16_t)(bank << 10);
        w[5] = (uint16_t)(02000u + key % 1024u);
    }
    eb = (uint16_t)(w[3] >> 8);
    if (n % 4u != 0u) {
        /* CM/FLAGS, mostly its switches (bits 1 to 4, 11 and 12). */
        w[AT(CM_FLAGS)] = pick(2u) ? (uint16_t)(rnd() & 06017u) : any_word();
        w[AT(IMODES33)] = pick(2u) ? (uint16_t)(rnd() & 040u) : any_word();
        /* The CDUs and the last gimbal angles. */
        w[AT(CDUX)] = any_word();
        w[AT(CDUY)] = any_word();
        w[AT(CDUZ)] = any_word();
        w[AT(AOG)] = pick(2u) ? w[AT(CDUX)] : any_word();
        w[AT(AIG)] = pick(2u) ? w[AT(CDUY)] : any_word();
        w[AT(AMG)] = pick(2u) ? w[AT(CDUZ)] : any_word();
        /* JETEM, CM/GYMDT, PIPTIME and TIME1. */
        w[AT(JETEM)] = pick(2u) ? sign_word() : number((int)pick(200u) - 100);
        w[AT(CM_GYMDT)] = value_word();
        w[AT(PIPTIME) + 1u] = value_word();
        w[AT(TIME1)] = value_word();
        /* ALFA/180, BETA/180 and ROLL/180, FLAGWRD2 and DAPDATR1. */
        w[AT(ALFA_180)] = value_word();
        w[AT(BETA_180)] = value_word();
        w[AT(ROLL_180)] = value_word();
        w[AT(FLAGWRD2)] = any_word();
        w[AT(DAPDATR1)] = any_word();
        /* The gimbal angles' changes and their sines and cosines, the
         * rates and their last changes, GAMDOT, the commanded angles. */
        w[AT(MDELAOG)] = value_word();
        w[AT(MDELAIG)] = value_word();
        w[AT(MDELAMG)] = value_word();
        w[AT(OLDELP)] = value_word();
        w[AT(OLDELQ)] = value_word();
        w[AT(OLDELR)] = value_word();
        w[AT(COSM)] = value_word();
        w[AT(SINO)] = value_word();
        w[AT(COSO)] = value_word();
        w[AT(SINOCOSM)] = value_word();
        w[AT(COSOCOSM)] = value_word();
        w[AT(PREL)] = value_word();
        w[AT(QREL)] = value_word();
        w[AT(RREL)] = value_word();
        w[AT(PHIDOT)] = value_word();
        w[AT(SALFA)] = value_word();
        w[AT(GAMDOT)] = sign_word();
        w[AT(ALFACOM)] = value_word();
        w[AT(BETACOM)] = value_word();
        w[AT(SR)] = any_word();
        /* The jet tag, the extra atmospheric DAP's rate, CALFA and SALFA,
         * the mode. */
        w[AT(JETAG)] = sign_word();
        w[AT(JETEM2)] = pick(2u) ? sign_word() : number((int)pick(400u) - 200);
        w[AT(CALFA)] = sign_word();
        w[AT(SALFA)] = pick(2u) ? sign_word() : value_word();
        w[AT(CMDAPMOD)] = sign_word();
        w[AT(P63FLAG)] = sign_word();
        hw->channel5 = any_word();
        hw->channel6 = any_word();
        /* Any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (pick(32u) == 0u) w[k] = value_word();
        /* A (often an overflow), L and Q. */
        w[0] = value_word();
        w[0] = pick(4u) ? register_word(w[0]) : (uint16_t)(register_word(w[0]) ^ 0100000u);
        if (pick(8u) == 0u) {
            /* A jet code tag: -1, +0 or +1, or an index that lands from
             * either table on LIMITS or in erasable memory. */
            int table = pick(2u) ? 03202 : 03205;
            unsigned k2 = pick(4u);
            w[0] = k2 < 2u    ? number((int)pick(3u) - 1)
                   : k2 == 2u ? number(04777 + (int)pick(4u) - table)
                   : pick(8u) ? number((int)pick(02000u) - table)
                              : number(5 - table);
            w[0] = sign_extended(w[0]);
        }
        w[1] = register_word(value_word());
        if (pick(4u) == 0u) { /* A and L about a dead zone's edge */
            w[0] = sign_extended(edge_word(pick(2u)));
            w[1] = sign_extended(edge_word(false));
            if (pick(4u) == 0u) /* an overflow, the word it leaves small */
                w[0] = (uint16_t)(pick(2u) ? 040000u + pick(40u) : 0137777u - pick(40u));
        }
        w[2] = return_word();
        if (pick(16u) == 0u) io->superbank = (uint16_t)(rnd() & (pick(2u) ? 0160u : 0177u));
        if (pick(32u) == 0u) io->interrupt_enabled = (uint16_t)pick(3u);
        if (pick(32u) == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (pick(8u) == 0u) {
            w[3] = pick(4u) ? (uint16_t)(pick(8u) << 8) : any_word();
            if (w[3] <= 03400u && !(w[3] & 0377u)) eb = (uint16_t)(w[3] >> 8);
        }
        if (pick(64u) == 0u) { /* an EBANK past E7, with BBANK agreeing */
            w[3] = (uint16_t)(pick(0100u) << 8);
            eb = (uint16_t)(w[3] >> 8);
        }
    }
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, ZERO
     * not +0, or a word of 16 bits. */
    if (n % 4u != 0u && pick(64u) == 0u) w[6] = any_word();
    if (n % 4u != 0u && pick(64u) == 0u) w[7] = any_word();
    if (n % 4u != 0u && pick(64u) == 0u) w[3u + pick(03775u)] |= 0100000u;
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     unsigned *runs, unsigned *written, unsigned *loops)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical;
        bool readable_key = cm_entry_dap_readable(key);
        unsigned ran = 0, bad = 0, cases = readable_key ? CASES : OTHER_CASES;

        if (readable_key != written_part(i)) {
            printf("FAIL: key %u (%04o) %s\n", key, 02000u + key % 1024u,
                   readable_key ? "runs readable code, no part has it" : "has no readable routine");
            ++failures;
        }
        *written += readable_key;
        for (n = 0; n < cases; ++n) {
            apollo_overlay_state start, fa, ra;
            cm_native_io io, fio, rio;
            agc_hardware hardware, fhw, rhw;
            agc_native_program p, fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, &hardware, key, n);
            fa = ra = start;
            fio = rio = io;
            fhw = rhw = hardware;
            memset(&p, 0, sizeof p);
            fp = rp = p;
            if (n % 64u != 63u) { /* now and then none */
                fp.hardware = &fhw;
                rp.hardware = &rhw;
            }
            sf = run(&faithful->entries[i], &fp, &fa, &fio);
            sr = run(&readable->entries[i], &rp, &ra, &rio);
            ran += sf == EXEC_OK;
            *loops += sf == LOOPS;
            if ((sf == LOOPS) != (sr == LOOPS))
                why = sf == LOOPS ? "faithful loops, readable does not" : "readable loops";
            else if (sf == LOOPS)
                why = NULL;
            else if (!readable_key) { /* the faithful routine on both sides */
                if (sf != sr || memcmp(&fa, &ra, sizeof fa) || memcmp(&fio, &rio, sizeof fio) ||
                    memcmp(&fhw, &rhw, sizeof fhw))
                    why = "the faithful routine twice, two results";
            } else if (sr != EXEC_OK &&
                       (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio) ||
                        memcmp(&rhw, &hardware, sizeof rhw)))
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
                printf("FAIL key %u (%04o) case %u: %s\n", key, 02000u + key % 1024u, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (readable_key && ran < CASES / 16u && bad == 0u) {
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
    unsigned i, failures = 0, runs = 0, written = 0, loops = 0;
    struct sigaction on_alarm;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_ENTRY_DAP_VECTORS; ++i)
        if (cm_entry_dap_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    failures += tables_differ(&cm_entry_dap_basic, &faithful_cm_entry_dap_basic, "basic");
    failures += tables_differ(&cm_entry_dap_interpretive, &faithful_cm_entry_dap_interpretive,
                              "interpretive");
    failures +=
        tables_differ(&cm_entry_dap_hot_routines, &faithful_cm_entry_dap_hot_routines, "hot");
    failures += hot_differs();
    if (failures) {
        printf("cm-entry-dap-cases: %u failures\n", failures);
        return 1;
    }
    memset(&on_alarm, 0, sizeof on_alarm);
    on_alarm.sa_handler = stop;
    sigaction(SIGALRM, &on_alarm, NULL);
    failures += keys(&cm_entry_dap_basic, &faithful_cm_entry_dap_basic, &runs, &written, &loops);
    printf(
        "cm-entry-dap-cases: %u of %u keys readable (parts A to C, all basic) from %u states each "
        "(the others %u), %u run by the faithful routines, %u looping on both sides, %u "
        "failures\n",
        written, faithful_cm_entry_dap_basic.count, CASES, OTHER_CASES, runs, loops, failures);
    return failures != 0;
}
