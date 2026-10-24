/* SPDX-License-Identifier: MIT */
/* Pinball verb 07 (DSPDPDEC/ENDDPDEC) and channel-noun load (CHANLOAD),
 * Luminary099 and Comanche055. See docs/LANEC-PIN-GAPS.md. */
#include "lanec_pin_gaps.h"
#include "lanec_symbols.h"
#include "lanec_ev_internal.h"
#include "agc_multiword.h"

typedef struct { uint16_t dspdpdec, dspalarm, tpagree, dsp2dec, chanload, loadlv, dspin11, dspsign, dspdcwd1, nvsub1, almcycle, lodsampt, flashon, flashoff, dspin, govnupdt, dspoctin, dspdecvn, postjump, bankjump, pluson, minuson; } places;
static const places lm = { LANEC_LM_DSPDPDEC, LANEC_LM_DSPALARM, LANEC_LM_TPAGREE, LANEC_LM_DSP2DEC,
                           LANEC_LM_CHANLOAD, LANEC_LM_LOADLV, LANEC_LM_11DSPIN, LANEC_LM_DSPSIGN,
                           LANEC_LM_DSPDCWD1, LANEC_LM_NVSUB1, LANEC_LM_ALMCYCLE,
                           LANEC_LM_LODSAMPT, LANEC_LM_FLASHON, LANEC_LM_FLASHOFF, LANEC_LM_DSPIN,
                           LANEC_LM_GOVNUPDT, LANEC_LM_DSPOCTIN, LANEC_LM_DSPDECVN, LANEC_LM_POSTJUMP, LANEC_LM_BANKJUMP,
                           LANEC_LM__P_ON, LANEC_LM__M_ON };
static const places cm = { LANEC_CM_DSPDPDEC, LANEC_CM_DSPALARM, LANEC_CM_TPAGREE, LANEC_CM_DSP2DEC,
                           LANEC_CM_CHANLOAD, LANEC_CM_LOADLV, LANEC_CM_11DSPIN, LANEC_CM_DSPSIGN,
                           LANEC_CM_DSPDCWD1, LANEC_CM_NVSUB1, LANEC_CM_ALMCYCLE,
                           LANEC_CM_LODSAMPT, LANEC_CM_FLASHON, LANEC_CM_FLASHOFF, LANEC_CM_DSPIN,
                           LANEC_CM_GOVNUPDT, LANEC_CM_DSPOCTIN, LANEC_CM_DSPDECVN, LANEC_CM_POSTJUMP, LANEC_CM_BANKJUMP,
                           LANEC_CM__P_ON, LANEC_CM__M_ON };
