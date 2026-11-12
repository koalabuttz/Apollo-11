/* SPDX-License-Identifier: MIT */
/* The readable Executive and WAITLIST against the faithful port's resident
 * kernel, case by case (make kernel-cases' last step).
 *
 * The corpus (c64-port/tests/test_resident_kernel.c, the step before this
 * one) holds both to the original program's digests on states a program
 * makes. This test runs every entry of the kernel's tables through both
 * on random states, most of them such states and some not: a core set's
 * PRIORITY at +0, a VAC area's use word negative, NEWJOB negative, a
 * counter below zero, RUPTAGN at +0, BUF as no scan leaves it. There the
 * routines follow the instructions wherever they lead (a CCS that lands
 * on a constant, which then runs as an instruction), and the two ports
 * must lead to the same place.
 *
 * Built once per mission, with the readable tables (src/binding/binding.c)
 * and the faithful port's (c64-port/src/lm_kernel.c, cm_kernel.c) under
 * the same names with faithful_ in front (the Makefile renames them).
 * Where the faithful routine runs, the readable one must too, with the
 * same erasable memory, registers, io record and hardware record, word
 * for word. Where the faithful routine refuses (leaves the state to the
 * original program), the readable one must refuse too: a case it ran alone
 * would be compared with nothing. Where the readable one refuses, it must
 * leave everything as it found it.
 *
 * An entry passes only if the faithful routine ran some of its cases and
 * some were run by both and compared.
 *
 * One kind of state no program makes, where the faithful routine departs
 * from the AGC and the readable one refuses, is not counted: a routine
 * that goes on to read fixed memory itself (VACFOUND's LXCH with a use
 * word that is not an erasable address; SETLOC's INDEX NEWJOB; CS PRIORITY
 * with a NEWJOB that takes it past erasable memory). The faithful routines
 * keep the low ten bits of every address and read erasable memory there
 * (c64-port/src/agc_image_ops.c, ai_ea); the AGC reads the rope. Their
 * count is held (DEPARTS): it is of this seed's cases, so a change to the
 * states made moves it, and it is then counted again and written here. */
#include <stdio.h>
#include <string.h>
#include "agc_transfer.h"
#include "resident_kernel.h"
#if LUMINARY099
#include "p66_bias.h"
#include "../src/agc/lm_names.inc"
typedef p66_bias_io mission_io;
#define MISSION "LM"
#define DEPARTS 4585u /* cases where the faithful routine departs from the AGC */
extern const agc_routine_table lm_kernel_routines, lm_kernel_job_routines, lm_kernel_task_routines,
    lm_kernel_idle_routines;
extern const agc_routine_table faithful_lm_kernel_routines, faithful_lm_kernel_job_routines,
    faithful_lm_kernel_task_routines, faithful_lm_kernel_idle_routines;
static const struct {
    const char *name;
    const agc_routine_table *readable, *faithful;
} tables[] = {
    {"lm_kernel_routines", &lm_kernel_routines, &faithful_lm_kernel_routines},
    {"lm_kernel_job_routines", &lm_kernel_job_routines, &faithful_lm_kernel_job_routines},
    {"lm_kernel_task_routines", &lm_kernel_task_routines, &faithful_lm_kernel_task_routines},
    {"lm_kernel_idle_routines", &lm_kernel_idle_routines, &faithful_lm_kernel_idle_routines}};
#else
#include "cm_native_image.h"
#include "../src/agc/cm_names.inc"
typedef cm_native_io mission_io;
#define MISSION "CM"
#define DEPARTS 4689u
extern const agc_routine_table cm_kernel_routines, cm_kernel_job_routines, cm_kernel_idle_routines;
extern const agc_routine_table faithful_cm_kernel_routines, faithful_cm_kernel_job_routines,
    faithful_cm_kernel_idle_routines;
static const struct {
    const char *name;
    const agc_routine_table *readable, *faithful;
} tables[] = {
    {"cm_kernel_routines", &cm_kernel_routines, &faithful_cm_kernel_routines},
    {"cm_kernel_job_routines", &cm_kernel_job_routines, &faithful_cm_kernel_job_routines},
    {"cm_kernel_idle_routines", &cm_kernel_idle_routines, &faithful_cm_kernel_idle_routines}};
#endif

