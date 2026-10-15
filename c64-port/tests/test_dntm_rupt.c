/* SPDX-License-Identifier: MIT */
/* dntm-rupt: the flight store's DOWNRUPT ends (src/lm_dntm_rupt.c,
 * src/cm_dntm_rupt.c) against the routines they copy. Every downrupt of the
 * down-telemetry cases (tests/lm_downtelemetry_cases.c and the CM's) runs
 * from the DOWNRUPT vector to the resumed Z twice: through the lead-in, DNTM
 * and RESUME tables the resident's wrappers run (src/rupt_lead_ins.c,
 * src/lm_t6rupt.c), and through the DNTM tables with the copies, as one
 * flight module run takes it. Image, io record, hardware, status and steps
 * must agree. Then each copy runs beside its original on pseudo-random
 * states, valid and not: same outcome, same state. */
#include <stdio.h>
#include <string.h>
#include "dntm_rupt_routines.h"
#include "rupt_lead_ins.h"
#include "lm_t6rupt_routines.h"
#include "lm_downtelemetry_cases.h"
#include "cm_downtelemetry_cases.h"
static unsigned checks, failures;
static void check(int ok, const char *what, unsigned n)
{
    ++checks;
    if (!ok) {
        ++failures;
        if (failures < 20u) fprintf(stderr, "FAIL %s %u\n", what, n);
    }
}
static uint32_t seed = 0x2d4e;
static uint32_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return seed >> 8;
}
/* The rope as resume_exact reads it: a fixed word per physical address. */
static agc_word rope(uint16_t physical)
{
    return (agc_word)((physical * 40503u + 0x1234u) & 077777u);
}
static bool fixed_data(const agc_native_program *p, uint16_t physical, agc_word *w)
{
    (void)p;
    *w = rope(physical);
    return true;
}
static agc_hardware hw;
/* The vector's entry state from a case's DODOWNTM entry: BB the interrupted
 * BB (A there), A,L whatever the interrupted code held (ARUPT,LRUPT get
 * them), ZRUPT a fixed-fixed point and BRUPT mostly the word there. */
static void vector_state(apollo_overlay_state *s, agc_word *superbank)
{
    agc_word bb = agc_correct(s->words[0]), z;
    s->words[0] = agc_extend((agc_word)(rnd() & 077777u));
    s->words[1] = agc_extend((agc_word)(rnd() & 077777u));
    s->words[6] = bb;
    s->words[4] = (agc_word)(bb & 076000u);
    s->words[3] = (agc_word)((bb & 7u) << 8);
    *superbank = (agc_word)((rnd() & 1u) ? 0100u : 0u);
    z = (agc_word)(06000u + rnd() % 02000u);
    s->words[015] = (agc_word)(z + 1u);
    s->words[017] = (rnd() & 3u) ? rope(agc_fixed_physical(z, 0, 0)) : (agc_word)(rnd() & 077777u);
    s->words[5] = 04040u;
    hw.in_interrupt = 1;
}
static enum exec_status run(const agc_routine_table *tables, uint8_t count, apollo_overlay_state *s,
                            void *io, const agc_word *superbank, uint16_t *steps)
{
    agc_native_program p = {tables, count, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, fixed_data, 0, &hw};
    return agc_transfer_run(&p, s, io, superbank, AGC_TRANSFER_NONE, 16, steps);
}
#define RUN_PAIR(T_WRAP, T_FUSED, IO, SB)                                                          \
    do {                                                                                           \
        static apollo_overlay_state a, b;                                                          \
        IO ia, ib;                                                                                 \
        agc_hardware ha, hb;                                                                       \
        enum exec_status sa, sb;                                                                   \
        uint16_t na = 0, nb = 0;                                                                   \
        a = s;                                                                                     \
        ia = io;                                                                                   \
        b = s;                                                                                     \
        ib = io;                                                                                   \
        hw = h0;                                                                                   \
        sa = run(T_WRAP, 3, &a, &ia, &ia.SB, &na);                                                 \
        ha = hw;                                                                                   \
        hw = h0;                                                                                   \
        sb = run(T_FUSED, 2, &b, &ib, &ib.SB, &nb);                                                \
        hb = hw;                                                                                   \
        check(na == 5u &&                                                                          \
                  (sa == EXEC_UNMAPPED || (sa == EXEC_OUT_OF_SCOPE && ha.in_interrupt == 1u)),     \
              "wrapper chain reaches RESUME", n);                                                  \
        ++ended[sa == EXEC_UNMAPPED ? 0 : 1];                                                      \
        check(sa == sb && na == nb && !memcmp(&a, &b, sizeof a) && !memcmp(&ia, &ib, sizeof ia) && \
                  !memcmp(&ha, &hb, sizeof ha),                                                    \
              "fused chain", n);                                                                   \
        s = a;                                                                                     \
        io = ia;                                                                                   \
        ++runs;                                                                                    \
    } while (0)
