/* SPDX-License-Identifier: MIT */
/* The readable interpreter's basic routines against the faithful port's,
 * case by case (make interpreter-cases' last step).
 *
 * The step before this one (tests/interpreter_fuzz.c) runs the interpreter
 * as interpretive code drives it, pass by pass. This test enters it where
 * basic code does: every entry of the readable table (src/binding/
 * interpreter_basic.c: DDV/BDDV, the MAXTEST prologues, UNIT, SQRTSUB,
 * SINE, POLY, DMPSUB, SHORTMP, INTPRET, NEWMODE and the rest), each on
 * random states. Three states in four are as a caller brings them. One in
 * four has a word no program leaves: MODE, MPTEMP, MAXDVSW, DVSIGN or
 * DVNORMCT of any sign and size, FIXLOC anywhere in erasable memory,
 * NEWJOB of any sign, Q or ADDRWD holding no address, an overflow in A
 * where the routine stores it. There a routine follows its instructions
 * wherever they lead, and the two ports must lead to the same place.
 * (One thing is always a caller's: where a power series lies, POWRSERS's
 * A and the words after a call of POLY. A made-up one has the INDEX of
 * every term make an instruction the listing does not intend, which the
 * readable code does not model.)
 *
 * Built once per mission, with the readable table and the faithful
 * port's (c64-port/src/lm_interpreter.c, cm_interpreter.c). The faithful
 * routines are built as the flight program has them, in three parts, each
 * with the table's name and faithful1_, faithful2_ or faithful3_ in front
 * (the Makefile renames them). A part runs until its instructions leave
 * its own words, so the faithful side of a case is run part after part,
 * until it is where the readable routine stopped or at no entry of any
 * part.
 *
 * A state a caller brings must be run by both, and where the faithful
 * routines run, the readable one must end with the same erasable memory,
 * registers and io record, word for word. Only a state no program makes
 * may be refused. Where the faithful routines refuse one (an operand
 * address outside what they take), the readable one may run. Where the
 * readable one refuses, it must leave everything as it found it, and it
 * may refuse a state the faithful routines run only for a step it does
 * not model, named by the refusal's own note:
 *
 *   - a shift count in DVNORMCT or MPTEMP that would enter no prologue of
 *     MAXTEST (the INDEX would make the TC go anywhere);
 *   - a return through a Q that holds no address (the AGC would run the
 *     word in Q as an instruction; the faithful routines keep its low
 *     twelve bits and go there).
 *
 * These are counted apart (README, Departures). */
#define _POSIX_C_SOURCE 200809L /* sigsetjmp, setitimer */
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "agc_transfer.h"
#include "rope_image.h"
#if LUMINARY099
#include "lm_interpreter.h"
#include "p66_bias.h"
#include "../src/agc/lm_names.inc"
typedef p66_bias_io mission_io;
#define MISSION    "LM"
#define ROPE       0u
#define NAME(n)    n##_lm_interpreter_basic
#define READABLE   lm_interpreter_basic
#define FILE_FIRST LM_INTERPRETER_FIRST
#define FILE_LAST  LM_INTERPRETER_LAST
#else
#include "cm_interpreter.h"
#include "cm_native_image.h"
#include "../src/agc/cm_names.inc"
typedef cm_native_io mission_io;
#define MISSION    "CM"
#define ROPE       1u
#define NAME(n)    n##_cm_interpreter_basic
#define READABLE   cm_interpreter_basic
#define FILE_FIRST CM_INTERPRETER_FIRST
#define FILE_LAST  CM_INTERPRETER_LAST
#endif

extern const agc_routine_table READABLE;
extern const agc_routine_table NAME(faithful1), NAME(faithful2), NAME(faithful3);
static const agc_routine_table *const parts[3] = {&NAME(faithful1), &NAME(faithful2),
                                                  &NAME(faithful3)};

/* tests/interpreter_side_seams.c: why the readable routine refused. */
const char *interpreter_refused_state(void);

