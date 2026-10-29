/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES display job selection, Luminary099 and
 * Comanche055 (bank 10). See docs/LANEC-DI-PLAY.md for the source map. */
#include "lanec_di_play.h"
#include "lanec_di_internal.h"
#include "lanec_symbols.h"

typedef struct {
    uint16_t makeprio, witchone, linuschr, jobxchs, gosleeps, goplay, priochng, poodoo, jobwake, qplus1;
    uint16_t endofjob, ebanktem, priotime, endinst, makemark, copynorm, copies2, relintq, markend;
} places;

#define PLACES(M) { LANEC_##M##_MAKEPRIO, LANEC_##M##_WITCHONE, LANEC_##M##_LINUSCHR, LANEC_##M##_JOBXCHS, \
    LANEC_##M##_GOSLEEPS, LANEC_##M##_GOPLAY, LANEC_##M##_PRIOCHNG, LANEC_##M##_POODOO, LANEC_##M##_JOBWAKE, \
    LANEC_##M##_Q_P_1, LANEC_##M##_ENDOFJOB, LANEC_##M##_EBANKTEM, LANEC_##M##_PRIOTIME, LANEC_##M##_ENDINST_WORD, \
    LANEC_##M##_MAKEMARK, LANEC_##M##_COPYNORM, LANEC_##M##_COPIES2, LANEC_##M##_RELINTQ, LANEC_##M##_MARKEND }
static const places lm = PLACES(LM), cm = PLACES(CM);

/* Offsets from MAKEPRIO and WITCHONE, the same in both missions. */
#define OFF_HIPRIO 8u
#define OFF_SETMARK 12u
#define OFF_ASKIFNRM 14u
#define OFF_SETNORM 18u
#define OFF_OKTOCOPY 20u
#define OFF_REDOPRIO 24u
#define OFF_KEEPPRIO 26u
#define OFF_MAKEPLAY 28u
#define OFF_IFLEGAL 39u
#define OFF_PRIOBORT 51u
#define OFF_OKTOPLAY 53u
#define OFF_PLAYJUM1 67u
#define OFF_PRIOPLAY 68u
#define OFF_XCHTOEND 13u
#define OFF_RELINTQ 20u
#define AT(M, X, N) (LANEC_##M##_##X == LANEC_##M##_MAKEPRIO + (N))
#define AT2(X, N) (AT(LM, X, N) && AT(CM, X, N))
#define SAME(X) (LANEC_LM_##X == LANEC_CM_##X)
typedef char layout[(AT2(HIPRIO, OFF_HIPRIO) && AT2(SETMARK, OFF_SETMARK) && AT2(ASKIFNRM, OFF_ASKIFNRM) &&
    AT2(SETNORM, OFF_SETNORM) && AT2(OKTOCOPY, OFF_OKTOCOPY) && AT2(REDOPRIO, OFF_REDOPRIO) && AT2(KEEPPRIO, OFF_KEEPPRIO) &&
    AT2(MAKEPLAY, OFF_MAKEPLAY) && AT2(IFLEGAL, OFF_IFLEGAL) && AT2(PRIOBORT, OFF_PRIOBORT) && AT2(OKTOPLAY, OFF_OKTOPLAY) &&
    AT2(PLAYJUM1, OFF_PLAYJUM1) && AT2(PRIOPLAY, OFF_PRIOPLAY) &&
    LANEC_LM_XCHTOEND == LANEC_LM_WITCHONE + OFF_XCHTOEND && LANEC_CM_XCHTOEND == LANEC_CM_WITCHONE + OFF_XCHTOEND &&
    LANEC_LM_XCHNYLOC == LANEC_LM_XCHTOEND + 1u && LANEC_CM_XCHNYLOC == LANEC_CM_XCHTOEND + 1u &&
    LANEC_LM_RELINTQ == LANEC_LM_WITCHONE + OFF_RELINTQ && LANEC_CM_RELINTQ == LANEC_CM_WITCHONE + OFF_RELINTQ &&
    LANEC_LM_MAKEPRIO_BANK == 010u && LANEC_CM_MAKEPRIO_BANK == 010u &&
    SAME(MPAC) && SAME(FLAGWRD4) && SAME(LOC) && SAME(LOCCTR) && SAME(CADRFLSH) && SAME(CADRSTOR) &&
    SAME(DSPLOCK) && SAME(MODE) && SAME(RESTREG) && SAME(TIME1) && SAME(PRIORITY) && SAME(PRIO37_WORD) &&
    SAME(PRIO33_WORD) && SAME(BITS15_P_7_WORD) && SAME(OCT20100_WORD) && SAME(OCT40400_WORD) &&
    SAME(OCT10200_WORD) && SAME(NBUSMASK_WORD) && SAME(PMMASK_WORD) && SAME(NVBUSMSK_WORD) &&
    SAME(BIT4_WORD) && SAME(BIT5_WORD) && SAME(BIT14_WORD) && SAME(TWO_WORD) && SAME(ONE_WORD)) ? 1 : -1];