static unsigned runs, ended[2];
static void lm_chains(void)
{
    agc_routine_table w[3], f[2];
    uint16_t n, k;
    w[0] = lm_lead_in_routines;
    w[1] = lm_downtelemetry_routines;
    w[2] = lm_t6rupt_routines;
    f[0] = lm_downtelemetry_routines;
    f[1] = lm_dntm_rupt_routines;
    for (n = 0; n < LM_DNTM_CASES; ++n) {
        static apollo_overlay_state s;
        lm_dntm_io io;
        agc_hardware h0;
        lm_dntm_begin(&s, &io, n);
        for (k = 0; k < lm_dntm_steps(n); ++k) {
            lm_dntm_entry(&s, &io, n, k);
            vector_state(&s, &io.control.superbank);
            h0 = hw;
            RUN_PAIR(w, f, lm_dntm_io, control.superbank);
        }
    }
}
static void cm_chains(void)
{
    agc_routine_table w[3], f[2];
    uint16_t n, k;
    w[0] = cm_lead_in_routines;
    w[1] = cm_downtelemetry_routines;
    w[2] = cm_lead_in_routines;
    f[0] = cm_downtelemetry_routines;
    f[1] = cm_dntm_rupt_routines;
    for (n = 0; n < CM_DNTM_CASES; ++n) {
        static apollo_overlay_state s;
        cm_dntm_io io;
        agc_hardware h0;
        cm_dntm_begin(&s, &io, n);
        for (k = 0; k < cm_dntm_steps(n); ++k) {
            cm_dntm_entry(&s, &io, n, k);
            vector_state(&s, &io.io.superbank);
            h0 = hw;
            RUN_PAIR(w, f, cm_dntm_io, io.superbank);
        }
    }
}
static agc_routine find(const agc_routine_table *t, agc_word z)
{
    uint16_t i, ph = (uint16_t)(2048u + (z - 04000u));
    for (i = 0; i < t->count; ++i)
        if (t->entries[i].physical == ph) return t->entries[i].call;
    return 0;
}
/* One copy against its original from one pseudo-random state. */
static unsigned fuzzed[2][3];
static void fuzz(int cm, agc_word z)
{
    static apollo_overlay_state s, a, b;
    agc_hardware h0, ha, hb;
    enum exec_status sa, sb;
    unsigned i;
    lm_dntm_io la, lb;
    cm_dntm_io ca, cb;
    agc_native_program p = {0, 0, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, fixed_data, 0, &hw};
    const agc_routine_table *orig = cm            ? &cm_lead_in_routines
                                    : z == 04040u ? &lm_lead_in_routines
                                                  : &lm_t6rupt_routines;
    agc_routine o = find(orig, z),
                c = find(cm ? &cm_dntm_rupt_routines : &lm_dntm_rupt_routines, z);
    for (i = 0; i < 2048u; ++i) s.words[i] = (agc_word)(rnd() & 077777u);
    s.words[0] = agc_extend(s.words[0]);
    s.words[1] = agc_extend(s.words[1]);
    s.words[7] = 0;
    s.words[5] = z;
    if (rnd() & 1u) s.words[016] &= 076167u;
    if (rnd() & 1u) s.words[015] = (agc_word)(02000u + rnd() % 06000u);
    if (rnd() & 1u) {
        agc_word bb = s.words[016], zr = (agc_word)(s.words[015] - 1u);
        s.words[017] = zr >= 02000u
                           ? rope(agc_fixed_physical(zr, (agc_word)(bb & 076000u),
                                                     (agc_word)((rnd() & 1u) ? 0100u : 0u)))
                           : s.words[017];
    }
    memset(&h0, 0, sizeof h0);
    h0.in_interrupt = (uint16_t)((rnd() & 7u) ? 1u : rnd() % 3u);
    memset(&la, 0, sizeof la);
    la.control.superbank = (agc_word)((rnd() & 1u) ? 0100u : 0u);
    la.control.interrupt_enabled = (agc_word)(rnd() & 1u);
    memset(&ca, 0, sizeof ca);
    ca.io.superbank = la.control.superbank;
    ca.io.interrupt_enabled = la.control.interrupt_enabled;
    lb = la;
    cb = ca;
    a = s;
    b = s;
    hw = h0;
    sa = o(&p, &a, cm ? (void *)&ca : (void *)&la);
    ha = hw;
    hw = h0;
    sb = c(&p, &b, cm ? (void *)&cb : (void *)&lb);
    hb = hw;
    check(o && c && sa == sb && !memcmp(&a, &b, sizeof a) && !memcmp(&la, &lb, sizeof la) &&
              !memcmp(&ca, &cb, sizeof ca) && !memcmp(&ha, &hb, sizeof ha),
          cm ? "CM copy" : "LM copy", z);
    ++fuzzed[cm][sa == EXEC_OK ? 0 : sa == EXEC_OUT_OF_SCOPE ? 1 : 2];
}
int main(void)
{
    unsigned i;
    lm_chains();
    i = runs;
    cm_chains();
    printf("dntm-rupt: %u LM and %u CM downrupts from the vector (%u resumed, %u refused at "
           "RESUME: BRUPT not the word at ZRUPT-1), the same through the copies",
           i, runs - i, ended[0], ended[1]);
    check(ended[0] > 1000u && ended[1] > 1000u, "both RESUME outcomes", 0);
    for (i = 0; i < 20000u; ++i) {
        fuzz(0, 04040u);
        fuzz(0, LM_RESUME);
        fuzz(1, 04040u);
        fuzz(1, 05333u);
    }
    printf("; fuzz LM %u ran %u refused, CM %u ran %u refused, same; %u checks, %u failures\n",
           fuzzed[0][0], fuzzed[0][1], fuzzed[1][0], fuzzed[1][1], checks, failures);
    check(fuzzed[0][0] > 1000u && fuzzed[0][1] > 1000u && fuzzed[1][0] > 1000u &&
              fuzzed[1][1] > 1000u && !fuzzed[0][2] && !fuzzed[1][2],
          "both outcomes fuzzed", 0);
    return failures != 0u;
}