#define CASES_PER_ENTRY 20000u
#define PARTS_PER_CASE  64u  /* more than any routine passes through */
#define RUN_LIMIT_MS    200L /* of the test's own processor time */
#define SHOWN_PER_ENTRY 3u

/* The central registers, by address. */
enum { K_A = 0, K_L = 1, K_Q = 2, K_EB = 3, K_FB = 4, K_Z = 5, K_BB = 6 };

#define FF(z)               (uint16_t)(2048u + (z) - 04000u) /* a fixed-fixed address's key */
#define NOT_MODELLED_SHIFT  "a shift count outside the divide's prologues"
#define NOT_MODELLED_RETURN "a return through a word that is no address"

/* ---- Random words ---- */

static uint32_t seed = 1202u;

static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static uint16_t sign_extended(uint16_t w)
{
    return (uint16_t)((w & 077777u) | ((w & 040000u) << 1));
}

static uint16_t complement(uint16_t w)
{
    return (uint16_t)(~w & 077777u);
}

/* A register: most often a word with its sign twice, sometimes 16 bits
 * that hold an overflow. */
static uint16_t reg(void)
{
    return rnd() % 16u ? sign_extended(rnd()) : (uint16_t)(rnd() | rnd() << 1);
}

/* A word of any sign and size, the two zeros often. */
static uint16_t any_sign(void)
{
    uint16_t v = (uint16_t)(1u + rnd() % 037776u);

    switch (rnd() % 4u) {
    case 0: return 0u;
    case 1: return 077777u;
    case 2: return v;
    default: return complement(v);
    }
}

/* A number: of any size, with the small, the zeros and the largest often. */
static uint16_t number(void)
{
    uint16_t v;

    switch (rnd() % 12u) {
    case 0: return 0u;
    case 1: return 077777u;
    case 2: v = (uint16_t)(1u + rnd() % 7u); break;
    case 3: v = (uint16_t)(037777u - rnd() % 4u); break;
    case 4: v = (uint16_t)(020000u + rnd() % 3u - 1u); break;
    case 5: v = (uint16_t)(1u << (rnd() % 14u)); break;
    case 6: v = (uint16_t)(rnd() % 0400u); break;
    default: v = (uint16_t)(rnd() & 037777u); break;
    }
    return rnd() % 2u ? v : complement(v);
}

/* ---- The tables ---- */

static agc_routine faithful(uint16_t key)
{
    unsigned part;
    uint16_t i;

    for (part = 0; part < 3u; ++part)
        for (i = 0; i < parts[part]->count; ++i)
            if (parts[part]->entries[i].physical == key) return parts[part]->entries[i].call;
    return 0;
}

/* The physical address Z names (bank 00's or fixed-fixed's), or 0xFFFF. */
static uint16_t key_of(const apollo_overlay_state *s, const mission_io *io)
{
    unsigned z = s->words[K_Z], bank;

    if (z >= 04000u && z <= 07777u) return FF(z);
    if (z < 02000u || z > 07777u) return 0xFFFFu;
    bank = (s->words[K_FB] >> 10) & 037u;
    if (bank >= 030u && (io->superbank & 0100u)) bank += 010u;
    return (uint16_t)(bank * 1024u + (z & 01777u));
}

/* An entry's address as the listing gives it. */
static uint16_t address_of(uint16_t key)
{
    return key >= 2048u ? (uint16_t)(04000u + key - 2048u) : (uint16_t)(02000u + key % 1024u);
}

static const char *where(uint16_t key)
{
    static char text[24];

    if (key >= 2048u)
        sprintf(text, "%05o", (unsigned)address_of(key));
    else
        sprintf(text, "%02o,%04o", (unsigned)(key / 1024u), (unsigned)address_of(key));
    return text;
}

/* ---- Where the program calls an entry ----
 * The words after each TC to a fixed-fixed entry, anywhere in the rope but
 * in the interpreter itself (a faithful part runs on through its own
 * words, where the readable routine has returned). */
