/* SPDX-License-Identifier: MIT */
/* The readable machine's instructions (src/agc/agc.h) against the faithful
 * port's (c64-port/src/agc_image_ops.c, checked against yaAGC) on random
 * machine states: every instruction on every kind of erasable operand (the
 * central registers, the editing registers, the switched bank), the
 * constant forms, and DV against the port's divide unit on random and edge
 * dividends. Each run must leave the same memory and the same outcome.
 *
 * The faithful helpers take erasable addresses only (they wrap anything
 * above 01777), so fixed operands are the constant forms here; the
 * interpreter's cases read fixed words through the readable machine. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_image_ops.h"
#include "agc_hardware_divide.h"
#include "../src/agc/agc.h"

static uint64_t seed = 0x9e3779b97f4a7c15ull;
static unsigned rnd(unsigned n)
{
    seed ^= seed << 13;
    seed ^= seed >> 7;
    seed ^= seed << 17;
    return (unsigned)(seed % n);
}

/* A word with the values that matter often: zeros, ones, extremes. */
static uint16_t word15(void)
{
    static const uint16_t edge[] = {0, 077777, 1, 077776, 037777, 040000, 020000, 057777};
    return rnd(4) ? (uint16_t)rnd(0100000) : edge[rnd(8)];
}
static uint16_t register16(void)
{
    static const uint16_t edge[] = {0, 0177777, 1, 0177776, 037777, 0140000, 040000, 0137777};
    return rnd(3) ? (uint16_t)rnd(0200000) : edge[rnd(8)];
}

static apollo_overlay_state before, faithful, readable;
static uint16_t superbank, interrupts, dsalmout, in_interrupt;

static void random_state(void)
{
    unsigned i, eb = rnd(8), fb = rnd(040);
    for (i = 0; i < 2048u; ++i) before.words[i] = word15();
    before.words[0] = register16();
    before.words[1] = sign_extend(overflow_corrected(register16())); /* L is always corrected */
    before.words[2] = rnd(2) ? register16() : word15();
    before.words[3] = (uint16_t)(eb << 8);
    before.words[4] = (uint16_t)(fb << 10);
    before.words[6] = (uint16_t)((fb << 10) | eb);
    before.words[7] = 0;
    before.words[023] &= 0177u; /* EDOP holds 7 bits */
}

/* An erasable address of every kind. */
static unsigned address(void)
{
    switch (rnd(6)) {
    case 0: return rnd(8);
    case 1: return 020u + rnd(4);
    case 2: return 01400u + rnd(0400);
    case 3: return 01776u + rnd(2);
    default: return rnd(02000);
    }
}

static unsigned failures, checks;

static void compare(const char *name, unsigned k, int faithful_result, int readable_result)
{
    unsigned i;
    ++checks;
    if (!memcmp(&faithful, &readable, sizeof faithful) && faithful_result == readable_result) return;
    if (++failures > 20) return;
    printf("FAIL %s %04o: result %d/%d", name, k, faithful_result, readable_result);
    for (i = 0; i < 2048u; ++i)
        if (faithful.words[i] != readable.words[i])
            printf(" [%04o] %06o/%06o (was %06o)", i, faithful.words[i], readable.words[i],
                   before.words[i]);
    printf("\n");
}

static void bind(void)
{
    readable = before;
    faithful = before;
    agc.memory = readable.words;
    agc.superbank = &superbank;
    agc.interrupts = &interrupts;
    agc.dsalmout = &dsalmout;
    agc.in_interrupt = &in_interrupt;
    agc.fixed_memory = NULL;
}

/* Each instruction: its faithful helper and the readable one. */
#define ONE(name, faithful_call, readable_call)                                                    \
    do {                                                                                           \
        int f_ = 0, r_ = 0;                                                                        \
        bind();                                                                                    \
        f_ = (faithful_call);                                                                      \
        r_ = (readable_call);                                                                      \
        compare(name, k, f_, r_);                                                                  \
    } while (0)
#define VOID(call) ((call), 0)

static int sign_code(enum sign s)
{
    return (int)s; /* POSITIVE 0, +0 1, NEGATIVE 2, -0 3: the faithful codes */
}