#define MPAC LANEC_LM_MPAC
#define PLAYTEM3 (MPAC + 3u)
#define PLAYTEM4 (MPAC + 4u)
#define FLAGWRD4 LANEC_LM_FLAGWRD4
#define COPINDEX LANEC_LM_LOC
#define LOC LANEC_LM_LOC
#define LOCCTR LANEC_LM_LOCCTR
#define CADRFLSH LANEC_LM_CADRFLSH
#define CADRSTOR LANEC_LM_CADRSTOR
#define DSPLOCK LANEC_LM_DSPLOCK
#define USERPRIO LANEC_LM_MODE
#define RESTREG LANEC_LM_RESTREG
#define TIME1 LANEC_LM_TIME1
#define PRIORITY LANEC_LM_PRIORITY
#define PRIO37 LANEC_LM_PRIO37_WORD
#define PRIO33 LANEC_LM_PRIO33_WORD
#define BITS15P7 LANEC_LM_BITS15_P_7_WORD
#define OCT20100 LANEC_LM_OCT20100_WORD
#define OCT40400 LANEC_LM_OCT40400_WORD
#define OCT10200 LANEC_LM_OCT10200_WORD
#define NBUSMASK LANEC_LM_NBUSMASK_WORD
#define PMMASK LANEC_LM_PMMASK_WORD
#define NVBUSMSK LANEC_LM_NVBUSMSK_WORD
#define BIT4 LANEC_LM_BIT4_WORD
#define BIT5 LANEC_LM_BIT5_WORD
#define BIT14 LANEC_LM_BIT14_WORD
#define TWO LANEC_LM_TWO_WORD
#define ONE LANEC_LM_ONE_WORD
#define DSALMOUT 011u
#define LOCCTR_MAX (01376u - LOC)
#define COPINDEX_MAX 0300u

static const places *at(enum lanec_mission m) { return m == LANEC_LM ? &lm : &cm; }
static void set_a(apollo_overlay_state *s, uint16_t word) { s->words[LANEC_A] = lanec_a16(word); }
static uint16_t corrected(uint16_t a) { return (uint16_t)((a & 037777u) | ((a >> 1) & 040000u)); }
static uint8_t zero16(uint16_t a) { return (uint8_t)(a == 0 || a == 0177777u); }

/* PRIOPLAY: TS COPINDEX, TCF GOPLAY (an overflow skips to PRIOPLAY +2). */
static void prioplay(apollo_overlay_state *s, const places *p)
{
    s->words[LANEC_Z] = lanec_ts(s, COPINDEX) ? (uint16_t)(p->makeprio + OFF_PRIOPLAY + 2u) : p->goplay;
}