static uint16_t sites[4096];
static unsigned site_count;

static void find_sites(uint16_t key)
{
    unsigned p;

    site_count = 0;
    if (key < 2048u) return;
    for (p = 2048u; p + 1u < 36u * 1024u && site_count < 4096u; ++p) {
        agc_word w = 0;

        if (p + 1u >= FILE_FIRST && p + 1u <= FILE_LAST + 2u) continue;
        if (apollo_rope_read(ROPE, (uint16_t)p, &w) && (w & 077777u) == address_of(key))
            sites[site_count++] = (uint16_t)(p + 1u);
    }
}

/* ---- A case's state ---- */

/* An operand's address, as ADDRWD holds it: in the job's work area, in
 * erasable memory after the interpreter's own words, or in the rope. */
static uint16_t operand_address(unsigned fixloc)
{
    switch (rnd() % 4u) {
    case 0: return (uint16_t)(fixloc + rnd() % 040u);
    case 1: return (uint16_t)(0400u + rnd() % 01370u);
    case 2: return (uint16_t)(02000u + rnd() % 01770u);
    default: return (uint16_t)(04000u + rnd() % 03770u);
    }
}

/* Returns whether the state has a word no program leaves. */
static bool make_state(apollo_overlay_state *s, mission_io *io, uint16_t key)
{
    unsigned i, fixloc;
    bool broken = rnd() % 4u == 0u;
    uint16_t z = address_of(key);

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[K_A] = rnd() % 3u ? reg() : sign_extended(number());
    s->words[K_L] = rnd() % 3u ? reg() : sign_extended(number());
    s->words[K_EB] = (uint16_t)((rnd() % 8u) << 8);
    s->words[K_FB] = key < 1024u ? 0u : (uint16_t)((rnd() % 040u) << 10);
    io->superbank = (uint16_t)(rnd() % 2u ? 0100u : 0u);
    io->interrupt_enabled = rnd() % 2u;
    s->words[K_Z] = z;
    /* Q: after a call of the entry somewhere in the program, or a word no
     * entry is at (but after POLY's call are the series' words, and after
     * DMP's the operand's address). */
    if (site_count && (z == K_POLY || z == K_DMP || rnd() % 4u)) {
        unsigned p = sites[rnd() % site_count], bank = p / 1024u;

        if (bank == 2u || bank == 3u) {
            s->words[K_Q] = (uint16_t)(04000u + p - 2048u);
        } else {
            if (bank >= 040u) {
                bank -= 010u;
                io->superbank = 0100u;
            } else if (bank >= 030u) {
                io->superbank = 0u;
            }
            if (key >= 1024u) s->words[K_FB] = (uint16_t)(bank << 10);
            s->words[K_Q] = (uint16_t)(02000u + p % 1024u);
        }
    } else {
        s->words[K_Q] = (uint16_t)(07740u + rnd() % 030u);
    }
    /* (The rope ends at bank 43: FB 34 to 37 with the superbank bit names
     * none.) */
    if (s->words[K_FB] >= 034u << 10) io->superbank = 0u;
    s->words[K_BB] = (uint16_t)(s->words[K_FB] | (s->words[K_EB] >> 8));

    /* The interpreter's own words, as a job keeps them. */
    fixloc = 0401u + 054u * (rnd() % 5u);
    s->words[K_FIXLOC] = (uint16_t)fixloc;
    s->words[K_PUSHLOC] = (uint16_t)(fixloc + rnd() % 046u);
    s->words[K_MODE] = rnd() % 3u ? (uint16_t)(rnd() % 2u) : 077776u;
    s->words[K_NEWJOB] = 0u;
    s->words[K_OVFIND] = (uint16_t)(rnd() % 4u == 0u);
    s->words[K_LOC] = (uint16_t)(02000u + rnd() % 02000u);
    s->words[K_BANKSET] = (uint16_t)((rnd() % 040u) << 10 | (s->words[K_EB] >> 8));
    s->words[K_INTBIT15] = (uint16_t)(s->words[K_BANKSET] & 040000u);
    s->words[K_EDOP] = 0u;
    s->words[K_ADDRWD] = operand_address(fixloc);
    s->words[K_MPTEMP] = (uint16_t)(rnd() % 16u);
    s->words[K_MAXDVSW] = rnd() % 2u ? 0u : 077776u;
    s->words[K_DVNORMCT] = complement((uint16_t)(1u + rnd() % 14u));
    s->words[K_DVSIGN] = rnd() % 2u ? 077777u : 077776u;
    for (i = 0; i < 7u; ++i) s->words[K_MPAC + i] = number();
    for (i = 0; i < 3u; ++i) s->words[K_BUF + i] = number();
    for (i = 0; i < 2u; ++i) s->words[K_BUF2 + i] = number();
    for (i = 0; i < 6u; ++i) s->words[K_VBUF + i] = number();
    for (i = 0; i < 046u; ++i)
        if (rnd() % 2u) s->words[fixloc + i] = number();
    /* Sign agreement, as most numbers have. */
    if (rnd() % 2u) {
        for (i = 0; i < 6u; i += (i == 0u ? 3u : 2u)) {
            uint16_t hi = s->words[K_MPAC + i], *lo = &s->words[K_MPAC + i + 1u];

            if ((hi & 040000u) != (*lo & 040000u)) *lo = complement(*lo);
        }
        if ((s->words[K_BUF] & 040000u) != (s->words[K_BUF + 1u] & 040000u))
            s->words[K_BUF + 1u] = complement(s->words[K_BUF + 1u]);
    }
    /* What the callers of some entries bring in A and L. */
    if (z == K_OVERFLOW) { /* A as TS leaves it after an overflow, L the component */
        s->words[K_A] = rnd() % 2u ? 1u : 0177776u;
        s->words[K_L] = (uint16_t)(2u * (rnd() % 3u));
    } else if (z == K_POWRSERS) { /* A the series in the rope, L its terms less one */
        s->words[K_A] = (uint16_t)(04000u + rnd() % 03700u);
        s->words[K_L] = (uint16_t)(rnd() % 6u);
    } else if (z == K_SHORTMP || z == K_DMPNSUB) { /* a single-precision factor */
        s->words[K_A] = sign_extended(number());
    } else if (z == K_SQRTSUB && rnd() % 16u == 0u) {
        /* The largest argument (the listing has its second Newton step overflow; none does). */
        s->words[K_MPAC] = 037777u;
        s->words[K_MPAC + 1u] = (uint16_t)(037777u - rnd() % 2u);
    }
    if (!broken) return false;

    /* One word in four of a broken state's is one no program leaves. */
    if (rnd() % 4u == 0u) s->words[K_MODE] = any_sign();
    if (rnd() % 4u == 0u) s->words[K_MPTEMP] = any_sign();
    if (rnd() % 4u == 0u) s->words[K_MAXDVSW] = any_sign();
    if (rnd() % 4u == 0u) s->words[K_DVSIGN] = any_sign();
    if (rnd() % 4u == 0u) s->words[K_DVNORMCT] = any_sign();
    if (rnd() % 8u == 0u) s->words[K_FIXLOC] = (uint16_t)(rnd() & 01777u);
    if (rnd() % 8u == 0u) s->words[K_NEWJOB] = any_sign();
    if (z == K_POWRSERS) {
        /* (The series stays where a caller has it; its count of terms may
         * be one POLY never leaves.) */
        if (rnd() % 2u)
            s->words[K_L] = rnd() % 2u ? 0177777u : sign_extended(complement(1u + rnd() % 5u));
        return true;
    }
    if (z != K_POLY && rnd() % 8u == 0u) {
        uint16_t kept = s->words[K_Q], inside;

        /* (But no word of the interpreter itself: a faithful part runs on
         * through its own words, where the readable routine has returned.) */
        s->words[K_Q] = reg();
        s->words[K_Z] = (uint16_t)(s->words[K_Q] & 07777u);
        inside = key_of(s, io);
        s->words[K_Z] = z;
        if (inside >= FILE_FIRST && inside <= FILE_LAST + 2u) s->words[K_Q] = kept;
    }
    if (rnd() % 4u == 0u) s->words[K_ADDRWD] = (uint16_t)(rnd() & 077777u);
    if (rnd() % 4u == 0u) s->words[K_A] = reg();
    if (rnd() % 4u == 0u) s->words[K_L] = reg();
    return true;
}

