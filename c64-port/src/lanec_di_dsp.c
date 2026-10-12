/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES display-start entries, Luminary099 and
 * Comanche055 (bank 10). See docs/LANEC-DI-DSP.md for the source map. */
#include "lanec_di_dsp.h"
#include "lanec_symbols.h"

typedef struct {
    uint16_t chkprio, markperf, godsp, godspret, godspr, cleandsp, reflashr, regodsp, regodspr, clocplay;
    uint16_t goflash, goflash2, priodspr, priodsp, exdspret, goperf1, goperf2, goperf4, goflashr, godsprs;
    uint16_t godsprs1, vacdsp, bothjobs, goperf1r, goperf2r, goperf4r, purrs4, savelocs, savelocr;
    uint16_t copynorm, copies, copies2, skipadd, relintq;
    uint16_t marsleep, setnorm, nv50dsp, makeplay, novac, spvac, makecadr, bankjump;
    uint16_t option1, ebanksav;  /* erasable cells placed differently in the two missions */
    uint16_t makegen;            /* GENADR MAKEPLAY */
} places;

#define PLACES(M) { LANEC_##M##_CHKPRIO, LANEC_##M##_MARKPERF, LANEC_##M##_GODSP, LANEC_##M##_GODSPRET, \
    LANEC_##M##_GODSPR, LANEC_##M##_CLEANDSP, REFLASHR_##M, LANEC_##M##_REGODSP, LANEC_##M##_REGODSPR, \
    LANEC_##M##_CLOCPLAY, LANEC_##M##_GOFLASH, LANEC_##M##_GOFLASH2, LANEC_##M##_PRIODSPR, LANEC_##M##_PRIODSP, \
    LANEC_##M##_EXDSPRET, LANEC_##M##_GOPERF1, LANEC_##M##_GOPERF2, LANEC_##M##_GOPERF4, LANEC_##M##_GOFLASHR, \
    LANEC_##M##_GODSPRS, LANEC_##M##_GODSPRS1, LANEC_##M##_VACDSP, LANEC_##M##_BOTHJOBS, LANEC_##M##_GOPERF1R, \
    LANEC_##M##_GOPERF2R, LANEC_##M##_GOPERF4R, LANEC_##M##_PURRS4, LANEC_##M##_SAVELOCS, LANEC_##M##_SAVELOCR, \
    LANEC_##M##_COPYNORM, LANEC_##M##_COPIES, LANEC_##M##_COPIES2, LANEC_##M##_SKIPADD, LANEC_##M##_RELINTQ, \
    LANEC_##M##_MARSLEEP, LANEC_##M##_SETNORM, LANEC_##M##_NV50DSP, LANEC_##M##_MAKEPLAY, LANEC_##M##_NOVAC, \
    LANEC_##M##_SPVAC, LANEC_##M##_MAKECADR, LANEC_##M##_BANKJUMP, LANEC_##M##_OPTION1, LANEC_##M##_EBANKSAV, \
    LANEC_##M##_MAKEGEN_WORD }
#define REFLASHR_LM LANEC_LM_REFLASHR
#define REFLASHR_CM 0u               /* Comanche055 has no REFLASHR */
static const places lm = PLACES(LM), cm = PLACES(CM);

