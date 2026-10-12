/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES display sleep and wake, Luminary099 and
 * Comanche055 (bank 10). See docs/LANEC-DI-SLEEP.md for the source map. */
#include "lanec_di_sleep.h"
#include "lanec_di_internal.h"
#include "lanec_symbols.h"

typedef struct {
    uint16_t gosleeps, xchsleep, jobxchs, idlemask, xchqadd, genmark, markwake, wakeplay, markfmsk, initdsp;
    uint16_t upent2, downent2, jobwake, jobsleep, supersw, priochng, bankjump, markplay, oktocopy, endret;
    uint16_t xchtoend, ebanktem, endinst, mkovbit;
} places;

#define PLACES(M, MKOV) { LANEC_##M##_GOSLEEPS, LANEC_##M##_XCHSLEEP, LANEC_##M##_JOBXCHS, LANEC_##M##_IDLEMASK, \
    LANEC_##M##_XCHQADD, LANEC_##M##_GENMARK, LANEC_##M##_MARKWAKE, LANEC_##M##_WAKEPLAY, LANEC_##M##_MARKFMSK, \
    LANEC_##M##_INITDSP, LANEC_##M##_UPENT2, LANEC_##M##_DOWNENT2, LANEC_##M##_JOBWAKE, LANEC_##M##_JOBSLEEP, \
    LANEC_##M##_SUPERSW, LANEC_##M##_PRIOCHNG, LANEC_##M##_BANKJUMP, LANEC_##M##_MARKPLAY, LANEC_##M##_OKTOCOPY, \
    LANEC_##M##_ENDRET, LANEC_##M##_XCHTOEND, LANEC_##M##_EBANKTEM, LANEC_##M##_ENDINST_WORD, LANEC_##M##_##MKOV##_WORD }
static const places lm = PLACES(LM, MKOVBIT), cm = PLACES(CM, BIT3);

#define SAME(X) (LANEC_LM_##X == LANEC_CM_##X)
#define BOTH(X, Y, N) (LANEC_LM_##X == LANEC_LM_##Y + (N) && LANEC_CM_##X == LANEC_CM_##Y + (N))
typedef char layout[(SAME(MPAC) && SAME(FLAGWRD4) && SAME(LOC) && SAME(LOCCTR) && SAME(TEMPOR2) &&
    SAME(TEMPFLSH) && SAME(RESTREG) && SAME(LOC) && SAME(WAITMASK_WORD) && SAME(IDLEMASK_WORD) &&
    SAME(MARKFMSK_WORD) && SAME(PRIOOCT_WORD) && SAME(PRIOOCT_P_1_WORD) && SAME(PRIOOCT_P_2_WORD) &&
    SAME(IDLESLEP_WORD) && SAME(BITS5_P_11_WORD) && SAME(BITS4_P_10_WORD) && SAME(BIT5_WORD) &&
    SAME(BIT5_P_1_WORD) && SAME(FOUR_WORD) && SAME(THREE_WORD) && SAME(ONE_WORD) && SAME(PRIO37_WORD) &&
    LANEC_LM_MKOVBIT_WORD == LANEC_CM_BIT3_WORD && LANEC_LM_TEMPOR2 == LANEC_LM_MPAC + 4u &&
    LANEC_LM_GOSLEEPS_BANK == 010u && LANEC_CM_GOSLEEPS_BANK == 010u &&
    BOTH(WAITMASK, GOSLEEPS, 4u) && BOTH(XCHSLEEP, GOSLEEPS, 8u) && BOTH(JOBXCHS, XCHSLEEP, 8u) &&
    BOTH(IDLEMASK, JOBXCHS, 12u) && BOTH(XCHQADD, JOBXCHS, 17u) && BOTH(GENMARK, JOBXCHS, 21u) &&
    BOTH(MARKWAKE, JOBXCHS, 23u) && BOTH(WAKEPLAY, MARKWAKE, 1u) && BOTH(MARKFMSK, WAKEPLAY, 5u) &&
    BOTH(INITDSP, WAKEPLAY, 11u) && LANEC_LM_XCHQADD_WORD == LANEC_LM_XCHSLEEP &&
    LANEC_CM_XCHQADD_WORD == LANEC_CM_XCHSLEEP) ? 1 : -1];

#define MPAC LANEC_LM_MPAC
#define FACEREG MPAC
#define TEMPOR2 LANEC_LM_TEMPOR2
#define TEMPFLSH LANEC_LM_TEMPFLSH
#define FLAGWRD4 LANEC_LM_FLAGWRD4
#define COPINDEX LANEC_LM_LOC
#define LOC LANEC_LM_LOC
#define LOCCTR LANEC_LM_LOCCTR
#define RESTREG LANEC_LM_RESTREG
#define LOCCTR_MAX (01376u - LOC)
#define LOCCTR_NONE 077776u                          /* -1: JOBWAKE found no sleeping job */