/* ---- Running a case ---- */

static sigjmp_buf out_of_time;

static void on_alarm(int sig)
{
    (void)sig;
    siglongjmp(out_of_time, 1);
}

static void limit(long ms)
{
    struct itimerval t;

    memset(&t, 0, sizeof t);
    t.it_value.tv_sec = ms / 1000;
    t.it_value.tv_usec = (ms % 1000) * 1000;
    setitimer(ITIMER_VIRTUAL, &t, 0);
}

enum outcome { RAN, REFUSED, NO_END };

/* The readable routine, on through every entry of its table it leaves
 * at. */
static enum outcome run_readable(agc_routine routine, const agc_native_program *program,
                                 apollo_overlay_state *s, mission_io *io)
{
    enum exec_status status;

    if (sigsetjmp(out_of_time, 1)) return NO_END;
    limit(RUN_LIMIT_MS);
    status = routine(program, s, io);
    limit(0);
    return status == EXEC_OK ? RAN : REFUSED;
}

/* The faithful routines, part after part, until Z is where the readable
 * routine stopped (stop_z) or names no entry. */
static enum exec_status faithful_parts(const agc_native_program *program, apollo_overlay_state *s,
                                       mission_io *io, bool stop, uint16_t stop_z)
{
    enum exec_status status = EXEC_UNMAPPED;
    unsigned part;

    for (part = 0; part < PARTS_PER_CASE; ++part) {
        agc_routine routine = faithful(key_of(s, io));

        if (!routine || (part && stop && s->words[K_Z] == stop_z)) break;
        status = routine(program, s, io);
        if (status != EXEC_OK) break;
    }
    return status;
}