/* tests/kernel_side_seams.c */
const char *kernel_refused_state(void);

#define CASES          20000u
#define CORE_SETS      (KW_NO_CORES + 1u)
#define CORE_SET_WORDS 12u
#define VAC_AREAS      5u
#define VAC_WORDS      054u
#define SITES          96u /* made-up call sites, each with the two words after its TC */

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

/* A 16-bit register: mostly a sign-extended word, sometimes an overflow. */
static uint16_t reg(void)
{
    return rnd() % 16u ? sign_extended(rnd()) : (uint16_t)(rnd() | rnd() << 1);
}

/* A word with each of CCS's four outcomes now and then, whatever a program
 * would keep there. */
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

/* A job's PRIORITY word: its priority in the high bits, and its VAC area's
 * address (or FAKEPRET, for a job with none) in the low nine. */
static uint16_t priority(void)
{
    uint16_t low = rnd() % 3u ? (uint16_t)(K_VAC1USE + 1u + VAC_WORDS * (rnd() % VAC_AREAS))
                              : (uint16_t)KW_FAKEPRET;
    if (rnd() % 16u == 0u) low = (uint16_t)(rnd() & 0777u);
    return (uint16_t)((1u + rnd() % 037u) << 9 | low);
}

static agc_site_operand operands[3u * SITES];

/* The editing register (CYR, SR, CYL or EDOP) that the state just made
 * points SETLOC's INDEX NEWJOB; CS PRIORITY at, or 0. */
static uint16_t setloc_register;

/* The made-up call sites: sorted physical addresses, three words each (the
 * word a TC's Q names and the two after), in fixed-fixed and in the banks. */
static void make_sites(void)
{
    unsigned i, j;
    for (i = 0; i < SITES; ++i) {
        /* Bank 01's and fixed-fixed's first, so that every entry has some. */
        unsigned bank = i < 16u ? 1u : i < 48u ? 2u + i % 2u : 4u + (i - 48u) % 040u;
        unsigned physical = bank * 1024u + 8u + 20u * ((i * 7u) % 50u) + (i % 5u);
        for (j = 0; j < 3u; ++j) {
            operands[3u * i + j].physical = (uint16_t)(physical + j);
            operands[3u * i + j].word = (agc_word)(rnd() & 077777u);
        }
    }
    /* Sorted by physical address, as a registry is. */
    for (i = 1; i < 3u * SITES; ++i) {
        agc_site_operand o = operands[i];
        for (j = i; j > 0 && operands[j - 1u].physical > o.physical; --j)
            operands[j] = operands[j - 1u];
        operands[j] = o;
    }
}

/* Q at a made-up site that the bank registers reach; for an entry in bank
 * 01, a site in bank 01 or in fixed-fixed. */
static uint16_t at_site(apollo_overlay_state *s, mission_io *io, bool bank01)
{
    for (;;) {
        unsigned physical = operands[3u * (rnd() % SITES)].physical, bank = physical / 1024u;
        if (bank == 2u || bank == 3u) return (uint16_t)(bank * 02000u + physical % 1024u);
        if (bank01 && bank != 1u) continue;
        if (bank >= 040u) {
            bank -= 010u;
            io->superbank = 0100u;
        } else if (bank >= 030u)
            io->superbank = 0u;
        s->words[4] = (uint16_t)(bank << 10);
        return (uint16_t)(02000u + physical % 1024u);
    }
}