/* Layout facts the paths below rely on, both missions. */
#define SAME(X) (LANEC_LM_##X == LANEC_CM_##X)
typedef char layout[(SAME(MPAC) && SAME(FLAGWRD4) && SAME(MARKNV) && SAME(NORMTEM1) && SAME(RUPTREG3) &&
    SAME(PRIORITY) && SAME(NEWPRIO) && SAME(LOCCTR) && SAME(CADRFLSH) && SAME(NVWORD) && SAME(LOC) &&
    SAME(V01N25_WORD) && SAME(PERFMASK_WORD) && SAME(PERF2MSK_WORD) && SAME(V04N06_WORD) &&
    SAME(PERF4MSK_WORD) && SAME(REDOMASK_WORD) && SAME(CLOCKCON_WORD) && SAME(BITS7_P_4_WORD) &&
    SAME(OCT3400_WORD) && SAME(CADRMASK_WORD) && SAME(MID7_WORD) && SAME(OCT24100_WORD) &&
    SAME(BIT15_P_6_WORD) && SAME(PRIO37_WORD) && SAME(OCTAL3_WORD) && SAME(BIT3_WORD) && SAME(BIT4_WORD) &&
    SAME(BIT6_WORD) && SAME(BIT14_WORD) && LANEC_LM_GODSP_BANK == 010u && LANEC_CM_GODSP_BANK == 010u &&
    LANEC_LM_GOFLASH2 == LANEC_LM_GOFLASH + 2u && LANEC_CM_GOFLASH2 == LANEC_CM_GOFLASH + 2u &&
    LANEC_LM_GODSPRS == LANEC_LM_GOFLASHR + 2u && LANEC_CM_GODSPRS == LANEC_CM_GOFLASHR + 2u &&
    LANEC_LM_GODSPRS1 == LANEC_LM_GODSPRS + 2u && LANEC_CM_GODSPRS1 == LANEC_CM_GODSPRS + 2u &&
    LANEC_LM_VACDSP == LANEC_LM_GODSPRS1 + 14u && LANEC_CM_VACDSP == LANEC_CM_GODSPRS1 + 14u &&
    LANEC_LM_BOTHJOBS == LANEC_LM_VACDSP + 6u && LANEC_CM_BOTHJOBS == LANEC_CM_VACDSP + 6u &&
    LANEC_LM_SAVELOCR == LANEC_LM_SAVELOCS + 5u && LANEC_CM_SAVELOCR == LANEC_CM_SAVELOCS + 5u &&
    LANEC_LM_COPIES == LANEC_LM_COPYNORM + 1u && LANEC_LM_COPIES2 == LANEC_LM_COPIES + 1u &&
    LANEC_LM_SKIPADD == LANEC_LM_COPIES2 + 10u && LANEC_CM_SKIPADD == LANEC_CM_COPIES2 + 10u &&
    LANEC_CM_COPIES == LANEC_CM_COPYNORM + 1u && LANEC_CM_COPIES2 == LANEC_CM_COPIES + 1u &&
    LANEC_LM_PRIODSP == LANEC_LM_PRIODSPR + 3u && LANEC_CM_PRIODSP == LANEC_CM_PRIODSPR + 3u) ? 1 : -1];

#define MPAC LANEC_LM_MPAC
#define PLAYTEM1 (MPAC + 1u)
#define PLAYTEM3 (MPAC + 3u)
#define PLAYTEM4 (MPAC + 4u)
#define FLAGWRD4 LANEC_LM_FLAGWRD4
#define MARKNV LANEC_LM_MARKNV
#define NORMTEM1 LANEC_LM_NORMTEM1
#define RUPTREG3 LANEC_LM_RUPTREG3
#define PRIORITY LANEC_LM_PRIORITY
#define NEWPRIO LANEC_LM_NEWPRIO
#define LOCCTR LANEC_LM_LOCCTR
#define CADRFLSH LANEC_LM_CADRFLSH
#define NVWORD LANEC_LM_NVWORD
#define COPINDEX LANEC_LM_LOC
#define V01N25 LANEC_LM_V01N25_WORD
#define PERFMASK LANEC_LM_PERFMASK_WORD
#define PERF2MSK LANEC_LM_PERF2MSK_WORD
#define V04N06 LANEC_LM_V04N06_WORD
#define PERF4MSK LANEC_LM_PERF4MSK_WORD
#define REDOMASK LANEC_LM_REDOMASK_WORD
#define CLOCKCON LANEC_LM_CLOCKCON_WORD
#define BITS7P4 LANEC_LM_BITS7_P_4_WORD
#define OCT3400 LANEC_LM_OCT3400_WORD
#define CADRMASK LANEC_LM_CADRMASK_WORD
#define VERBMASK LANEC_LM_MID7_WORD
#define OCT24100 LANEC_LM_OCT24100_WORD
#define BIT15P6 LANEC_LM_BIT15_P_6_WORD
#define PRIO37 LANEC_LM_PRIO37_WORD
#define THREE LANEC_LM_OCTAL3_WORD
#define MKOVBIT LANEC_LM_BIT3_WORD
#define BIT4 LANEC_LM_BIT4_WORD
#define BIT6 LANEC_LM_BIT6_WORD
#define BIT14 LANEC_LM_BIT14_WORD
#define SUPERBNK_READ(io) ((io)->superbank)
/* Highest LOCCTR (job register-set offset) for which both BOTHJOBS copies
 * land in unswitched erasable below 01377; COPINDEX likewise. */