static enum outcome run_faithful(const agc_native_program *program, apollo_overlay_state *s,
                                 mission_io *io, bool stop, uint16_t stop_z)
{
    enum exec_status status;

    if (sigsetjmp(out_of_time, 1)) return NO_END;
    limit(RUN_LIMIT_MS);
    status = faithful_parts(program, s, io, stop, stop_z);
    limit(0);
    return status == EXEC_OK ? RAN : REFUSED;
}

static void show(const char *why, uint16_t key, unsigned n, const apollo_overlay_state *start,
                 const apollo_overlay_state *readable, const apollo_overlay_state *faithful_state)
{
    unsigned w, shown = 0;

    printf("FAIL %s entry %s case %u: %s\n", MISSION, where(key), n, why);
    for (w = 0; w < APOLLO_SHARED_WORDS && shown < 12u; ++w)
        if (readable->words[w] != faithful_state->words[w]) {
            printf("  word %04o: readable %06o, faithful %06o (at the start %06o)\n", w,
                   (unsigned)readable->words[w], (unsigned)faithful_state->words[w],
                   (unsigned)start->words[w]);
            ++shown;
        }
    printf("  at the start: A %06o L %06o Q %06o EB %06o FB %06o MODE %05o FIXLOC %05o ADDRWD "
           "%05o MPTEMP %05o DVNORMCT %05o\n",
           (unsigned)start->words[K_A], (unsigned)start->words[K_L], (unsigned)start->words[K_Q],
           (unsigned)start->words[K_EB], (unsigned)start->words[K_FB],
           (unsigned)start->words[K_MODE], (unsigned)start->words[K_FIXLOC],
           (unsigned)start->words[K_ADDRWD], (unsigned)start->words[K_MPTEMP],
           (unsigned)start->words[K_DVNORMCT]);
}