static void state(apollo_overlay_state *s, mission_io *io, agc_hardware *hw, uint16_t key)
{
    unsigned i, k;
    bool bank01 = key < 2048u;
    bool broken = rnd() % 4u == 0u; /* a state no program makes, somewhere */

    setloc_register = 0u;
    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = rnd() % 4u ? reg() : (uint16_t)(rnd() % 3u ? priority() : rnd() % 02000u);
    s->words[1] = reg();
    s->words[3] = (uint16_t)((rnd() % 8u) << 8);
    s->words[4] = bank01 ? 02000u : (uint16_t)((rnd() % 040u) << 10);
    io->superbank = (uint16_t)(rnd() % 2u ? 0100u : 0u);
    io->interrupt_enabled = rnd() % 2u;
    hw->in_interrupt = rnd() % 2u;
    hw->channel11 = (agc_word)(rnd() & 077777u);
#if COMANCHE055
    io->channel11 = (agc_word)(rnd() & 077777u);
#endif
    s->words[2] = rnd() % 8u ? at_site(s, io, bank01) : reg();
    s->words[5] =
        key >= 2048u ? (uint16_t)(04000u + key - 2048u) : (uint16_t)(02000u + key - 1024u);
    s->words[6] = (uint16_t)(s->words[4] | (s->words[3] >> 8));

    /* The core sets: free (-0), a job (positive), a sleeping job
     * (negative); LOC a fixed address, negative for an interpretive job. */
    for (k = 0; k < CORE_SETS; ++k) {
        uint16_t *set = &s->words[K_MPAC + CORE_SET_WORDS * k];
        uint16_t p = priority();
        switch (rnd() % 8u) {
        case 0:
        case 1:
        case 2: set[K_PRIORITY - K_MPAC] = 077777u; break;
        case 3: set[K_PRIORITY - K_MPAC] = complement(p); break;
        default: set[K_PRIORITY - K_MPAC] = p; break;
        }
        if (broken && rnd() % 8u == 0u) set[K_PRIORITY - K_MPAC] = any_sign();
        set[K_LOC - K_MPAC] = (uint16_t)(02000u + rnd() % 06000u);
        if (rnd() % 4u == 0u) set[K_LOC - K_MPAC] = complement(set[K_LOC - K_MPAC]);
        set[K_BANKSET - K_MPAC] = (uint16_t)((rnd() % 040u) << 10 | (rnd() % 8u));
    }
    /* The VAC areas' use words: their own address (free) or +0 (used). */
    for (k = 0; k < VAC_AREAS; ++k) {
        uint16_t use = (uint16_t)(K_VAC1USE + VAC_WORDS * k);
        s->words[use] = rnd() % 2u ? use : 0u;
        if (broken && rnd() % 8u == 0u) s->words[use] = any_sign();
    }
    /* NEWJOB: -0 (idle), +0 (the job in core set 0 runs on), or the core
     * set to switch to. */
    switch (rnd() % 4u) {
    case 0: s->words[K_NEWJOB] = 077777u; break;
    case 1: s->words[K_NEWJOB] = 0u; break;
    default: s->words[K_NEWJOB] = (uint16_t)(CORE_SET_WORDS * (rnd() % CORE_SETS)); break;
    }
    if (broken && rnd() % 4u == 0u) s->words[K_NEWJOB] = any_sign();
    if (rnd() % 2u) s->words[K_NEWPRIO] = priority();
    if (rnd() % 2u) s->words[K_LOCCTR] = (uint16_t)(CORE_SET_WORDS * (rnd() % CORE_SETS));
    if (broken && rnd() % 4u == 0u) s->words[K_LOCCTR] = any_sign();
    if (rnd() % 2u) s->words[K_EXECTEM2] = (uint16_t)(rnd() % (CORE_SETS + 1u));
    if (broken && rnd() % 4u == 0u) s->words[K_EXECTEM2] = any_sign();
    /* BUF and BUF +1 as the scan for the next job finds or leaves them. */
    switch (rnd() % 4u) {
    case 0: s->words[K_BUF] = 0u; break;
    case 1: s->words[K_BUF] = (uint16_t)(K_EJSCAN + 1u + 4u * (rnd() % (CORE_SETS - 1u))); break;
    default: break;
    }
    switch (rnd() % 4u) {
    case 0: s->words[K_BUF + 1u] = 077777u; break;
    case 1: s->words[K_BUF + 1u] = complement(priority()); break;
    default: break;
    }
    if (broken && rnd() % 4u == 0u) s->words[K_BUF] = any_sign();
    if (broken && rnd() % 4u == 0u) s->words[K_BUF + 1u] = any_sign();

    /* The waitlist: RUPTAGN +1 or -0; the gaps of LST1 small and negative,
     * with -1/2 after the last task; LST2 the tasks' 2CADRs. */
    s->words[K_RUPTAGN] = rnd() % 2u ? 1u : 077777u;
    if (broken && rnd() % 4u == 0u) s->words[K_RUPTAGN] = any_sign();
    k = rnd() % 9u; /* tasks waiting */
    for (i = 0; i < 8u; ++i) {
        s->words[K_LST1 + i] = i < k ? complement((uint16_t)(rnd() % 01000u)) : 057777u;
        if (rnd() % 16u == 0u) s->words[K_LST1 + i] = any_sign();
    }
    for (i = 0; i < 9u; ++i) {
        s->words[K_LST2 + 2u * i] = (uint16_t)(02000u + rnd() % 06000u);
        s->words[K_LST2 + 2u * i + 1u] = (uint16_t)((rnd() % 040u) << 10 | (rnd() % 8u));
    }
    if (rnd() % 2u) s->words[K_TIME3] = (uint16_t)(037777u - rnd() % 01000u);
    if (rnd() % 4u == 0u) s->words[0] = (uint16_t)(1u + rnd() % 01000u); /* a delay */
    /* LONGTIME: a double-precision time, often near its last BIT14. */
    switch (rnd() % 4u) {
    case 0:
        s->words[K_LONGTIME] = 0u;
        s->words[K_LONGTIME + 1u] = (uint16_t)(rnd() % 040000u);
        break;
    case 1:
        s->words[K_LONGTIME] = (uint16_t)(rnd() % 4u);
        s->words[K_LONGTIME + 1u] = (uint16_t)(rnd() % 040000u);
        break;
    case 2:
        s->words[K_LONGTIME] = any_sign();
        s->words[K_LONGTIME + 1u] = any_sign();
        break;
    default: break;
    }
    if (rnd() % 64u == 0u) s->words[7] = 1u;

    /* Three states made on purpose, since no random one comes to them
     * (or only by luck). */
    if (broken && rnd() % 8u == 0u) {
        /* A new priority (PRIOCHNG's A) that sums with the job's VAC
         * address to -0, so that BUF +1 starts the scan at +0; and a core
         * set whose PRIORITY is 1, which EJ1 then finds equal to it. */
        s->words[0] = sign_extended(complement((uint16_t)(s->words[K_PRIORITY] & 0777u)));
        s->words[K_PRIORITY + CORE_SET_WORDS * (1u + rnd() % KW_NO_CORES)] = 1u;
    } else if (broken && rnd() % 8u == 0u) {
        /* Gaps of +0 and a new task a few counts after the first: the
         * walk down the gaps counts A down to +0 and carries +0 on to the
         * end of the list. */
        uint16_t delay = (uint16_t)(010u + rnd() % 0400u);
        for (i = 0; i < 8u; ++i) s->words[K_LST1 + i] = 0u;
        s->words[0] = delay;
        s->words[K_TIME3] = (uint16_t)(037777u - delay + 1u + rnd() % 8u);
    } else if (broken && rnd() % 8u == 0u) {
        /* A NEWJOB that takes SETLOC's INDEX NEWJOB; CS PRIORITY down to an
         * editing register (NEWJOB is PRIORITY's address less the
         * register's, negative): the CS reads the register, which edits
         * its word again. PRIORITY as a variable of C would not. */
        setloc_register = (uint16_t)(020u + rnd() % 4u);
        s->words[K_NEWJOB] = complement((uint16_t)(K_PRIORITY - setloc_register));
    }
}