typedef char same_erasable[(LANEC_LM_MIXBR == LANEC_CM_MIXBR && LANEC_LM_NOUNADD == LANEC_CM_NOUNADD &&
    LANEC_LM_NOUNCADR == LANEC_CM_NOUNCADR && LANEC_LM_DSPCOUNT == LANEC_CM_DSPCOUNT &&
    LANEC_LM_ENTEXIT == LANEC_CM_ENTEXIT && LANEC_LM_MPAC == LANEC_CM_MPAC &&
    LANEC_LM_DSPDPDEC_BANK == 040u && LANEC_CM_DSPDPDEC_BANK == 040u &&
    LANEC_LM_CHANLOAD_BANK == 041u && LANEC_CM_CHANLOAD_BANK == 041u &&
    LANEC_LM_ENDDPDEC == LANEC_LM_DSPDPDEC + 14u && LANEC_CM_ENDDPDEC == LANEC_CM_DSPDPDEC + 14u &&
    LANEC_LM_END2DEC == LANEC_LM_DSP2DEC + 10u && LANEC_CM_END2DEC == LANEC_CM_DSP2DEC + 10u &&
    LANEC_LM_WDRET == LANEC_CM_WDRET && LANEC_LM_CODE == LANEC_CM_CODE &&
    LANEC_LM_WDCNT == LANEC_CM_WDCNT && LANEC_LM_COUNT == LANEC_CM_COUNT &&
    LANEC_LM_DSPDCWD1_BANK == 040u && LANEC_CM_DSPDCWD1_BANK == 040u && LANEC_LM_DSPIN_BANK == 040u && LANEC_CM_DSPIN_BANK == 040u &&
    LANEC_LM_DSP2DEC_BANK == 040u && LANEC_CM_DSP2DEC_BANK == 040u &&
    LANEC_LM_NVSUBB_BANK == 041u && LANEC_CM_NVSUBB_BANK == 041u &&
    LANEC_LM_LOADLV1 == LANEC_LM_NVSUBB + 1u && LANEC_CM_LOADLV1 == LANEC_CM_NVSUBB + 1u &&
    LANEC_LM_NVSUBB == LANEC_CM_NVSUBB && LANEC_LM_GOALMCYC == LANEC_CM_GOALMCYC &&
    LANEC_LM_GOALMCYC_BANK == 041u && LANEC_CM_GOALMCYC_BANK == 041u &&
    LANEC_LM_FULLDSP == LANEC_CM_FULLDSP && LANEC_LM_TSTLTS3 == LANEC_CM_TSTLTS3 &&
    LANEC_LM_FULLDSP_BANK == 041u && LANEC_CM_FULLDSP_BANK == 041u && LANEC_LM_TIME2 == LANEC_CM_TIME2 &&
    LANEC_LM_SAMPTIME == LANEC_CM_SAMPTIME &&
    LANEC_LM_GOVNUPDT_BANK == 040u && LANEC_CM_GOVNUPDT_BANK == 040u && LANEC_LM_DSPOCTIN_BANK == 040u &&
    LANEC_CM_DSPOCTIN_BANK == 040u && LANEC_LM_DSPDECVN_BANK == 040u && LANEC_CM_DSPDECVN_BANK == 040u &&
    LANEC_LM_OCTBACK == LANEC_CM_OCTBACK && LANEC_LM_OCTBACK_BANK == 041u && LANEC_CM_OCTBACK_BANK == 041u &&
    LANEC_LM_WDAGAIN == LANEC_CM_WDAGAIN && LANEC_LM_OCTBACK == LANEC_LM_WDAGAIN + 16u &&
    LANEC_LM_UPDAT1 == LANEC_CM_UPDAT1 && LANEC_LM_UPDAT1_BANK == 041u && LANEC_CM_UPDAT1_BANK == 041u &&
    LANEC_LM_UPDATRET == LANEC_CM_UPDATRET && LANEC_LM_DSPWDRET == LANEC_CM_DSPWDRET &&
    LANEC_LM_DSPSIGN_BANK == 040u && LANEC_CM_DSPSIGN_BANK == 040u && LANEC_LM__P_ON_BANK == 040u &&
    LANEC_CM__P_ON_BANK == 040u && LANEC_LM__M_ON_BANK == 040u && LANEC_CM__M_ON_BANK == 040u) ? 1 : -1];
#define MIXBR LANEC_LM_MIXBR
#define NOUNADD LANEC_LM_NOUNADD
#define NOUNCADR LANEC_LM_NOUNCADR
#define DSPCOUNT LANEC_LM_DSPCOUNT
#define ENTEXIT LANEC_LM_ENTEXIT
#define MPAC LANEC_LM_MPAC
#define WDRET LANEC_LM_WDRET
#define CODE LANEC_LM_CODE
#define WDCNT LANEC_LM_WDCNT
#define COUNT LANEC_LM_COUNT
#define MPTEMP 0135u
#define VD1 023u
#define NVSUBB LANEC_LM_NVSUBB
#define GOALMCYC LANEC_LM_GOALMCYC
#define WAITEND (LANEC_LM_FULLDSP - 1u)     /* TC ENDOFJOB after TSTLTS1's WAITLIST */
#define NOVACEND (LANEC_LM_TSTLTS3 - 1u)    /* TC TASKOVER after TSTLTS2's NOVAC */
#define TIME2 LANEC_LM_TIME2
#define SAMPTIME LANEC_LM_SAMPTIME
#define BIT6 040u
#define DSALMOUT 011u
#define R2D1 011u
#define R1D1 016u
#define SEVEN 7u
#define LOW9 0777u
#define CYL 022u
#define OCTBACK LANEC_LM_OCTBACK
#define UPDATRET LANEC_LM_UPDATRET
#define UPDAT2 (LANEC_LM_UPDAT1 + 2u)
#define DSPWDRET LANEC_LM_DSPWDRET
#define ENDSPOCT 063411u                    /* CADR OCTBACK (bank 41 under the superbank) */

