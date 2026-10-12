/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES mark displays, Luminary099 and Comanche055.
 * See docs/LANEC-DI-MARK.md for the source map. */
#include "lanec_di_mark.h"
#include "lanec_di_internal.h"
#include "lanec_symbols.h"

typedef struct {
    uint16_t blanket, endmark, clearmrk, markend, gomark, kleenex, gomark2, gomark3, gomark4, gomarkr;
    uint16_t gomarkfr, gomark2r, gomark3r, makemark, markplay, copytogo, chkprio, priodsp, exdspret, skipadd;
    uint16_t relintq, postjump, markover, prioplay, endofjob, extvbact, markeban, ebanksav, copies;
} places;

#define PLACES(M) { LANEC_##M##_BLANKET, LANEC_##M##_ENDMARK, LANEC_##M##_CLEARMRK, LANEC_##M##_MARKEND, \
    LANEC_##M##_GOMARK, LANEC_##M##_KLEENEX, LANEC_##M##_GOMARK2, LANEC_##M##_GOMARK3, LANEC_##M##_GOMARK4, \
    LANEC_##M##_GOMARKR, LANEC_##M##_GOMARKFR, LANEC_##M##_GOMARK2R, LANEC_##M##_GOMARK3R, LANEC_##M##_MAKEMARK, \
    LANEC_##M##_MARKPLAY, LANEC_##M##_COPYTOGO, LANEC_##M##_CHKPRIO, LANEC_##M##_PRIODSP, LANEC_##M##_EXDSPRET, \
    LANEC_##M##_SKIPADD, LANEC_##M##_RELINTQ, LANEC_##M##_POSTJUMP, LANEC_##M##_MARKOVER, LANEC_##M##_PRIOPLAY, \
    LANEC_##M##_ENDOFJOB, LANEC_##M##_EXTVBACT, LANEC_##M##_MARKEBAN, LANEC_##M##_EBANKSAV, LANEC_##M##_COPIES }
static const places lm = PLACES(LM), cm = PLACES(CM);

#define SAME(X) (LANEC_LM_##X == LANEC_CM_##X)
#define BOTH(X, Y, N) (LANEC_LM_##X == LANEC_LM_##Y + (N) && LANEC_CM_##X == LANEC_CM_##Y + (N))
typedef char layout[(SAME(MPAC) && SAME(FLAGWRD4) && SAME(MARKNV) && SAME(LOC) && SAME(BANKSET) &&
    SAME(MARKFMSK_WORD) && SAME(MPERFMSK_WORD) && SAME(MARK3MSK_WORD) && SAME(MARK4MSK_WORD) &&
    SAME(BIT15_WORD) && SAME(OCT34300_WORD) && LANEC_LM_MRKNVBIT_WORD == LANEC_CM_BIT9_WORD &&
    LANEC_LM_XDSPBIT_WORD == LANEC_CM_BIT1_WORD && SAME(FIVE_WORD) && SAME(ONE_WORD) && SAME(BIT5_WORD) &&
    SAME(PRIOOCT_WORD) && SAME(PRIOOCT_P_1_WORD) && SAME(PRIOOCT_P_2_WORD) &&
    LANEC_LM_MARKEND_BANK == 010u && LANEC_CM_MARKEND_BANK == 010u &&
    BOTH(GOMARS, GOMARK, 1u) && BOTH(GOMARKF, KLEENEX, 1u) && BOTH(MARKFORM, GOMARK2, 1u) &&
    BOTH(MARKPLAY, MAKEMARK, 11u) && BOTH(GOGOMARK, MARKPLAY, 6u) && BOTH(MARKCOP, GOGOMARK, 6u) &&
    BOTH(COPYTOGO, MARKCOP, 2u) && BOTH(COPYPACS, COPYTOGO, 2u) && BOTH(CHKPRIO, COPYPACS, 8u) &&
    BOTH(ENDMARK, BLANKET, 6u) && BOTH(CLEARMRK, ENDMARK, 2u)) ? 1 : -1];