/* An entry's place, as the listing shows it. */
static const char *where(uint16_t key)
{
    static char text[24];
    if (key >= 2048u)
        sprintf(text, "%05o", (unsigned)(04000u + key - 2048u));
    else
        sprintf(text, "%02o,%04o", (unsigned)(key / 1024u), (unsigned)(02000u + key % 1024u));
    return text;
}

static agc_routine find(const agc_routine_table *t, uint16_t key)
{
    uint16_t i;
    for (i = 0; i < t->count; ++i)
        if (t->entries[i].physical == key) return t->entries[i].call;
    return 0;
}

int main(void)
{
    agc_native_program program;
    agc_hardware hw_a, hw_b, hw_start;
    unsigned t, i, checks = 0, both = 0, departs = 0, edited = 0, failures = 0;

    make_sites();
    memset(&program, 0, sizeof program);
    program.operands = operands;
    program.operand_count = 3u * SITES;

    for (t = 0; t < sizeof tables / sizeof tables[0]; ++t) {
        if (tables[t].readable->count != tables[t].faithful->count) {
            printf("FAIL %s: %u entries, faithful %u\n", tables[t].name,
                   (unsigned)tables[t].readable->count, (unsigned)tables[t].faithful->count);
            return 1;
        }
        for (i = 0; i < tables[t].faithful->count; ++i) {
            uint16_t key = tables[t].faithful->entries[i].physical;
            agc_routine r = find(tables[t].readable, key), f = tables[t].faithful->entries[i].call;
            unsigned n, ran = 0, refused = 0, compared = 0, bad = 0;
            if (!r) {
                printf("FAIL %s: no readable entry at %s\n", tables[t].name, where(key));
                ++failures;
                continue;
            }
            for (n = 0; n < CASES; ++n) {
                apollo_overlay_state a, b, start;
                mission_io ia, ib, istart;
                enum exec_status sa, sb;
                const char *why = 0, *refusal;
                state(&a, &ia, &hw_a, key);
                program.hardware = rnd() % 64u ? &hw_a : 0;
                b = a;
                ib = ia;
                hw_b = hw_start = hw_a;
                start = a;
                istart = ia;
                sa = f(&program, &a, &ia);
                if (program.hardware) program.hardware = &hw_b;
                sb = r(&program, &b, &ib);
                refusal = kernel_refused_state();
                ++checks;
                if (sa == EXEC_OK) ++ran;
                if (sb != EXEC_OK) ++refused;
                if (sb != EXEC_OK) {
                    if (memcmp(&b, &start, sizeof b) || memcmp(&ib, &istart, sizeof ib) ||
                        memcmp(&hw_b, &hw_start, sizeof hw_b))
                        why = "readable refusal changed the state";
                    else if (sa == EXEC_OK && refusal && strstr(refusal, "reading fixed memory"))
                        ++departs;
                    else if (sa == EXEC_OK)
                        why = "readable refused, faithful ran";
                } else if (sa == EXEC_OK) {
                    ++compared;
                    if (memcmp(&a, &b, sizeof a) || memcmp(&ia, &ib, sizeof ia) ||
                        memcmp(&hw_a, &hw_b, sizeof hw_a))
                        why = "different";
                    /* SETLOC's CS read the editing register NEWJOB named. */
                    if (setloc_register && a.words[setloc_register] != start.words[setloc_register])
                        ++edited;
                } else
                    why = "readable ran, faithful refused";
                if (why && ++bad <= 3) {
                    unsigned w;
                    printf("FAIL %s %s case %u: %s (status %d, faithful %d)\n", tables[t].name,
                           where(key), n, why, (int)sb, (int)sa);
                    for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                        if (a.words[w] != b.words[w])
                            printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, b.words[w],
                                   a.words[w], start.words[w]);
                    if (memcmp(&ia, &ib, sizeof ia)) printf("  io record differs\n");
                    if (memcmp(&hw_a, &hw_b, sizeof hw_a)) printf("  hardware differs\n");
                    printf(
                        "  start A %06o L %06o Q %06o EB %06o FB %06o SB %03o IE %o NEWJOB %05o\n",
                        start.words[0], start.words[1], start.words[2], start.words[3],
                        start.words[4], (unsigned)istart.superbank,
                        (unsigned)istart.interrupt_enabled, start.words[K_NEWJOB]);
                }
            }
            failures += bad;
            both += compared;
            printf("%s kernel entry %s: %u cases, %u run by the faithful routine, %u by both and "
                   "compared, %u refused by the readable one, %u failures\n",
                   MISSION, where(key), CASES, ran, compared, refused, bad);
            if (ran == 0u || compared == 0u) {
                printf("FAIL %s %s: %s\n", tables[t].name, where(key),
                       ran == 0u ? "the faithful routine never ran" : "no case was run by both");
                ++failures;
            }
        }
    }
    if (edited == 0u) {
        printf("FAIL: no case brought SETLOC's INDEX NEWJOB; CS PRIORITY to an editing register\n");
        ++failures;
    }
    if (departs != DEPARTS) {
        printf("FAIL: %u cases where the faithful routine departs from the AGC (held: %u)\n",
               departs, DEPARTS);
        ++failures;
    }
    printf("kernel-side-cases %s: %u checks, %u run by both and compared (%u of them with SETLOC's "
           "CS at an editing register), %u where the faithful routine departs from the AGC, %u "
           "failures\n",
           MISSION, checks, both, edited, departs, failures);
    return failures != 0;
}