static const places *at(enum lanec_mission m) { return m == LANEC_LM ? &lm : &cm; }

/* DSPDPDEC +0 INDEX MIXBR, +1 TC +0, +2 TC +2, +3 TC DSPALARM, +4 EXTEND,
 * +5 INDEX NOUNADD, +6 DCA 0, +7 DXCH MPAC, +8 CAF R1D1, +9 TS DSPCOUNT,
 * +10 CAF ZERO, +11 TS MPAC +2, +12 TC TPAGREE, +13 TC DSP2DEC,
 * +14 ENDDPDEC TC ENTEXIT. */
enum lanec_status lanec_pin_dspdpdec(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words, x = p->dspdpdec, z = w[LANEC_Z], k, hi, lo, old_hi, old_lo;
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 040u) return LANEC_OUT_OF_SCOPE;
    if (z == x + 13u) { lanec_tc(s, p->dsp2dec, (uint16_t)(x + 14u)); return LANEC_OK; }
    if (z == x + 14u) { lanec_tc(s, ENTEXIT, (uint16_t)(x + 15u)); return LANEC_OK; }
    k = w[NOUNADD];
    if (z != x || (w[MIXBR] != 1u && w[MIXBR] != 2u)) return LANEC_OUT_OF_SCOPE;
    if (w[MIXBR] == 2u) {                                   /* mixed noun: TC +0 indexed to TC DSPALARM */
        lanec_tc(s, (uint16_t)(x + 3u), (uint16_t)(x + 2u));    /* (NOUNADD is not read) */
        lanec_tc(s, p->dspalarm, (uint16_t)(x + 4u));
        return LANEC_OK;
    }
    if (k < 010u || k == 01377u || k > 01776u) return LANEC_OUT_OF_SCOPE;
    lanec_tc(s, (uint16_t)(x + 2u), (uint16_t)(x + 2u));    /* normal noun: TC +0 indexed to TC +2 */
    lanec_tc(s, (uint16_t)(x + 4u), (uint16_t)(x + 3u));
    lo = lanec_erasable(s, (uint16_t)(k + 1u));            /* DCA: L first */
    lo = lanec_a16((uint16_t)(w[lo] & 077777u));           /* a register reads its low 15 bits */
    hi = lanec_erasable(s, k);                              /* then A, which reads the new L */
    hi = lanec_a16((uint16_t)((hi == 1u ? lo : w[hi]) & 077777u));
    lanec_rewrite(s, (uint16_t)(k + 1u)); lanec_rewrite(s, k);
    old_hi = w[MPAC]; old_lo = w[MPAC + 1u];                /* DXCH MPAC (A and L from a */
    w[MPAC] = lanec_ev_corrected(hi); w[MPAC + 1u] = lanec_ev_corrected(lo);   /* 16-bit register store corrected) */
    w[LANEC_A] = lanec_a16(old_hi); w[LANEC_L] = lanec_a16(old_lo);
    w[DSPCOUNT] = R1D1;                                     /* CAF R1D1, TS DSPCOUNT */
    w[LANEC_A] = 0; w[MPAC + 2u] = 0;                       /* CAF ZERO, TS MPAC +2 */
    lanec_tc(s, p->tpagree, (uint16_t)(x + 13u));
    return LANEC_OK;
}