#define LOCCTR_MAX (01376u - (MPAC + 4u))
#define COPINDEX_MAX 0300u

static const places *at(enum lanec_mission m) { return m == LANEC_LM ? &lm : &cm; }

static void set_a(apollo_overlay_state *s, uint16_t word) { s->words[LANEC_A] = lanec_a16(word); }
static uint16_t corrected(uint16_t a) { return (uint16_t)((a & 037777u) | ((a >> 1) & 040000u)); }

/* Source "TC Q" / "TC L": Z goes to the register (2 or 1), which holds a
 * TC to fixed memory; that TC leaves Q = register address + 1. */
static void tc_register(apollo_overlay_state *s, uint16_t reg, uint16_t return_address)
{
    uint16_t target = s->words[reg];
    lanec_tc(s, reg, return_address);
    lanec_tc(s, target, (uint16_t)(reg + 1u));
}

/* SAVELOCS: INHINT, CS OCT3400, MASK PLAYTEM4, AD EBANK, TS PLAYTEM4,
 * SAVELOCR: LXCH Q, TC MAKECADR. */
static void savelocr(apollo_overlay_state *s, const places *p)
{
    uint16_t *w = s->words, t = w[LANEC_L];
    w[LANEC_L] = w[LANEC_Q]; w[LANEC_Q] = t;
    lanec_tc(s, p->makecadr, (uint16_t)(p->savelocs + 7u));
}

static void savelocs(apollo_overlay_state *s, lanec_io *io, const places *p)
{
    uint16_t *w = s->words;
    io->interrupt_enabled = 0;
    w[LANEC_A] = (uint16_t)~lanec_a16(OCT3400);
    lanec_mask(s, w[PLAYTEM4]);
    lanec_ad(s, w[LANEC_EB]);
    if (lanec_ts(s, PLAYTEM4)) { lanec_tc(s, p->makecadr, (uint16_t)(p->savelocs + 7u)); return; }
    savelocr(s, p);
}

/* GOFLASH2: TS PLAYTEM4, TC SAVELOCS, RELINT, TCF MAKEPLAY. */
static void goflash2(apollo_overlay_state *s, lanec_io *io, const places *p)
{
    if (lanec_ts(s, PLAYTEM4)) { io->interrupt_enabled = 1; s->words[LANEC_Z] = p->makeplay; return; }
    lanec_tc(s, p->savelocs, (uint16_t)(p->goflash2 + 2u));
    savelocs(s, io, p);
}

/* GODSPRS1: INHINT, TS RUPTREG3, CA PRIORITY, MASK PRIO37, TS NEWPRIO,
 * CA PLAYTEM4, MASK BIT4, CCS A, TCF VACDSP, CA NEWPRIO, TC NOVAC (2CADR
 * MAKEPLAY); VACDSP: CA BBANK, EXTEND, ROR SUPERBNK, TS L, CAF MAKEGEN,
 * TC SPVAC. */