#define MPAC LANEC_LM_MPAC
#define PLAYTEM1 (MPAC + 1u)
#define PLAYTEM4 (MPAC + 4u)
#define TEMPOR2 (MPAC + 4u)
#define OUTHERE (MPAC + 5u)
#define GENMASK (MPAC + 6u)
#define FLAGWRD4 LANEC_LM_FLAGWRD4
#define MARKNV LANEC_LM_MARKNV
#define COPINDEX LANEC_LM_LOC
#define MPAC2SAV LANEC_LM_BANKSET
#define MARKFMSK LANEC_LM_MARKFMSK_WORD
#define MPERFMSK LANEC_LM_MPERFMSK_WORD
#define MARK3MSK LANEC_LM_MARK3MSK_WORD
#define MARK4MSK LANEC_LM_MARK4MSK_WORD
#define BIT15 LANEC_LM_BIT15_WORD
#define OCT34300 LANEC_LM_OCT34300_WORD
#define MRKNVBIT LANEC_LM_MRKNVBIT_WORD
#define XDSPBIT LANEC_LM_XDSPBIT_WORD
#define FIVE LANEC_LM_FIVE_WORD
#define ONE LANEC_LM_ONE_WORD
#define BIT5 LANEC_LM_BIT5_WORD
static const uint16_t priooct[3] = { LANEC_LM_PRIOOCT_WORD, LANEC_LM_PRIOOCT_P_1_WORD, LANEC_LM_PRIOOCT_P_2_WORD };
#define OFFSET_MAX (01376u - PLAYTEM4)   /* BLANKET's indexed ADS stays in unswitched erasable */

static const places *at(enum lanec_mission m) { return m == LANEC_LM ? &lm : &cm; }
static void set_a(apollo_overlay_state *s, uint16_t word) { s->words[LANEC_A] = lanec_a16(word); }
static uint16_t corrected(uint16_t a) { return (uint16_t)((a & 037777u) | ((a >> 1) & 040000u)); }

/* A "TC Q" back to the caller needs a TC to fixed memory outside the code
 * translated here (bank 10 MARKEND..PRIODSP and EXDSPRET..SKIPADD, RELINTQ,
 * and BLANKET..CLEARMRK in fixed-fixed). */
static uint8_t caller_ok(const places *p, uint16_t q, uint16_t bank)
{
    if (q < 02000u || q > 07777u) return 0;
    if (q >= 04000u) return (uint8_t)(q < p->blanket || q > p->clearmrk + 7u);
    if (bank != 010u) return 1;
    return (uint8_t)((q < p->markend || q > p->priodsp + 2u) && (q < p->exdspret || q > p->skipadd + 3u) &&
                     (q < p->relintq || q > p->relintq + 1u));
}

/* CLEARMRK +2: INHINT, CS XDSPBIT, MASK FLAGWRD4, TS FLAGWRD4, RELINT, TC Q. */
static void clearmrk2(apollo_overlay_state *s, lanec_io *io, const places *p)
{
    uint16_t *w = s->words;
    io->interrupt_enabled = 0;
    w[LANEC_A] = (uint16_t)~lanec_a16(XDSPBIT);
    lanec_mask(s, w[FLAGWRD4]);
    w[FLAGWRD4] = corrected(w[LANEC_A]);           /* a masked value: no overflow */
    io->interrupt_enabled = 1;
    lanec_di_tc_register(s, LANEC_Q, (uint16_t)(p->clearmrk + 8u));
}

static void clearmrk(apollo_overlay_state *s, lanec_io *io, const places *p)
{
    s->words[LANEC_A] = 0;                         /* CAF ZERO, TS EXTVBACT */
    s->words[p->extvbact] = 0;
    clearmrk2(s, io, p);
}

/* MARKPLAY: INHINT, CS FIVE, MASK FLAGWRD4, AD ONE, TS FLAGWRD4, RELINT;
 * GOGOMARK: CS MARKFLAG, MASK BIT5, CCS A, TCF MARKCOP, CS MARKNV,
 * TS MARKNV; MARKCOP: CAF ONE, TCF PRIOPLAY. */