/* CHANLOAD +0 CS SEVEN, +1 AD NOUNCADR, +2 EXTEND, +3 BZF LOADLV, +4 CA NOUNCADR,
 * +5 MASK LOW9, +6 XCH MPAC, +7 EXTEND, +8 INDEX MPAC, +9 WRITE 0, +10 TC LOADLV. */
enum lanec_status lanec_pin_chanload(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words, y = p->chanload, channel, old;
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 041u || w[LANEC_Z] != y) return LANEC_OUT_OF_SCOPE;
    channel = (uint16_t)(w[NOUNCADR] & LOW9);
    if (channel > 077u || (channel == 7u && w[NOUNCADR] != SEVEN)) return LANEC_OUT_OF_SCOPE;
    w[LANEC_A] = (uint16_t)~SEVEN;
    lanec_ad(s, w[NOUNCADR]);
    if (w[LANEC_A] == 0 || w[LANEC_A] == 0177777u) { w[LANEC_Z] = p->loadlv; return LANEC_OK; }
    w[LANEC_A] = lanec_a16(w[NOUNCADR]);                    /* CA NOUNCADR */
    lanec_mask(s, LOW9);
    old = w[MPAC];                                          /* XCH MPAC */
    w[MPAC] = (uint16_t)((w[LANEC_A] & 037777u) | ((w[LANEC_A] >> 1) & 040000u));
    w[LANEC_A] = lanec_a16(old);
    lanec_write(s, io, channel);                            /* INDEX MPAC, WRITE 0 */
    lanec_tc(s, p->loadlv, (uint16_t)(y + 11u));
    return LANEC_OK;
}

/* DSP2DEC +0 XCH Q, +1 TS WDRET, +2 CAF ZERO, +3 TS CODE, +4 CAF THREE,
 * +5 TC 11DSPIN, +6 CAF FOUR, +7 TC 11DSPIN, +8 TC DSPSIGN, +9 CAF R2D1,
 * +10 END2DEC TC DSPDCWD1. Entries: +0, and +6, +8, +9 where 11DSPIN and
 * DSPSIGN return, and END2DEC. */
enum lanec_status lanec_pin_dsp2dec(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words, x = p->dsp2dec, z = w[LANEC_Z], a;
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 040u || z < x || z > x + 10u) return LANEC_OUT_OF_SCOPE;
    switch (z - x) {
    case 0:
        a = w[LANEC_A]; w[LANEC_A] = w[LANEC_Q]; w[LANEC_Q] = a;     /* XCH Q */
        if (!lanec_ts(s, WDRET)) w[LANEC_A] = 0;                   /* TS WDRET (overflow skips CAF ZERO) */
        (void)lanec_ts(s, CODE);
        w[LANEC_A] = 3u;                                            /* CAF THREE */
        lanec_tc(s, p->dspin11, (uint16_t)(x + 6u));
        return LANEC_OK;
    case 6: w[LANEC_A] = 4u; lanec_tc(s, p->dspin11, (uint16_t)(x + 8u)); return LANEC_OK;
    case 8: lanec_tc(s, p->dspsign, (uint16_t)(x + 9u)); return LANEC_OK;
    case 9: w[LANEC_A] = R2D1; lanec_tc(s, p->dspdcwd1, (uint16_t)(x + 11u)); return LANEC_OK;
    case 10: lanec_tc(s, p->dspdcwd1, (uint16_t)(x + 11u)); return LANEC_OK;
    default: return LANEC_OUT_OF_SCOPE;
    }
}

/* SHORTMP (CAF BINCON first): MPAC's triple word times ten, A and L the
 * high product, as the decimal display modules run it. w holds words 0-0156. */