static void godsprs1(apollo_overlay_state *s, lanec_io *io, const places *p)
{
    uint16_t *w = s->words;
    io->interrupt_enabled = 0;
    if (!lanec_ts(s, RUPTREG3)) set_a(s, w[PRIORITY]);
    lanec_mask(s, PRIO37);
    if (!lanec_ts(s, NEWPRIO)) set_a(s, w[PLAYTEM4]);
    lanec_mask(s, BIT4);
    if (lanec_ccs_a(s) == 0) {                         /* flashing R display: a VAC job */
        set_a(s, w[LANEC_BB]);
        set_a(s, (uint16_t)(corrected(w[LANEC_A]) | SUPERBNK_READ(io)));
        w[LANEC_L] = w[LANEC_A];                        /* TS L; A cannot overflow here */
        set_a(s, p->makegen);
        lanec_tc(s, p->spvac, p->bothjobs);
        return;
    }
    set_a(s, w[NEWPRIO]);
    lanec_tc(s, p->novac, (uint16_t)(p->godsprs1 + 11u));
}

/* GODSPRS: TS PLAYTEM4, CAF THREE, then GODSPRS1. GODSPR2: TS PLAYTEM4,
 * CAF ZERO, TCF GODSPRS1. */
static void godsprs(apollo_overlay_state *s, lanec_io *io, const places *p, uint16_t constant)
{
    if (!lanec_ts(s, PLAYTEM4)) set_a(s, constant);
    godsprs1(s, io, p);
}

/* PURRS4: TS OPTION1, CAF V04N06, TS PLAYTEM1, CAF PERF4MSK, TC Q. */
static void purrs4(apollo_overlay_state *s, const places *p, uint16_t back)
{
    lanec_tc(s, p->purrs4, back);
    if (!lanec_ts(s, p->option1)) set_a(s, V04N06);
    if (!lanec_ts(s, PLAYTEM1)) set_a(s, PERF4MSK);
    tc_register(s, LANEC_Q, (uint16_t)(p->purrs4 + 5u));
}

/* "TS cell" then a constant (the skip on overflow leaves A at +-1). */
static void ts_caf(apollo_overlay_state *s, uint16_t cell, uint16_t constant)
{
    if (!lanec_ts(s, cell)) set_a(s, constant);
}

/* MAKECADR returns to SAVELOCS +7: TS PLAYTEM3, AD RUPTREG3, TC L. L holds
 * the return SAVELOCR saved: GOFLASH2 +2 (RELINT, TCF MAKEPLAY), BOTHJOBS +1
 * (copy the user's words into the job's register set, TC SAVELOCR) or
 * BOTHJOBS +12 (RELINT, TCF BANKJUMP). */
static uint8_t back_ok(const apollo_overlay_state *s, const places *p)
{
    uint16_t l = s->words[LANEC_L];
    if (l == p->goflash2 + 2u || l == p->bothjobs + 12u) return 1;
    return (uint8_t)(l == p->bothjobs + 1u && s->words[LOCCTR] <= LOCCTR_MAX);
}

static void dxch_indexed(apollo_overlay_state *s, uint16_t k)
{
    uint16_t *w = s->words, a = w[LANEC_A], l = w[LANEC_L], hi = w[k], lo = w[k + 1u];
    w[k] = corrected(a); w[k + 1u] = corrected(l);
    set_a(s, hi); w[LANEC_L] = lanec_a16(lo);
}