static const uint16_t priooct[3] = { LANEC_LM_PRIOOCT_WORD, LANEC_LM_PRIOOCT_P_1_WORD, LANEC_LM_PRIOOCT_P_2_WORD };
static const uint16_t bit5[2] = { LANEC_LM_BIT5_WORD, LANEC_LM_BIT5_P_1_WORD };
static const uint16_t bits5p11[2] = { LANEC_LM_BITS5_P_11_WORD, LANEC_LM_BITS4_P_10_WORD };
static const uint16_t wakecadr_lm[2] = { LANEC_LM_WAKECADR_WORD, LANEC_LM_WAKECADR_P_1_WORD };
static const uint16_t wakecadr_cm[2] = { LANEC_CM_WAKECADR_WORD, LANEC_CM_WAKECADR_P_1_WORD };

static const places *at(enum lanec_mission m) { return m == LANEC_LM ? &lm : &cm; }
static void set_a(apollo_overlay_state *s, uint16_t word) { s->words[LANEC_A] = lanec_a16(word); }
static uint16_t wakecadr(enum lanec_mission m, uint16_t i) { return m == LANEC_LM ? wakecadr_lm[i] : wakecadr_cm[i]; }
/* An INDEX by -0 (CS ONE + 1 in FACEREG for MARK) adds nothing. */
static uint16_t idx(uint16_t v) { return v == 077777u ? 0u : v; }
static uint8_t is01(uint16_t v) { return (uint8_t)(idx(v) <= 1u); }

/* XCHSLEEP: INDEX FACEREG, CAF WAKECADR, INHINT, TC JOBWAKE. */
static void xchsleep(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, const places *p)
{
    set_a(s, wakecadr(m, idx(s->words[FACEREG])));
    io->interrupt_enabled = 0;
    lanec_tc(s, p->jobwake, (uint16_t)(p->xchsleep + 4u));
}

/* XCHSLEEP +5: INDEX FACEREG, CAF WAKECADR, TCF JOBSLEEP. */
static void xchsleep5(apollo_overlay_state *s, enum lanec_mission m, const places *p)
{
    set_a(s, wakecadr(m, idx(s->words[FACEREG])));
    s->words[LANEC_Z] = p->jobsleep;
}

/* JOBXCHS +8: INDEX FACEREG, CA MARKOCT, MASK IDLESLEP, TC DOWNENT2. */
static void jobxchs8(apollo_overlay_state *s, const places *p)
{
    set_a(s, priooct[1u + idx(s->words[FACEREG])]);
    lanec_mask(s, LANEC_LM_IDLESLEP_WORD);
    lanec_tc(s, p->downent2, p->idlemask);
}

/* WAKEPLAY +1: INDEX TEMPOR2 (unless skipped), CA BITS5+11, AD FOUR, TC DOWNENT2. */
static void wakeplay1(apollo_overlay_state *s, const places *p, uint16_t index)
{
    set_a(s, bits5p11[index]);
    lanec_ad(s, LANEC_LM_FOUR_WORD);
    lanec_tc(s, p->downent2, p->markfmsk);
}

static uint8_t locctr_ok(uint16_t v) { return (uint8_t)(v == LOCCTR_NONE || v <= LOCCTR_MAX); }