/* XCHTOEND: CAF ENDINST; XCHNYLOC: XCH LOCCTR, EXTEND, BZMF RELINTQ,
 * XCH LOCCTR, INDEX LOCCTR, TS LOC; RELINTQ: RELINT, TC Q. */
static void xchnyloc(apollo_overlay_state *s, lanec_io *io, const places *p)
{
    uint16_t *w = s->words, found = w[LOCCTR];
    w[LOCCTR] = corrected(w[LANEC_A]);
    set_a(s, found);
    if (!zero16(w[LANEC_A]) && !(w[LANEC_A] & 0100000u)) {
        uint16_t endinst = w[LOCCTR];
        w[LOCCTR] = found;
        set_a(s, endinst);
        w[LOC + found] = endinst;
    }
    io->interrupt_enabled = 1;
    lanec_di_tc_register(s, LANEC_Q, (uint16_t)(p->witchone + OFF_RELINTQ + 2u));
}

static uint8_t locctr_ok(const apollo_overlay_state *s)
{
    uint16_t v = s->words[LOCCTR];
    return (uint8_t)((v & 040000u) || v <= LOCCTR_MAX);
}

static void xchtoend(apollo_overlay_state *s, lanec_io *io, const places *p, uint16_t back)
{
    lanec_tc(s, (uint16_t)(p->witchone + OFF_XCHTOEND), back);
    set_a(s, p->endinst);
    xchnyloc(s, io, p);
}

/* WITCHONE: CS BIT5, EXTEND, WAND DSALMOUT, CA FLAGWRD4, MASK NVBUSMSK,
 * CCS A, CAF ONE, TS L, CAF ZERO, INDEX L, XCH CADRSTOR, INHINT, TC Q. */
static void witchone(apollo_overlay_state *s, lanec_io *io, const places *p, uint16_t back)
{
    uint16_t *w = s->words, v, k;
    lanec_tc(s, p->witchone, back);
    v = (uint16_t)(corrected((uint16_t)~lanec_a16(BIT5)) & io->channel[DSALMOUT]);
    io->channel[DSALMOUT] = v;                        /* the KEY REL light goes out */
    set_a(s, w[FLAGWRD4]);
    lanec_mask(s, NVBUSMSK);
    w[LANEC_L] = lanec_ccs_a(s) == 0 ? 1u : 0u;       /* NVSUB asleep: CADRSTOR +1 */
    k = (uint16_t)(CADRSTOR + w[LANEC_L]);
    set_a(s, w[k]);
    w[k] = 0;
    io->interrupt_enabled = 0;
    lanec_di_tc_register(s, LANEC_Q, (uint16_t)(p->witchone + 13u));
}

/* LINUSCHR: CS PLAYTEM4, MASK BIT14, CCS A, TCF Q+1, CS PLAYTEM3, INDEX
 * COPINDEX, AD CADRFLSH, EXTEND, BZF +2, TC Q, CCS DSPLOCK, TC ENDOFJOB,
 * TC Q. Returns 1 when control is back at the caller's TC Q return. */
static uint8_t linuschr(apollo_overlay_state *s, const places *p, uint16_t back)
{
    uint16_t *w = s->words;
    lanec_tc(s, p->linuschr, back);
    w[LANEC_A] = (uint16_t)~lanec_a16(w[PLAYTEM4]);
    lanec_mask(s, BIT14);
    if (lanec_ccs_a(s) == 0) { w[LANEC_Z] = p->qplus1; return 0; }          /* not a LINUS */
    w[LANEC_A] = (uint16_t)~lanec_a16(w[PLAYTEM3]);
    lanec_ad(s, w[CADRFLSH + w[COPINDEX]]);
    if (!zero16(w[LANEC_A])) { lanec_di_tc_register(s, LANEC_Q, (uint16_t)(p->linuschr + 10u)); return 1; }
    if (lanec_ccs(s, w[DSPLOCK]) == 0) { lanec_tc(s, p->endofjob, (uint16_t)(p->linuschr + 12u)); return 0; }
    lanec_di_tc_register(s, LANEC_Q, (uint16_t)(p->linuschr + 13u));
    return 1;
}