static void markcop(apollo_overlay_state *s, const places *p)
{
    set_a(s, ONE);
    s->words[LANEC_Z] = p->prioplay;
}

static void gogomark(apollo_overlay_state *s, const places *p)
{
    uint16_t *w = s->words;
    w[LANEC_A] = (uint16_t)~lanec_a16(w[p->markeban]);
    lanec_mask(s, BIT5);
    if (lanec_ccs_a(s) == 0) { markcop(s, p); return; }
    w[LANEC_A] = (uint16_t)~lanec_a16(w[MARKNV]);
    if (lanec_ts(s, MARKNV)) { w[LANEC_Z] = p->prioplay; return; }  /* skips CAF ONE */
    markcop(s, p);
}

static void markplay(apollo_overlay_state *s, lanec_io *io, const places *p)
{
    uint16_t *w = s->words;
    io->interrupt_enabled = 0;
    w[LANEC_A] = (uint16_t)~lanec_a16(FIVE);
    lanec_mask(s, w[FLAGWRD4]);
    lanec_ad(s, ONE);
    w[FLAGWRD4] = corrected(w[LANEC_A]);           /* bit 1 was just cleared: no overflow */
    io->interrupt_enabled = 1;
    gogomark(s, p);
}

/* MAKEMARK: CAF ONE, TC COPIES, CA FLAGWRD4, MASK OCT34300, CCS A,
 * TCF CHKPRIO, CA FLAGWRD4, MASK MRKNVBIT, EXTEND, BZF MARKPLAY,
 * TCF ENDOFJOB. */
void lanec_di_makemark(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words;
    set_a(s, ONE);
    lanec_tc(s, p->copies, (uint16_t)(p->makemark + 2u));
    lanec_di_copies(s, io, m);
    set_a(s, w[FLAGWRD4]);
    lanec_mask(s, OCT34300);
    if (lanec_ccs_a(s) == 0) { lanec_di_chkprio(s, io, m); return; }
    set_a(s, w[FLAGWRD4]);
    lanec_mask(s, MRKNVBIT);
    if (w[LANEC_A] == 0) { markplay(s, io, p); return; }
    w[LANEC_Z] = p->endofjob;
}

/* COPYTOGO: CA MPAC2SAV, TS MPAC +2; COPYPACS: INDEX COPINDEX, CAF PRIOOCT,
 * TS GENMASK, INDEX COPINDEX, CA EBANKSAV, TS TEMPOR2, TS EBANK, TC Q. */
static void copypacs(apollo_overlay_state *s, const places *p, uint16_t index)
{
    uint16_t *w = s->words, v;
    set_a(s, priooct[index]);
    w[GENMASK] = priooct[index];
    v = w[p->ebanksav + index];
    set_a(s, v);
    w[TEMPOR2] = v;
    w[LANEC_EB] = (uint16_t)(v & 03400u);          /* TS EBANK; BB follows EB */
    w[LANEC_BB] = (uint16_t)(w[LANEC_FB] | (w[LANEC_EB] >> 8));
    lanec_di_tc_register(s, LANEC_Q, (uint16_t)(p->copytogo + 10u));
}