static void accumulate(agc_register_pair *r, uint16_t *w, uint16_t a)
{
    agc_dp d;
    d.high = w[a]; d.low = w[a + 1u];
    agc_double_add(r, &d);
    w[a] = d.high; w[a + 1u] = d.low;
}
static void shortmp(uint16_t *w)
{
    agc_register_pair r;
    agc_word minor = w[MPAC + 1u], major = w[MPAC];
    w[MPTEMP] = 10u;
    r = agc_multiply(10u, w[MPAC + 2u]); w[MPAC + 2u] = agc_correct(r.a); w[MPAC + 1u] = 0;
    r = agc_multiply(agc_extend(minor), 10u); accumulate(&r, w, MPAC + 1u); w[MPAC] = agc_correct(r.a);
    r = agc_multiply(agc_extend(major), 10u); accumulate(&r, w, MPAC);
    w[LANEC_A] = r.a; w[LANEC_L] = r.l;
}

/* DSPDCWD1 -1 CAF FOUR, +0 TS WDCNT, CAF BINCON, TC SHORTMP, +3 TRACE1
 * INDEX MPAC, CAF RELTAB, MASK LOW5, TS CODE, CAF ZERO, XCH MPAC +2,
 * XCH MPAC +1, TS MPAC, XCH DSPCOUNT, +12 TRACE1S TS COUNT, CCS A,
 * TS DSPCOUNT, +15 TC DSPIN, +16 CCS WDCNT, TC DSPDCWD1, CS VD1,
 * TS DSPCOUNT, +20 TC WDRET. Entries: -1, +0, and +16 where DSPIN returns.
 * MPAC after SHORTMP must index RELTAB's ten digits (or be -0). */
enum lanec_status lanec_pin_dspdcwd1(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    static const uint16_t relay[10] = { 025u, 003u, 031u, 033u, 017u, 036u, 034u, 023u, 035u, 037u };
    const places *p = at(m);
    uint16_t *w = s->words, x = p->dspdcwd1, z = w[LANEC_Z], a, t[MPAC + 3u], i;
    uint8_t skip;
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 040u || (z != x - 1u && z != x && z != x + 16u))
        return LANEC_OUT_OF_SCOPE;
    if (z != x + 16u) {
        a = z == x - 1u ? 4u : w[LANEC_A];                          /* CAF FOUR */
        if (((a >> 15) ^ (a >> 14)) & 1u) return LANEC_OUT_OF_SCOPE;  /* TS WDCNT would skip */
        for (i = 0; i < MPAC + 3u; ++i) t[i] = w[i];
        shortmp(t);
        if (t[MPAC] > 9u && t[MPAC] != 077777u) return LANEC_OUT_OF_SCOPE;
        w[LANEC_A] = a; (void)lanec_ts(s, WDCNT);                   /* TS WDCNT */
        w[MPTEMP] = t[MPTEMP]; w[MPAC] = t[MPAC]; w[MPAC + 1u] = t[MPAC + 1u]; w[MPAC + 2u] = t[MPAC + 2u];
        w[LANEC_L] = t[LANEC_L];
        w[CODE] = relay[t[MPAC] == 077777u ? 0u : t[MPAC]];          /* INDEX MPAC, CAF RELTAB, MASK LOW5, TS CODE */
        a = w[MPAC + 2u]; w[MPAC + 2u] = 0;                            /* CAF ZERO, XCH MPAC +2 */
        i = w[MPAC + 1u]; w[MPAC + 1u] = a;                            /* XCH MPAC +1 */
        w[MPAC] = i;                                                   /* TS MPAC */
        w[LANEC_A] = lanec_a16(w[DSPCOUNT]); w[DSPCOUNT] = i;          /* XCH DSPCOUNT */
        (void)lanec_ts(s, COUNT);                                      /* TRACE1S TS COUNT */
        skip = lanec_ccs_a(s);                                         /* CCS A */
        if (skip == 0u) (void)lanec_ts(s, DSPCOUNT);
        if (skip <= 1u) { lanec_tc(s, p->dspin, (uint16_t)(x + 16u)); return LANEC_OK; }
        if (skip == 3u) { lanec_tc(s, x, (uint16_t)(x + 18u)); return LANEC_OK; }
    }
    skip = lanec_ccs(s, w[WDCNT]);                                     /* +16 CCS WDCNT */
    if (skip == 0u) { lanec_tc(s, x, (uint16_t)(x + 18u)); return LANEC_OK; }
    if (skip == 1u) w[LANEC_A] = (uint16_t)~lanec_a16(VD1);            /* CS VD1 */
    if (skip <= 2u) (void)lanec_ts(s, DSPCOUNT);
    lanec_tc(s, WDRET, (uint16_t)(x + 21u));
    return LANEC_OK;
}