static void jobxchs(apollo_overlay_state *s, const places *p, uint16_t a)
{
    set_a(s, a);
    s->words[LANEC_Z] = p->jobxchs;
}

static void redoprio(apollo_overlay_state *s, const places *p)
{
    uint16_t *w = s->words;
    set_a(s, w[TIME1]);                               /* CA TIME1, TS PRIOTIME */
    w[p->priotime] = w[TIME1];
    w[LANEC_A] = 0;                                   /* KEEPPRIO: CAF ZERO, TCF PRIOPLAY */
    prioplay(s, p);
}

static void oktocopy(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, const places *p)
{
    uint16_t x = p->makeprio + OFF_OKTOCOPY;
    lanec_tc(s, p->copynorm, (uint16_t)(x + 1u));
    s->words[LANEC_A] = 0;                            /* COPYNORM: CAF ZERO */
    lanec_di_copies(s, io, m);
    witchone(s, io, p, (uint16_t)(x + 2u));
    lanec_tc(s, p->jobwake, (uint16_t)(x + 3u));
}

/* ASKIFNRM: CA FLAGWRD4, MASK OCT10200, EXTEND, BZF OKTOCOPY; SETNORM. */
static void askifnrm(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, const places *p)
{
    set_a(s, s->words[FLAGWRD4]);
    lanec_mask(s, OCT10200);
    if (zero16(s->words[LANEC_A])) oktocopy(s, io, m, p);
    else jobxchs(s, p, ONE);
}

/* HIPRIO: CA FLAGWRD4, MASK OCT40400, EXTEND, BZF ASKIFNRM; SETMARK. */
static void hiprio(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, const places *p)
{
    set_a(s, s->words[FLAGWRD4]);
    lanec_mask(s, OCT40400);
    if (zero16(s->words[LANEC_A])) askifnrm(s, io, m, p);
    else jobxchs(s, p, 0);
}

static void prioabort(apollo_overlay_state *s, const places *p)
{
    lanec_tc(s, p->poodoo, (uint16_t)(p->makeprio + OFF_PRIOBORT + 1u));   /* TC POODOO, OCT 1502 */
}

/* MAKEPRIO +4: CA FLAGWRD4, MASK OCT20100, CCS A, TCF PRIOBORT; HIPRIO. */
static void makeprio4(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, const places *p)
{
    set_a(s, s->words[FLAGWRD4]);
    lanec_mask(s, OCT20100);
    if (lanec_ccs_a(s) == 0) prioabort(s, p);
    else hiprio(s, io, m, p);
}

static void makeprio(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, const places *p)
{
    s->words[LANEC_A] = 0;                            /* CAF ZERO, TS COPINDEX */
    s->words[COPINDEX] = 0;
    if (linuschr(s, p, (uint16_t)(p->makeprio + 3u))) hiprio(s, io, m, p);   /* TCF HIPRIO */
}

/* OKTOPLAY: TC COPIES2, CA USERPRIO, EXTEND, ROR SUPERBNK, TS RESTREG,
 * CA FLAGWRD4, MASK PMMASK, CCS A, TCF GOSLEEPS, TCF +2, TCF GOSLEEPS,
 * TC WITCHONE, TC JOBWAKE. */
static void oktoplay(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, const places *p)
{
    uint16_t *w = s->words, x = p->makeprio + OFF_OKTOPLAY;
    lanec_tc(s, p->copies2, (uint16_t)(x + 1u));
    lanec_di_copies2(s, io, m);
    set_a(s, w[USERPRIO]);
    set_a(s, (uint16_t)(corrected(w[LANEC_A]) | io->superbank));
    w[RESTREG] = corrected(w[LANEC_A]);
    set_a(s, w[FLAGWRD4]);
    lanec_mask(s, PMMASK);
    if (lanec_ccs_a(s) != 1u) { w[LANEC_Z] = p->gosleeps; return; }   /* a mark or prio is going */
    witchone(s, io, p, (uint16_t)(x + 12u));
    lanec_tc(s, p->jobwake, (uint16_t)(x + 13u));
}

