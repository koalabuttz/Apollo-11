/* SPDX-License-Identifier: MIT */
/* The resident's idle memo (platform/c64/resident_flight.c, RK_IDLE_MEMO)
 * replays the idle loop's two steps from what they did last time: the
 * self-check pass from SELFRET's return (CCS SMODE, CHECKNJ, POSTJUMP to
 * ADVAN) and ADVAN back to the pass. It is exact only if, with NEWJOB -0 and
 * SMODE +0 (the memo's gate) and FB and Z at the step's start, each step
 * reads no word but its key (Z, L, EB, BB, SELFRET and the superbank) and
 * writes no word but its result (the key and A, Q, FB). This test runs
 * both steps, with the routines the flight store runs them with, on random
 * images and io records: every other erasable word, io field and hardware
 * field stays as it was, and two runs that share the key but differ in
 * everything else end on the same result. Build with -DRK_LM=1 or
 * -DRK_CM=1 (tools/mk/idle-memo.mk). */
#include <stdio.h>
#include <string.h>
#include "resident_kernel.h"
#include "self_check.h"
#if RK_LM
#include "lm_interbank_dap.h"
#include "lm_exec_routines.h"
#include "p66_bias.h"
typedef p66_bias_io memo_io;
#define PASS_Z 03304u
#define ADVAN_Z 03214u
#define PASS_TABLE lm_self_check_idle_routines
#define POSTJUMP_TABLE lm_interbank_postjump_hot_routines
#define IDLE_SITES lm_self_check_idle_sites
#define KERNEL_IDLE lm_kernel_idle_routines
#define EXEC_SITES lm_exec_site_table
#define MISSION "LM"
#else
#include "cm_interbank_postjump.h"
#include "cm_exec_routines.h"
#include "cm_native_image.h"
typedef cm_native_io memo_io;
#define PASS_Z 03306u
#define ADVAN_Z 03231u
#define PASS_TABLE cm_self_check_idle_routines
#define POSTJUMP_TABLE cm_interbank_postjump_hot_routines
#define IDLE_SITES cm_self_check_idle_sites
#define KERNEL_IDLE cm_kernel_idle_routines
#define EXEC_SITES cm_exec_site_table
#define MISSION "CM"
#endif
#define SELFRET 01361u
#define NEWJOB 067u
#define SMODE 01362u
#define CASES 4000u
/* The words of the result, in the order of rk_memo_at. */
static const uint16_t result[8] = {5, 1, 3, 6, SELFRET, 0, 2, 4};

static unsigned checks, failures, seams;
/* The resident's job-switch seams (RK_SEAMS): an idle step calls neither. */
void rk_seams_exchange(agc_word newjob)
{
    (void)newjob;
    ++seams;
}
void rk_seams_release(void) { ++seams; }
static void check(int ok, const char *what, unsigned n)
{
    ++checks;
    if (!ok && ++failures < 20u) fprintf(stderr, "FAIL " MISSION " %s case %u\n", what, n);
}
static uint32_t seed = 0x1d1e5eedu;
static uint16_t rnd(void)
{
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return (uint16_t)(seed >> 8);
}
static int is_result(uint16_t a)
{
    unsigned i;
    for (i = 0; i < sizeof result / sizeof result[0]; ++i)
        if (result[i] == a) return 1;
    return 0;
}
/* A random image and io record at the start of step (0 the pass, 1 ADVAN),
 * with the gate and the step's FB and Z. */