enum lanec_status lanec_di_sleep_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words, z, v;
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 010u) return LANEC_OUT_OF_SCOPE;
    z = w[LANEC_Z];
    if (z == p->gosleeps) {                           /* INDEX COPINDEX, CA PRIOOCT, MASK WAITMASK, TC UPENT2 */
        if (w[COPINDEX] > 2u) return LANEC_OUT_OF_SCOPE;
        set_a(s, priooct[w[COPINDEX]]);
        lanec_mask(s, LANEC_LM_WAITMASK_WORD);
        lanec_tc(s, p->upent2, (uint16_t)(p->gosleeps + 4u));
        return LANEC_OK;
    }
    if (z == p->gosleeps + 5u) {                      /* CS ONE, AD COPINDEX, TS FACEREG; XCHSLEEP */
        if (w[COPINDEX] < 1u || w[COPINDEX] > 2u) return LANEC_OUT_OF_SCOPE;
        w[LANEC_A] = (uint16_t)~lanec_a16(LANEC_LM_ONE_WORD);
        lanec_ad(s, w[COPINDEX]);
        (void)lanec_ts(s, FACEREG);
        xchsleep(s, io, m, p);
        return LANEC_OK;
    }
    if (z == p->xchsleep) { if (!is01(w[FACEREG])) return LANEC_OUT_OF_SCOPE; xchsleep(s, io, m, p); return LANEC_OK; }
    if (z == p->xchsleep + 4u) {                      /* TC XCHTOEND */
        v = w[LOCCTR];
        if (!is01(w[FACEREG]) || !((v & 040000u) || v <= LOCCTR_MAX)) return LANEC_OUT_OF_SCOPE;
        lanec_tc(s, p->xchtoend, (uint16_t)(p->xchsleep + 5u));
        set_a(s, p->endinst);
        lanec_di_xchnyloc(s, io, m);
        xchsleep5(s, m, p);
        return LANEC_OK;
    }
    if (z == p->xchsleep + 5u) { if (!is01(w[FACEREG])) return LANEC_OUT_OF_SCOPE; xchsleep5(s, m, p); return LANEC_OK; }
    if (z == p->jobxchs) {                            /* TS FACEREG, TC WITCHONE, TC JOBWAKE */
        if (w[LANEC_A] > 1u) return LANEC_OUT_OF_SCOPE;
        w[FACEREG] = w[LANEC_A];
        lanec_di_witchone(s, io, m, (uint16_t)(p->jobxchs + 2u));
        lanec_tc(s, p->jobwake, (uint16_t)(p->jobxchs + 3u));
        return LANEC_OK;
    }
    if (z == p->jobxchs + 3u) {   /* CA FACEREG, INDEX LOCCTR, TS FACEREG, CAF XCHQADD, TC XCHNYLOC; +8 */
        v = w[LOCCTR];
        if (!is01(w[FACEREG]) || !locctr_ok(v)) return LANEC_OUT_OF_SCOPE;
        set_a(s, w[FACEREG]);
        w[v == LOCCTR_NONE ? FACEREG - 1u : FACEREG + v] = w[FACEREG];
        lanec_tc(s, (uint16_t)(p->xchtoend + 1u), (uint16_t)(p->jobxchs + 8u));
        set_a(s, p->xchsleep);
        lanec_di_xchnyloc(s, io, m);
        jobxchs8(s, p);
        return LANEC_OK;
    }
    if (z == p->jobxchs + 8u) { if (!is01(w[FACEREG])) return LANEC_OUT_OF_SCOPE; jobxchs8(s, p); return LANEC_OK; }
    if (z == p->idlemask + 1u) {                      /* INDEX FACEREG, CA BIT5, AD FOUR, TC UPENT2 */
        if (!is01(w[FACEREG])) return LANEC_OUT_OF_SCOPE;
        set_a(s, bit5[idx(w[FACEREG])]);
        lanec_ad(s, LANEC_LM_FOUR_WORD);
        lanec_tc(s, p->upent2, p->xchqadd);
        return LANEC_OK;
    }
    if (z == p->xchqadd + 1u) {                       /* CA FLAGWRD4, MASK MKOVBIT, CCS A */
        set_a(s, w[FLAGWRD4]);
        lanec_mask(s, p->mkovbit);
        if (lanec_ccs_a(s) == 0) lanec_tc(s, p->markplay, (uint16_t)(p->genmark + 1u));
        else w[LANEC_Z] = p->oktocopy;
        return LANEC_OK;
    }
    if (z == p->markwake || z == p->wakeplay) {       /* CAF ZERO; WAKEPLAY: TS TEMPOR2 */
        if (z == p->markwake) w[LANEC_A] = 0;
        else if (w[LANEC_A] > 1u && (w[LANEC_A] >> 14) != 1u && (w[LANEC_A] >> 14) != 2u) return LANEC_OUT_OF_SCOPE;
        if (lanec_ts(s, TEMPOR2)) wakeplay1(s, p, 0);  /* an overflow skips the INDEX */
        else wakeplay1(s, p, idx(w[TEMPOR2]));
        return LANEC_OK;
    }
    if (z == p->markfmsk + 1u) {                      /* INDEX TEMPOR2, CAF WAKECADR, INHINT, TC JOBWAKE */
        if (!is01(w[TEMPOR2])) return LANEC_OUT_OF_SCOPE;
        set_a(s, wakecadr(m, idx(w[TEMPOR2])));
        io->interrupt_enabled = 0;
        lanec_tc(s, p->jobwake, (uint16_t)(p->markfmsk + 5u));
        return LANEC_OK;
    }
    if (z == p->markfmsk + 5u) { w[LANEC_Z] = p->endret; return LANEC_OK; }
    if (z == p->initdsp) {                            /* CA EBANKTEM, TS EBANK, CA RESTREG, TC SUPERSW */
        v = w[p->ebanktem];
        set_a(s, v);
        w[LANEC_EB] = (uint16_t)(v & 03400u);         /* BB follows EB */
        w[LANEC_BB] = (uint16_t)(w[LANEC_FB] | (w[LANEC_EB] >> 8));
        set_a(s, w[RESTREG]);
        lanec_tc(s, p->supersw, (uint16_t)(p->initdsp + 4u));
        return LANEC_OK;
    }
    if (z == p->initdsp + 4u) {                       /* MASK PRIO37, TC PRIOCHNG */
        lanec_mask(s, LANEC_LM_PRIO37_WORD);
        lanec_tc(s, p->priochng, (uint16_t)(p->initdsp + 6u));
        return LANEC_OK;
    }
    if (z == p->initdsp + 6u) {                       /* CS THREE, AD TEMPFLSH, TCF BANKJUMP */
        w[LANEC_A] = (uint16_t)~lanec_a16(LANEC_LM_THREE_WORD);
        lanec_ad(s, w[TEMPFLSH]);
        w[LANEC_Z] = p->bankjump;
        return LANEC_OK;
    }
    return LANEC_OUT_OF_SCOPE;
}