struct counts {
    unsigned checks, both, faithful_only, both_refuse, not_modelled, faithful_no_end, failures;
};

/* One case of the entry at key. Returns what is wrong with it, or NULL. */
static const char *one_case(uint16_t key, agc_routine routine, const agc_native_program *program,
                            struct counts *counts, apollo_overlay_state *start,
                            apollo_overlay_state *readable, apollo_overlay_state *faithful_state)
{
    static mission_io io_start, io_readable, io_faithful;
    enum outcome r, f;
    const char *note;
    bool broken;

    broken = make_state(start, &io_start, key);
    *readable = *faithful_state = *start;
    io_readable = io_faithful = io_start;
    ++counts->checks;

    (void)interpreter_refused_state();
    r = run_readable(routine, program, readable, &io_readable);
    note = interpreter_refused_state();
    if (r == NO_END) return "the readable routine does not end";
    f = run_faithful(program, faithful_state, &io_faithful, r == RAN, readable->words[K_Z]);
    if (f == NO_END) {
        /* (A loop on a state no program makes: nothing to compare with.) */
        ++counts->faithful_no_end;
        return broken ? NULL : "the faithful routines do not end on a state a caller brings";
    }
    if (r == REFUSED) {
        if (memcmp(readable, start, sizeof *start) ||
            memcmp(&io_readable, &io_start, sizeof io_start))
            return "the readable routine refused and changed the state";
        if (!broken) return "the readable routine refused a state a caller brings";
        if (f == REFUSED) {
            ++counts->both_refuse;
            return NULL;
        }
        if (!note || (strcmp(note, NOT_MODELLED_SHIFT) && strcmp(note, NOT_MODELLED_RETURN)))
            return "the readable routine refused a state the faithful routines run";
        ++counts->not_modelled;
        return NULL;
    }
    if (f == REFUSED) {
        if (!broken) return "the faithful routines refuse a state a caller brings";
        ++counts->faithful_only;
        return NULL;
    }
    ++counts->both;
    if (memcmp(readable, faithful_state, sizeof *start) ||
        memcmp(&io_readable, &io_faithful, sizeof io_start))
        return "the two differ";
    return NULL;
}

int main(int argc, char **argv)
{
    static apollo_overlay_state start, readable, faithful_state;
    agc_native_program program;
    struct counts total;
    unsigned cases = argc > 2 ? (unsigned)strtoul(argv[2], 0, 0) : CASES_PER_ENTRY;
    uint16_t i;

    if (argc < 2 || !apollo_rope_load(ROPE, argv[1])) {
        fprintf(stderr, "usage: %s <%s rope> [cases per entry]\n", argv[0], MISSION);
        return 2;
    }
    memset(&program, 0, sizeof program);
    memset(&total, 0, sizeof total);
    signal(SIGVTALRM, on_alarm);

    for (i = 0; i < READABLE.count; ++i) {
        uint16_t key = READABLE.entries[i].physical;
        unsigned n, shown = 0;

        if (!faithful(key)) {
            printf("FAIL %s: no faithful entry at %s\n", MISSION, where(key));
            ++total.failures;
            continue;
        }
        find_sites(key);
        for (n = 0; n < cases; ++n) {
            const char *why = one_case(key, READABLE.entries[i].call, &program, &total, &start,
                                       &readable, &faithful_state);

            if (!why) continue;
            ++total.failures;
            if (++shown <= SHOWN_PER_ENTRY) show(why, key, n, &start, &readable, &faithful_state);
        }
    }
    printf("interpreter-side-cases %s: %u entries, %u checks, %u run by both and compared, %u "
           "refused by both, %u that only the faithful routines refuse, %u steps the readable "
           "code does not model, %u where the faithful routines do not end, %u failures\n",
           MISSION, (unsigned)READABLE.count, total.checks, total.both, total.both_refuse,
           total.faithful_only, total.not_modelled, total.faithful_no_end, total.failures);
    return total.failures != 0;
}