static void saved_return(apollo_overlay_state *s, lanec_io *io, const places *p)
{
    uint16_t *w = s->words;
    if (!lanec_ts(s, PLAYTEM3)) lanec_ad(s, w[RUPTREG3]);
    tc_register(s, LANEC_L, (uint16_t)(p->savelocs + 10u));
    if (w[LANEC_Z] == p->bothjobs + 1u) {
        set_a(s, w[MPAC + 1u]); w[LANEC_L] = lanec_a16(w[MPAC + 2u]);   /* DCA MPAC +1 */
        dxch_indexed(s, (uint16_t)(MPAC + 1u + w[LOCCTR]));
        set_a(s, w[MPAC + 3u]); w[LANEC_L] = lanec_a16(w[MPAC + 4u]);   /* DCA MPAC +3 */
        dxch_indexed(s, (uint16_t)(MPAC + 3u + w[LOCCTR]));
        set_a(s, w[LOCCTR]);                                            /* CA LOCCTR */
        w[MPAC + 5u] = w[LOCCTR];                                       /* TS MPAC +5 */
        lanec_tc(s, p->savelocr, (uint16_t)(p->bothjobs + 12u));
        savelocr(s, p);
        return;
    }
    io->interrupt_enabled = 1;                                          /* RELINT */
    w[LANEC_Z] = w[LANEC_Z] == p->goflash2 + 2u ? p->makeplay : p->bankjump;
}

/* COPIES: TS COPINDEX; COPIES2: INHINT, CA PLAYTEM4, INDEX COPINDEX,
 * TS EBANKSAV, MASK CADRMASK, EXTEND, BZF SKIPADD, CA PLAYTEM3, INDEX
 * COPINDEX, TS CADRFLSH; SKIPADD: CA PLAYTEM1, INDEX COPINDEX, TS NVWORD,
 * TCF RELINTQ; RELINTQ: RELINT, TC Q. */
static void copies2(apollo_overlay_state *s, lanec_io *io, const places *p)
{
    uint16_t *w = s->words, i = w[COPINDEX];
    io->interrupt_enabled = 0;
    set_a(s, w[PLAYTEM4]);
    w[p->ebanksav + i] = w[PLAYTEM4];
    lanec_mask(s, CADRMASK);
    if (w[LANEC_A] != 0) { set_a(s, w[PLAYTEM3]); w[CADRFLSH + i] = w[PLAYTEM3]; }
    set_a(s, w[PLAYTEM1]);
    w[NVWORD + i] = w[PLAYTEM1];
    io->interrupt_enabled = 1;
    tc_register(s, LANEC_Q, (uint16_t)(p->relintq + 2u));
}

static uint8_t copies_ok(const apollo_overlay_state *s, uint16_t index)
{
    uint16_t q = s->words[LANEC_Q];
    return (uint8_t)(index <= COPINDEX_MAX && q >= 04000u && q <= 07777u);
}