/* NVSUBB TC NVSUB1, LOADLV1 TC LOADLV and GOALMCYC TC ALMCYCLE (the
 * PUTDCSF2 table's octal-only entry), the bank-41 lead-ins (both missions). */
enum lanec_status lanec_pin_leadin(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t z = s->words[LANEC_Z];
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 041u || (z != NVSUBB && z != NVSUBB + 1u && z != GOALMCYC))
        return LANEC_OUT_OF_SCOPE;
    lanec_tc(s, z == NVSUBB ? p->nvsub1 : z == GOALMCYC ? p->almcycle : p->loadlv, (uint16_t)(z + 1u));
    return LANEC_OK;
}

/* LODSAMPT (EXTEND, DCA TIME2, DXCH SAMPTIME, TC Q), FLASHON (CAF BIT6,
 * EXTEND, WOR DSALMOUT, TC Q) and FLASHOFF (CS BIT6, EXTEND, WAND DSALMOUT,
 * TC Q), the fixed-fixed subroutines other files call (TC Q runs the return
 * address in Q as a TC, which leaves Q = 3); and the bank-41 words
 * where WAITLIST and NOVAC return to the lamp test, TC ENDOFJOB and
 * TC TASKOVER. */
enum lanec_status lanec_pin_small(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words, z = w[LANEC_Z];
    if (!lanec_registers_valid(s)) return LANEC_OUT_OF_SCOPE;
    if ((z == p->lodsampt || z == p->flashon || z == p->flashoff) && (w[LANEC_Q] < 02000u || w[LANEC_Q] > 07777u))
        return LANEC_OUT_OF_SCOPE;    /* TC Q runs Q as a TC to fixed memory */
    if (z == p->lodsampt) {
        w[LANEC_A] = lanec_ev_sx(w[TIME2]); w[LANEC_L] = lanec_ev_sx(w[TIME2 + 1u]);
        lanec_ev_dxch(s, SAMPTIME);
        lanec_tc(s, w[LANEC_Q], 3u);
        return LANEC_OK;
    }
    if (z == p->flashon || z == p->flashoff) {
        if (z == p->flashon) { w[LANEC_A] = BIT6; lanec_ev_wor(s, io, DSALMOUT); }
        else { w[LANEC_A] = (uint16_t)~BIT6; lanec_ev_wand(s, io, DSALMOUT); }
        lanec_tc(s, w[LANEC_Q], 3u);
        return LANEC_OK;
    }
    if ((z == WAITEND || z == NOVACEND) && lanec_current_bank(s, io) == 041u) {
        lanec_tc(s, z == WAITEND ? (m == LANEC_LM ? LANEC_LM_ENDOFJOB : LANEC_CM_ENDOFJOB)
                                 : (m == LANEC_LM ? LANEC_LM_TASKOVER : LANEC_CM_TASKOVER), (uint16_t)(z + 1u));
        return LANEC_OK;
    }
    return LANEC_OUT_OF_SCOPE;
}