/* IFLEGAL +4: CS EBANKTEM, MASK BIT4, CCS A, TCF OKTOPLAY, CA FLAGWRD4,
 * MASK NBUSMASK, EXTEND, BZF OKTOPLAY; PRIOBORT. */
static void iflegal4(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, const places *p)
{
    uint16_t *w = s->words;
    w[LANEC_A] = (uint16_t)~lanec_a16(w[p->ebanktem]);
    lanec_mask(s, BIT4);
    if (lanec_ccs_a(s) == 0) { oktoplay(s, io, m, p); return; }
    set_a(s, w[FLAGWRD4]);
    lanec_mask(s, NBUSMASK);
    if (zero16(w[LANEC_A])) oktoplay(s, io, m, p);
    else prioabort(s, p);
}

static void iflegal(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, const places *p)
{
    set_a(s, TWO);                                    /* CAF TWO, TS COPINDEX */
    s->words[COPINDEX] = TWO;
    if (linuschr(s, p, (uint16_t)(p->makeprio + OFF_IFLEGAL + 3u))) oktoplay(s, io, m, p);
}

/* MAKEPLAY +5: CA PLAYTEM4, MASK BITS15+7, CCS A, TCF MAKEPRIO, TCF IFLEGAL,
 * TCF MAKEMARK. MAKEMARK's exits into SETNORM and PRIOPLAY continue here. */
static void makeplay5(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, const places *p)
{
    uint8_t k;
    set_a(s, s->words[PLAYTEM4]);
    lanec_mask(s, BITS15P7);
    k = lanec_ccs_a(s);
    if (k == 0) { makeprio(s, io, m, p); return; }
    if (k == 1) { iflegal(s, io, m, p); return; }
    s->words[LANEC_Z] = p->makemark;
    lanec_di_makemark(s, io, m);
    if (s->words[LANEC_Z] == p->makeprio + OFF_SETNORM) jobxchs(s, p, ONE);
    else if (s->words[LANEC_Z] == p->makeprio + OFF_PRIOPLAY) prioplay(s, p);
}

void lanec_di_witchone(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, uint16_t back)
{
    witchone(s, io, at(m), back);
}

void lanec_di_xchnyloc(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    xchnyloc(s, io, at(m));
}

/* A caller's return run by TC Q: a TC to fixed memory outside the bank 10
 * code this suite translates (MARKEND..RELINTQ +1 and LINUSCHR). */
static uint8_t caller_ok(const apollo_overlay_state *s, lanec_io *io, const places *p)
{
    uint16_t q = s->words[LANEC_Q];
    if (q < 02000u || q > 07777u) return 0;
    if (q >= 04000u || lanec_current_bank(s, io) != 010u) return 1;
    if (q >= p->linuschr && q <= p->linuschr + 12u) return 0;
    return (uint8_t)(q < p->markend || q >= p->witchone + OFF_RELINTQ + 2u);
}