static void one_state(void)
{
    unsigned k = address();
    uint16_t c = word15();
    reg16 divisor = rnd(2) ? sign_extend(word15()) : register16();
    agc_word pair[2];

    ONE("CA", VOID(ai_ca(&faithful, k)), VOID(ca(k)));
    ONE("CS", VOID(ai_cs(&faithful, k)), VOID(cs(k)));
    ONE("AD", VOID(ai_ad(&faithful, k)), VOID(ad_at(k)));
    ONE("SU", VOID(ai_su(&faithful, k)), VOID(su_at(k)));
    ONE("MASK", VOID(ai_mask(&faithful, k)), VOID(mask_at(k)));
    ONE("TS", ai_ts(&faithful, k), ts(k));
    ONE("XCH", VOID(ai_xch(&faithful, k)), VOID(xch(k)));
    ONE("LXCH", VOID(ai_lxch(&faithful, k)), VOID(lxch(k)));
    ONE("QXCH", VOID(ai_qxch(&faithful, k)), VOID(qxch(k)));
    ONE("ADS", VOID(ai_ads(&faithful, k)), VOID(ads(k)));
    ONE("INCR", VOID(ai_incr(&faithful, k)), VOID(incr(k)));
    ONE("AUG", VOID(ai_aug(&faithful, k)), VOID(aug(k)));
    ONE("DIM", VOID(ai_dim(&faithful, k)), VOID(dim(k)));
    ONE("CCS", ai_ccs(&faithful, k), sign_code(ccs_at(k)));
    ONE("MP", VOID(ai_mp(&faithful, k)), VOID(mp_at(k)));
    ONE("DV", VOID(ai_dv(&faithful, k)), VOID(dv_at(k)));
    if (k < 01777u) {
        ONE("DAS", VOID(ai_das(&faithful, k)), VOID(das(k)));
        ONE("DCA", VOID(ai_dca(&faithful, k)), VOID(dca(k)));
        ONE("DCS", VOID(ai_dcs(&faithful, k)), VOID(dcs(k)));
        ONE("DXCH", VOID(ai_dxch(&faithful, k)), VOID(dxch(k)));
    }
    /* The constant forms (CAF, CS of a constant, AD, MASK, MP, DV and CCS
     * of a word), and OVSK, BZF, BZMF. */
    k = c;
    ONE("CAF", VOID(ai_caf(&faithful, c)), VOID(set_A(c)));
    ONE("CSF", VOID(ai_csf(&faithful, c)), VOID(set_A(negate(c))));
    ONE("ADF", VOID(ai_adf(&faithful, c)), VOID((ad(c), correct_L())));
    ONE("SUF", VOID(ai_suf(&faithful, c)), VOID((su(c), correct_L())));
    ONE("MASKF", VOID(ai_maskf(&faithful, c)), VOID((mask(c), correct_L())));
    ONE("MPF", VOID(ai_mpf(&faithful, c)), VOID(mp(c)));
    ONE("CCSF", ai_ccsf(&faithful, c), sign_code(ccs(c)));
    ONE("OVSK", ai_ovsk(&faithful), A_has_overflow());
    ONE("BZF", ai_bzf(&faithful), A_is_zero());
    ONE("BZMF", ai_bzmf(&faithful), A_is_zero_or_minus());
    pair[0] = c;
    pair[1] = word15();
    ONE("DCAF", VOID(ai_dcaf(&faithful, pair)), VOID((set_L(pair[1]), A = sign_extend(pair[0]))));
    /* DV by a register's 16 bits, against the divide unit itself. */
    {
        agc_register_pair p;
        bind();
        p.a = faithful.words[0];
        p.l = faithful.words[1];
        p = agc_hardware_divide(p, divisor);
        faithful.words[0] = p.a;
        faithful.words[1] = p.l;
        dv(divisor);
        k = divisor;
        compare("DV16", k, 0, 0);
    }
}

int main(int argc, char **argv)
{
    unsigned n = argc > 1 ? (unsigned)strtoul(argv[1], NULL, 10) : 200000u, i;
    for (i = 0; i < n; ++i) {
        random_state();
        one_state();
    }
    printf("machine-cases: %u states, %u checks, %u failures\n", n, checks, failures);
    return failures != 0;
}