/* The words the file reaches through POSTJUMP and BANKJUMP, and their
 * lead-ins. Bank 40: GOVNUPDT TC DSPDECVN, +1 TC POSTJUMP (CADR UPDAT1 +2);
 * DSPOCTIN TC DSPIN, +1 CAF +2, TC BANKJUMP (ENDSPOCT CADR OCTBACK).
 * Bank 41: UPDAT1 +2 TC UPDATRET; OCTBACK CCS WDCNT, TC WDAGAIN, DSPLV CS
 * VD1, TS DSPCOUNT, TC WDRET, where WDAGAIN (OCTBACK -16) is TS WDCNT, CS CYL
 * three times, CS A, MASK DSPMSK, INDEX A, CAF RELTAB, MASK LOW5, TS CODE,
 * XCH DSPCOUNT, TS COUNT, CCS A, TS DSPCOUNT, TC POSTJUMP (CADR DSPOCTIN).
 * WDAGAIN runs inline; its CCS A must find DSPCOUNT positive or +0 (a
 * negative count would run the CADR as an instruction). */
enum lanec_status lanec_pin_jumps(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    static const uint16_t relay[8] = { 025u, 003u, 031u, 033u, 017u, 036u, 034u, 023u };
    const places *p = at(m);
    uint16_t *w = s->words, z = w[LANEC_Z], bank, t;
    uint8_t skip, i;
    if (!lanec_registers_valid(s)) return LANEC_OUT_OF_SCOPE;
    bank = lanec_current_bank(s, io);
    if (bank == 040u && z == p->govnupdt) { lanec_tc(s, p->dspdecvn, (uint16_t)(z + 1u)); return LANEC_OK; }
    if (bank == 040u && z == p->govnupdt + 1u) { lanec_tc(s, p->postjump, (uint16_t)(z + 1u)); return LANEC_OK; }
    if (bank == 040u && z == p->dspoctin) { lanec_tc(s, p->dspin, (uint16_t)(z + 1u)); return LANEC_OK; }
    if (bank == 040u && z == p->dspoctin + 1u) {
        w[LANEC_A] = lanec_a16(ENDSPOCT);                               /* CAF +2 */
        lanec_tc(s, p->bankjump, (uint16_t)(z + 2u));
        return LANEC_OK;
    }
    if (bank == 041u && z == UPDAT2) { lanec_tc(s, UPDATRET, (uint16_t)(z + 1u)); return LANEC_OK; }
    if (bank != 041u || z != OCTBACK) return LANEC_OUT_OF_SCOPE;
    if ((w[WDCNT] & 040000u) == 0 && w[WDCNT] != 0 && (w[DSPCOUNT] & 040000u)) return LANEC_OUT_OF_SCOPE;
    skip = lanec_ccs(s, w[WDCNT]);                                      /* CCS WDCNT */
    if (skip == 0u) {                                                   /* TC WDAGAIN */
        (void)lanec_ts(s, WDCNT);
        for (i = 0; i < 3u; ++i) { w[LANEC_A] = (uint16_t)~lanec_a16(w[CYL]); lanec_rewrite(s, CYL); }
        w[LANEC_A] = (uint16_t)~w[LANEC_A];                             /* CS A */
        lanec_mask(s, SEVEN);
        w[LANEC_A] = relay[w[LANEC_A] & 7u];                            /* INDEX A, CAF RELTAB, MASK LOW5 */
        (void)lanec_ts(s, CODE);
        t = w[DSPCOUNT]; w[DSPCOUNT] = w[LANEC_A]; w[LANEC_A] = lanec_a16(t);   /* XCH DSPCOUNT */
        (void)lanec_ts(s, COUNT);
        if (lanec_ccs_a(s) == 0u) (void)lanec_ts(s, DSPCOUNT);         /* CCS A, TS DSPCOUNT */
        lanec_tc(s, p->postjump, (uint16_t)(OCTBACK - 1u));
        return LANEC_OK;
    }
    if (skip == 1u) w[LANEC_A] = (uint16_t)~lanec_a16(VD1);             /* CS VD1 */
    if (skip <= 2u) (void)lanec_ts(s, DSPCOUNT);
    lanec_tc(s, WDRET, (uint16_t)(OCTBACK + 5u));
    return LANEC_OK;
}