enum lanec_status lanec_di_dsp_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words, z = w[LANEC_Z], a;
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 010u) return LANEC_OUT_OF_SCOPE;
    a = w[LANEC_A];
    if (z == p->copynorm || z == p->copies || z == p->copies2) {
        uint16_t index = z == p->copynorm ? 0u : z == p->copies ? corrected(a) : w[COPINDEX];
        if (!copies_ok(s, index)) return LANEC_OUT_OF_SCOPE;
        if (z == p->copynorm) w[LANEC_A] = 0;
        /* TS COPINDEX; an overflow skip passes over INHINT, which RELINTQ
         * undoes anyway, so both paths end alike. */
        if (z != p->copies2) (void)lanec_ts(s, COPINDEX);
        copies2(s, io, p);
        return LANEC_OK;
    }
    if (z == p->savelocs + 7u) {
        if (!back_ok(s, p)) return LANEC_OUT_OF_SCOPE;
        saved_return(s, io, p);
        return LANEC_OK;
    }
    if (z == p->chkprio) {
        set_a(s, w[FLAGWRD4]);
        lanec_mask(s, OCT24100);
        if (lanec_ccs_a(s) == 0) { w[LANEC_Z] = p->marsleep; return LANEC_OK; }
        w[LANEC_A] = (uint16_t)~lanec_a16(w[FLAGWRD4]);
        lanec_mask(s, MKOVBIT);
        io->interrupt_enabled = 0;
        lanec_ad(s, w[FLAGWRD4]);                                       /* ADS FLAGWRD4 */
        w[FLAGWRD4] = corrected(w[LANEC_A]);
        w[LANEC_Z] = p->setnorm;
        return LANEC_OK;
    }
    if (z == p->markperf) {
        set_a(s, w[MARKNV]);
        lanec_mask(s, VERBMASK);
        w[LANEC_Z] = p->nv50dsp;
        return LANEC_OK;
    }
    if (z == p->godsp) { ts_caf(s, PLAYTEM1, 0); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->godsp + 1u || z == p->cleandsp) { w[LANEC_A] = 0; if (z == p->godsp + 1u) { goflash2(s, io, p); return LANEC_OK; } z = p->cleandsp + 1u; }
    if (z == p->cleandsp + 1u) { ts_caf(s, PLAYTEM1, REDOMASK); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->godspret) { ts_caf(s, PLAYTEM1, BIT6); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->godspr || z == p->regodspr) {
        ts_caf(s, PLAYTEM1, z == p->godspr ? 0u : BIT14);
        godsprs(s, io, p, 0);
        return LANEC_OK;
    }
    if (z == p->godspr + 1u) { w[LANEC_A] = 0; godsprs(s, io, p, 0); return LANEC_OK; }
    if (z == p->godspr + 2u) { godsprs(s, io, p, 0); return LANEC_OK; }
    if (m == LANEC_LM && z == p->reflashr) { ts_caf(s, PLAYTEM1, REDOMASK); godsprs(s, io, p, THREE); return LANEC_OK; }
    if (z == p->regodsp) { ts_caf(s, PLAYTEM1, BIT14); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->clocplay) { ts_caf(s, PLAYTEM1, CLOCKCON); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->goflash) { ts_caf(s, PLAYTEM1, BIT4); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->goflash2) { goflash2(s, io, p); return LANEC_OK; }
    if (z == p->priodspr) { ts_caf(s, PLAYTEM1, BITS7P4); godsprs(s, io, p, THREE); return LANEC_OK; }
    if (z == p->priodsp) { ts_caf(s, PLAYTEM1, BITS7P4); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->priodsp + 1u) { set_a(s, BITS7P4); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->exdspret) { ts_caf(s, PLAYTEM1, BIT15P6); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->goperf1 || z == p->goperf1r) { ts_caf(s, NORMTEM1, V01N25); z = (uint16_t)(z + 2u); }
    if (z == p->goperf1 + 2u) { ts_caf(s, PLAYTEM1, PERFMASK); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->goperf1r + 2u) { ts_caf(s, PLAYTEM1, PERFMASK); godsprs(s, io, p, THREE); return LANEC_OK; }
    if (z == p->goperf2) { ts_caf(s, PLAYTEM1, PERF2MSK); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->goperf2r) { ts_caf(s, PLAYTEM1, PERF2MSK); godsprs(s, io, p, THREE); return LANEC_OK; }
    if (z == p->goperf4) { purrs4(s, p, (uint16_t)(p->goperf4 + 1u)); goflash2(s, io, p); return LANEC_OK; }
    if (z == p->goperf4r) { purrs4(s, p, (uint16_t)(p->goperf4r + 1u)); godsprs(s, io, p, THREE); return LANEC_OK; }
    if (z == p->goflashr) { ts_caf(s, PLAYTEM1, BIT4); godsprs(s, io, p, THREE); return LANEC_OK; }
    if (z == p->godsprs) { godsprs(s, io, p, THREE); return LANEC_OK; }
    if (z == p->godsprs1) { godsprs1(s, io, p); return LANEC_OK; }
    if (z == p->godsprs1 + 13u || z == p->bothjobs) {               /* NOVAC returns: TCF BOTHJOBS */
        lanec_tc(s, p->savelocs, (uint16_t)(p->bothjobs + 1u));
        savelocs(s, io, p);
        return LANEC_OK;
    }
    return LANEC_OUT_OF_SCOPE;
}