enum lanec_status lanec_di_mark_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words, z = w[LANEC_Z], bank, q = w[LANEC_Q];
    if (!lanec_registers_valid(s)) return LANEC_OUT_OF_SCOPE;
    bank = lanec_current_bank(s, io);
    if (z >= 04000u) {                             /* fixed-fixed entries */
        if (z == p->endmark) { lanec_tc(s, p->postjump, (uint16_t)(p->endmark + 1u)); return LANEC_OK; }
        if (!caller_ok(p, q, bank)) return LANEC_OUT_OF_SCOPE;
        if (z == p->clearmrk) { clearmrk(s, io, p); return LANEC_OK; }
        if (z == p->clearmrk + 2u) { clearmrk2(s, io, p); return LANEC_OK; }
        if (z == p->blanket) {
            /* BLANKET: TS MPAC +6, CS PLAYTEM4, MASK MPAC +6, INDEX MPAC +5,
             * ADS PLAYTEM4, TC Q. */
            uint16_t k = (uint16_t)(PLAYTEM4 + w[OUTHERE]);
            if (w[OUTHERE] > OFFSET_MAX) return LANEC_OUT_OF_SCOPE;
            if (!lanec_ts(s, GENMASK)) w[LANEC_A] = (uint16_t)~lanec_a16(w[PLAYTEM4]);
            lanec_mask(s, w[GENMASK]);
            lanec_ad(s, w[k]);
            w[k] = corrected(w[LANEC_A]);
            lanec_di_tc_register(s, LANEC_Q, (uint16_t)(p->blanket + 6u));
            return LANEC_OK;
        }
        return LANEC_OUT_OF_SCOPE;
    }
    if (bank != 010u) return LANEC_OUT_OF_SCOPE;
    if (z == p->markend) {
        lanec_tc(s, p->clearmrk, (uint16_t)(p->markend + 1u));
        clearmrk(s, io, p);
        w[LANEC_Z] = p->markover;                  /* MARKEND +1: TCF MARKOVER */
        return LANEC_OK;
    }
    if (z == p->gomark) { if (!lanec_ts(s, PLAYTEM1)) set_a(s, BIT15); lanec_di_goflash2(s, io, m); return LANEC_OK; }
    if (z == p->gomark + 1u) { set_a(s, BIT15); lanec_di_goflash2(s, io, m); return LANEC_OK; }
    if (z == p->kleenex || z == p->kleenex + 1u) {
        if (z == p->kleenex) w[LANEC_A] = 0;
        if (!lanec_ts(s, PLAYTEM1)) set_a(s, MARKFMSK);
        lanec_di_goflash2(s, io, m);
        return LANEC_OK;
    }
    if (z == p->gomark2) { if (!lanec_ts(s, PLAYTEM1)) set_a(s, MPERFMSK); lanec_di_goflash2(s, io, m); return LANEC_OK; }
    if (z == p->gomark2 + 1u) { set_a(s, MPERFMSK); lanec_di_goflash2(s, io, m); return LANEC_OK; }
    if (z == p->gomark3) { if (!lanec_ts(s, PLAYTEM1)) set_a(s, MARK3MSK); lanec_di_goflash2(s, io, m); return LANEC_OK; }
    if (z == p->gomark4) { if (!lanec_ts(s, PLAYTEM1)) set_a(s, MARK4MSK); lanec_di_goflash2(s, io, m); return LANEC_OK; }
    if (z == p->gomarkr) { if (!lanec_ts(s, PLAYTEM1)) set_a(s, BIT15); lanec_di_godspr2(s, io, m); return LANEC_OK; }
    if (z == p->gomarkfr) { if (!lanec_ts(s, PLAYTEM1)) set_a(s, MARKFMSK); lanec_di_godsprs(s, io, m); return LANEC_OK; }
    if (z == p->gomark2r) { if (!lanec_ts(s, PLAYTEM1)) set_a(s, MPERFMSK); lanec_di_godsprs(s, io, m); return LANEC_OK; }
    if (z == p->gomark3r) { if (!lanec_ts(s, PLAYTEM1)) set_a(s, MARK3MSK); lanec_di_godsprs(s, io, m); return LANEC_OK; }
    if (z == p->makemark) { lanec_di_makemark(s, io, m); return LANEC_OK; }
    if (z == p->markplay) { markplay(s, io, p); return LANEC_OK; }
    if (z == p->markplay + 6u) { gogomark(s, p); return LANEC_OK; }
    if (z == p->markplay + 12u) { markcop(s, p); return LANEC_OK; }
    if (z == p->copytogo || z == p->copytogo + 2u) {
        uint16_t index = w[COPINDEX];
        if (index > 2u || !caller_ok(p, q, bank)) return LANEC_OUT_OF_SCOPE;
        if (z == p->copytogo) { set_a(s, w[MPAC2SAV]); w[MPAC + 2u] = w[MPAC2SAV]; }
        copypacs(s, p, index);
        return LANEC_OK;
    }
    return LANEC_OUT_OF_SCOPE;
}