/* DSPSIGN +0 XCH Q, TS DSPWDRET, CCS MPAC, TC +8D, TC +7, AD ONE, TS MPAC,
 * +7 TC -ON, +010 CS MPAC +1, TS MPAC +1, TC DSPWDRET, +013 TC +ON,
 * +014 TC DSPWDRET. Entries: +0, and +010 and +014 where -ON and +ON
 * return. TC DSPWDRET runs the saved return address as a TC, which must
 * reach fixed memory (Q is then DSPWDRET +1). */
enum lanec_status lanec_pin_dspsign(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words, x = p->dspsign, z = w[LANEC_Z], t;
    uint8_t skip;
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 040u || (z != x && z != x + 010u && z != x + 014u))
        return LANEC_OUT_OF_SCOPE;
    if (z == x) {
        t = w[LANEC_Q];
        if (((t >> 15) ^ (t >> 14)) & 1u) return LANEC_OUT_OF_SCOPE;   /* TS DSPWDRET would skip */
        w[LANEC_Q] = w[LANEC_A]; w[LANEC_A] = t;                        /* XCH Q */
        (void)lanec_ts(s, DSPWDRET);
        skip = lanec_ccs(s, w[MPAC]);                                   /* CCS MPAC */
        if (skip <= 1u) { lanec_tc(s, p->pluson, (uint16_t)(x + 014u)); return LANEC_OK; }
        if (skip == 2u) lanec_ad(s, 1u);                                /* AD ONE */
        (void)lanec_ts(s, MPAC);
        lanec_tc(s, p->minuson, (uint16_t)(x + 010u));
        return LANEC_OK;
    }
    if (w[DSPWDRET] < 02000u || w[DSPWDRET] > 07777u) return LANEC_OUT_OF_SCOPE;
    if (z == x + 010u) {
        w[LANEC_A] = (uint16_t)~lanec_a16(w[MPAC + 1u]);                /* CS MPAC +1 */
        (void)lanec_ts(s, MPAC + 1u);
    }
    lanec_tc(s, w[DSPWDRET], DSPWDRET + 1u);                            /* TC DSPWDRET */
    return LANEC_OK;
}

enum lanec_status lanec_pin_gaps_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t z = s->words[LANEC_Z];
    if (z == p->chanload && lanec_current_bank(s, io) == 041u) return lanec_pin_chanload(s, io, m);
    if (((z == p->govnupdt || z == p->govnupdt + 1u || z == p->dspoctin || z == p->dspoctin + 1u) && lanec_current_bank(s, io) == 040u) ||
        ((z == UPDAT2 || z == OCTBACK) && lanec_current_bank(s, io) == 041u))
        return lanec_pin_jumps(s, io, m);
    if ((z == NVSUBB || z == NVSUBB + 1u || z == GOALMCYC) && lanec_current_bank(s, io) == 041u) return lanec_pin_leadin(s, io, m);
    if (z >= p->dsp2dec && z <= p->dsp2dec + 10u && lanec_current_bank(s, io) == 040u) return lanec_pin_dsp2dec(s, io, m);
    if ((z == p->dspdcwd1 - 1u || z == p->dspdcwd1 || z == p->dspdcwd1 + 16u) && lanec_current_bank(s, io) == 040u)
        return lanec_pin_dspdcwd1(s, io, m);
    if ((z == p->dspsign || z == p->dspsign + 010u || z == p->dspsign + 014u) && lanec_current_bank(s, io) == 040u)
        return lanec_pin_dspsign(s, io, m);
    if (z == p->lodsampt || z == p->flashon || z == p->flashoff || ((z == WAITEND || z == NOVACEND) && lanec_current_bank(s, io) == 041u))
        return lanec_pin_small(s, io, m);
    return lanec_pin_dspdpdec(s, io, m);
}