enum lanec_status lanec_di_play_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words, z, x;
    uint8_t linus_ok = (uint8_t)(w[DSPLOCK] < 040000u);
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 010u) return LANEC_OUT_OF_SCOPE;
    z = w[LANEC_Z]; x = p->makeprio;
    if (z == x + OFF_MAKEPLAY) {
        set_a(s, w[PRIORITY]);                        /* CA PRIORITY, MASK PRIO37, TS USERPRIO */
        lanec_mask(s, PRIO37);
        w[USERPRIO] = corrected(w[LANEC_A]);
        set_a(s, PRIO33);                             /* CAF PRIO33, TC PRIOCHNG */
        lanec_tc(s, p->priochng, (uint16_t)(x + OFF_MAKEPLAY + 5u));
        return LANEC_OK;
    }
    if (z == x + OFF_MAKEPLAY + 5u) { if (!linus_ok) return LANEC_OUT_OF_SCOPE; makeplay5(s, io, m, p); return LANEC_OK; }
    if (z == x) { if (!linus_ok) return LANEC_OUT_OF_SCOPE; makeprio(s, io, m, p); return LANEC_OK; }
    if (z == x + 4u) { makeprio4(s, io, m, p); return LANEC_OK; }
    if (z == x + OFF_HIPRIO) { hiprio(s, io, m, p); return LANEC_OK; }
    if (z == x + OFF_SETMARK) { jobxchs(s, p, 0); return LANEC_OK; }
    if (z == x + OFF_ASKIFNRM) { askifnrm(s, io, m, p); return LANEC_OK; }
    if (z == x + OFF_SETNORM) { jobxchs(s, p, ONE); return LANEC_OK; }
    if (z == x + OFF_OKTOCOPY) { oktocopy(s, io, m, p); return LANEC_OK; }
    if (z == x + OFF_OKTOCOPY + 3u) {
        if (!locctr_ok(s)) return LANEC_OUT_OF_SCOPE;
        xchtoend(s, io, p, (uint16_t)(x + OFF_REDOPRIO));
        redoprio(s, p);
        return LANEC_OK;
    }
    if (z == x + OFF_REDOPRIO) { redoprio(s, p); return LANEC_OK; }
    if (z == x + OFF_KEEPPRIO) { w[LANEC_A] = 0; prioplay(s, p); return LANEC_OK; }
    if (z == x + OFF_IFLEGAL) { if (!linus_ok) return LANEC_OUT_OF_SCOPE; iflegal(s, io, m, p); return LANEC_OK; }
    if (z == x + OFF_IFLEGAL + 4u) { iflegal4(s, io, m, p); return LANEC_OK; }
    if (z == x + OFF_PRIOBORT) { prioabort(s, p); return LANEC_OK; }
    if (z == x + OFF_OKTOPLAY) {
        if (w[COPINDEX] > COPINDEX_MAX) return LANEC_OUT_OF_SCOPE;
        oktoplay(s, io, m, p);
        return LANEC_OK;
    }
    if (z == x + OFF_OKTOPLAY + 13u) {
        if (!locctr_ok(s)) return LANEC_OUT_OF_SCOPE;
        xchtoend(s, io, p, (uint16_t)(x + OFF_PLAYJUM1));
        set_a(s, TWO);
        prioplay(s, p);
        return LANEC_OK;
    }
    if (z == x + OFF_PLAYJUM1) { set_a(s, TWO); prioplay(s, p); return LANEC_OK; }
    if (z == x + OFF_PRIOPLAY) { prioplay(s, p); return LANEC_OK; }
    if (!caller_ok(s, io, p)) return LANEC_OUT_OF_SCOPE;
    if (z == p->witchone) { witchone(s, io, p, w[LANEC_Q]); return LANEC_OK; }
    if (z == p->witchone + OFF_XCHTOEND || z == p->witchone + OFF_XCHTOEND + 1u) {
        if (!locctr_ok(s)) return LANEC_OUT_OF_SCOPE;
        if (z == p->witchone + OFF_XCHTOEND) set_a(s, p->endinst);
        xchnyloc(s, io, p);
        return LANEC_OK;
    }
    if (z == p->linuschr) {
        if (!linus_ok || w[COPINDEX] > COPINDEX_MAX) return LANEC_OUT_OF_SCOPE;
        (void)linuschr(s, p, w[LANEC_Q]);
        return LANEC_OK;
    }
    return LANEC_OUT_OF_SCOPE;
}