static void start(apollo_overlay_state *s, memo_io *io, agc_hardware *hw, unsigned step)
{
    uint16_t a, eb = (uint16_t)(rnd() & 7u) << 8;
    uint8_t *b = (uint8_t *)io;
    for (a = 0; a < APOLLO_SHARED_WORDS; ++a) s->words[a] = rnd() & 077777u;
    for (a = 0; a < sizeof *io; ++a) b[a] = (uint8_t)rnd();
    b = (uint8_t *)hw;
    for (a = 0; a < sizeof *hw; ++a) b[a] = (uint8_t)rnd();
    hw->in_interrupt = 0;
    hw->channel11 &= 077777u;
    io->superbank = (agc_word)(0100u | (rnd() & 060u));
    io->interrupt_enabled = rnd() & 1u;
    s->words[NEWJOB] = 077777u;
    s->words[SMODE] = 0;
    s->words[7] = 0;
    s->words[4] = step ? 02000u : 066000u;
    s->words[3] = eb;
    s->words[6] = (agc_word)(s->words[4] | eb >> 8);
    s->words[5] = step ? ADVAN_Z : PASS_Z;
}
static enum exec_status run(apollo_overlay_state *s, memo_io *io, agc_hardware *hw, unsigned step)
{
    agc_routine_table pass[2] = {PASS_TABLE, POSTJUMP_TABLE}, advan[1] = {KERNEL_IDLE};
    agc_native_program p = {0, 0, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, 0, 0, 0};
    agc_word superbank = io->superbank;
    uint16_t steps, stop;
    p.hardware = hw;
    if (step) {
        p.basic = advan;
        p.basic_count = 1;
        p.operands = EXEC_SITES(&p.operand_count);
        stop = AGC_TRANSFER_NONE; /* the resident kernel runs one routine a step */
    } else {
        p.basic = pass;
        p.basic_count = 2;
        p.operands = IDLE_SITES;
        p.operand_count = 1;
        stop = (uint16_t)(1024u + ADVAN_Z - 02000u);
    }
    {
        enum exec_status st = agc_transfer_run(&p, s, io, &superbank, stop, step ? 1u : 8u, &steps);
        return st == EXEC_OUT_OF_SCOPE && step && steps == 1u ? EXEC_OK : st;
    }
}
int main(void)
{
    static apollo_overlay_state s0, s1, t0, t1;
    memo_io io0, io1, jo0, jo1;
    agc_hardware hw0, hw1, h0, h1;
    unsigned n, step;
    uint16_t a;
    for (step = 0; step < 2u; ++step)
        for (n = 0; n < CASES; ++n) {
            enum exec_status st0, st1;
            start(&s0, &io0, &hw0, step);
            start(&s1, &io1, &hw1, step);
            /* The second run shares only the key. */
            /* Half the ADVAN cases return to the pass, as the idle loop does. */
            if (step && (n & 1u)) s0.words[SELFRET] = PASS_Z;
            s1.words[1] = s0.words[1];
            s1.words[3] = s0.words[3];
            s1.words[6] = s0.words[6];
            s1.words[SELFRET] = s0.words[SELFRET];
            io1.superbank = io0.superbank;
            memcpy(&t0, &s0, sizeof s0);
            memcpy(&t1, &s1, sizeof s1);
            memcpy(&jo0, &io0, sizeof io0);
            memcpy(&jo1, &io1, sizeof io1);
            memcpy(&h0, &hw0, sizeof hw0);
            memcpy(&h1, &hw1, sizeof hw1);
            st0 = run(&s0, &io0, &hw0, step);
            st1 = run(&s1, &io1, &hw1, step);
            check(st0 == EXEC_OK && st1 == EXEC_OK, step ? "ADVAN status" : "pass status", n);
            if (!step) check(s0.words[5] == ADVAN_Z && s0.words[4] == 02000u, "pass ends at ADVAN", n);
            else if (n & 1u) check(s0.words[5] == PASS_Z && s0.words[4] == 066000u, "ADVAN ends at the pass", n);
            for (a = 0; a < APOLLO_SHARED_WORDS; ++a)
                if (!is_result(a)) {
                    check(s0.words[a] == t0.words[a] && s1.words[a] == t1.words[a], "word outside the result", a);
                } else {
                    check(s0.words[a] == s1.words[a], step ? "ADVAN result word differs with the same key" : "pass result word differs with the same key", a);
                }
            jo0.superbank = io0.superbank;
            jo1.superbank = io1.superbank;
            check(!memcmp(&io0, &jo0, sizeof io0) && !memcmp(&io1, &jo1, sizeof io1), "io beyond the superbank", n);
            check(io0.superbank == io1.superbank, "superbank differs with the same key", n);
            check(!seams, "a job-switch seam", n);
            check(!memcmp(&hw0, &h0, sizeof hw0) && !memcmp(&hw1, &h1, sizeof hw1), "hardware", n);
        }
    printf("idle memo " MISSION ": %u checks, %u failures\n", checks, failures);
    return failures != 0;
}
