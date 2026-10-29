/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS short verb bodies in bank 43, Luminary099 and Comanche055:
 * the flag verbs, the job and task starters, the DAP, display and test
 * verbs that only check a condition and hand over to another routine, and
 * SHOWSUM. Each case is one basic block of the source, entered at its
 * address; see docs/LANEC-EV-SHORT.md for the source map. */
#include "lanec_ev_short.h"
#include "lanec_ev_internal.h"
#include "lanec_symbols.h"

#define NOT_ENTRY 0xffffu
#define CALL(T, R) return lanec_ev_call(s, (T), (uint16_t)(R))
#define JUMP(T) return (uint16_t)(T)
#define E(X) lanec_erasable(s, (X))

/* ------------------------------------------------------------------ LM */
#define L(X) LANEC_LM_##X
#define LW(X) LANEC_LM_##X##_WORD

/* Entries: every label and every return point of the LM blocks below. */
static const uint16_t lm_entries[] = {
    L(DAPATTER), L(DAPATTER) + 2u, L(TOTATTER), L(TOTATTER) + 2u, L(LRON), L(LRON) + 2u, L(LROFF), L(LROFF) + 2u,
    L(GOLOADLV), L(GOLOADLV) + 1u, L(V47TXACT), L(V47TXACT) + 1u, L(V47TXACT) + 5u, L(CKMODCAD),
    L(ALINTIME), L(ALINTIME) + 1u, L(VB64), L(VB64) + 1u, L(VB64) + 2u, L(VB64) + 6u,
    L(V82PERF), L(V82PERF) + 1u, L(V82PERF) + 3u, L(V83PERF), L(V83PERF) + 1u, L(V83PERF) + 5u,
    L(V89PERF), L(V89PERF) + 1u, L(V89PERF) + 2u, L(V89PERF) + 6u, L(V90PERF), L(V90PERF) + 1u, L(V90PERF) + 5u,
    L(MINIMP), L(NOMINIMP), L(NOMINIMP) + 6u, L(CREWMANU), L(CREWMANU) + 1u, L(CREWMANU) + 2u, L(CREWMANU) + 6u,
    L(TRMTRACK), L(TRMTRACK) + 6u, L(TRMTRACK) + 8u, L(TRMTRACK) + 10u, L(DNEDUMP),
    L(LEMVEC), L(LEMVEC) + 2u, L(CSMVEC), L(NOUPDOWN), L(NOUPDOWN) + 2u, L(UPDATOFF), L(UPDATOFF) + 2u,
    L(SYSTEST), L(SYSTEST) + 1u, L(SYSTEST) + 2u, L(SYSTEST) + 6u, L(WMATRXNG),
    L(SHOWSUM), L(SHOWSUM) + 1u, L(SHOWSUM) + 2u, L(SHOWSUM) + 4u, L(SDISPLAY), L(NOKILL), L(NOKILL) + 5u,
    L(NOKILL) + 6u, L(NOKILL) + 7u, L(NOKILL) + 8u, L(ENDSUMS),
    L(DAPDISP), L(DAPDISP) + 1u, L(DAPDISP) + 3u, L(ATTACHED), L(ATTACHED) + 4u,
    L(VERB96), L(VERB96) + 2u, L(V67), L(V67) + 1u, L(V67) + 5u, L(SNUFFOUT), L(SNUFFOUT) + 2u,
    L(OUTSNUFF), L(OUTSNUFF) + 2u, L(VERB85), L(VERB85) + 1u
};

/* job starters: CAF PRIOn, TC FINDVAC (2CADR follows) */
static uint16_t lm_findvac(apollo_overlay_state *s, uint16_t prio, uint16_t at)
{
    lanec_ev_ca(s, prio);
    CALL(L(FINDVAC), at + 1u);
}

static uint16_t lm_step(apollo_overlay_state *s, lanec_io *io, uint16_t z)
{
    uint16_t *w = s->words, hi, lo;
    switch (z) {
    /* flag verbs: TC UPFLAG/DOWNFLAG, ADRES flag, TC/TCF GOPIN */
    case L(DAPATTER): CALL(L(DOWNFLAG), z + 1u);
    case L(TOTATTER): CALL(L(UPFLAG), z + 1u);
    case L(LRON): CALL(L(UPFLAG), z + 1u);
    case L(LROFF): CALL(L(DOWNFLAG), z + 1u);
    case L(LEMVEC): CALL(L(DOWNFLAG), z + 1u);
    case L(CSMVEC): CALL(L(UPFLAG), z + 1u);
    case L(NOUPDOWN): CALL(L(DOWNFLAG), z + 1u);
    case L(UPDATOFF): CALL(L(UPFLAG), z + 1u);
    case L(SNUFFOUT): CALL(L(UPFLAG), z + 1u);
    case L(OUTSNUFF): CALL(L(DOWNFLAG), z + 1u);
    case L(VERB96): CALL(L(UPFLAG), z + 1u);
    case L(DAPATTER) + 2u: case L(TOTATTER) + 2u: case L(UPDATOFF) + 2u: case L(SNUFFOUT) + 2u:
    case L(OUTSNUFF) + 2u:
        CALL(L(GOPIN), z + 1u);
    case L(LRON) + 2u: case L(LROFF) + 2u: case L(NOUPDOWN) + 2u: JUMP(L(GOPIN));
    case L(LEMVEC) + 2u: CALL(L(NOUPDOWN), z + 1u);                   /* TC NOUPDOWN */
    case L(VERB96) + 2u: w[LANEC_A] = 0; CALL(L(POSTJUMP), z + 2u);   /* CAF ZERO, POSTJUMP V37 */
    /* TC CHKPOOH and TC TESTXACT return to the next word */
    case L(VB64): case L(V89PERF): case L(CREWMANU): case L(SYSTEST): case L(SHOWSUM):
        CALL(L(CHKPOOH), z + 1u);
    case L(V47TXACT): case L(ALINTIME): case L(VB64) + 1u: case L(V82PERF): case L(V83PERF):
    case L(V89PERF) + 1u: case L(V90PERF): case L(CREWMANU) + 1u: case L(SYSTEST) + 1u: case L(SHOWSUM) + 1u:
    case L(DAPDISP): case L(V67): case L(VERB85):
        CALL(L(TESTXACT), z + 1u);
    case L(V47TXACT) + 1u: case L(VB64) + 2u: return lm_findvac(s, LW(PRIO4), z + 1u);
    case L(V89PERF) + 2u: case L(CREWMANU) + 2u: return lm_findvac(s, LW(PRIO10), z + 1u);
    case L(SYSTEST) + 2u: return lm_findvac(s, LW(PRIO22), z + 1u);
    case L(V90PERF) + 1u: return lm_findvac(s, LW(PRIO7), z + 1u);
    case L(V67) + 1u: return lm_findvac(s, LW(PRIO5), z + 1u);
    case L(ATTACHED): return lm_findvac(s, LW(PRIO10), z + 1u);
    case L(V47TXACT) + 5u: case L(VB64) + 6u: case L(V83PERF) + 5u: case L(V89PERF) + 6u:
    case L(CREWMANU) + 6u: case L(SYSTEST) + 6u: case L(ATTACHED) + 4u: case L(V67) + 5u:
        CALL(L(ENDOFJOB), z + 1u);
    case L(V90PERF) + 5u: JUMP(L(ENDOFJOB));
    case L(ALINTIME) + 1u: case L(VERB85) + 1u: CALL(L(POSTJUMP), z + 1u);
    case L(V82PERF) + 1u: case L(DAPDISP) + 1u: case L(SHOWSUM) + 2u:   /* CAF PRIO7, TC PRIOCHNG */
        lanec_ev_ca(s, LW(PRIO7));
        CALL(L(PRIOCHNG), z + 2u);
    case L(DAPDISP) + 3u: CALL(L(POSTJUMP), z + 1u);
    case L(V82PERF) + 3u:                      /* EXTEND, DCA V82CON, TC SUPDXCHZ */
        lanec_ev_set_l(s, lanec_a16(LW(V82CON_P_1)));
        lanec_ev_ca(s, LW(V82CON));
        CALL(L(SUPDXCHZ), z + 3u);
    case L(V83PERF) + 1u: lanec_ev_ca(s, LW(BIT2)); CALL(L(WAITLIST), z + 2u);
    /* GOLOADLV: TC FLASHOFF, CAF PINSUPBT, EXTEND, WRITE SUPERBNK, TC POSTJUMP */
    case L(GOLOADLV): CALL(L(FLASHOFF), z + 1u);
    case L(GOLOADLV) + 1u:
        lanec_ev_ca(s, LW(PINSUPBT));
        io->superbank = (uint16_t)(lanec_ev_corrected(w[LANEC_A]) & 0160u);
        CALL(L(POSTJUMP), z + 4u);
    /* CKMODCAD: CA MODECADR, EXTEND, BZF TCQ, TC ALM/END */
    case L(CKMODCAD):
        lanec_ev_ca(s, w[L(MODECADR)]);
        if (lanec_ev_zero(s)) JUMP(L(TCQ));
        CALL(L(ALM_S_END), z + 4u);
    /* MINIMP: INHINT, CS DAPBOOLS, MASK PULSES, ADS DAPBOOLS, TCF GOPIN */
    case L(MINIMP):
        io->interrupt_enabled = 0;
        lanec_ev_cs(s, w[L(DAPBOOLS)]);
        lanec_mask(s, LW(PULSES));
        lanec_ev_ads(s, L(DAPBOOLS));
        JUMP(L(GOPIN));
    /* NOMINIMP: INHINT, CS PULSES, MASK DAPBOOLS, TS DAPBOOLS, TC IBNKCALL */
    case L(NOMINIMP):
        io->interrupt_enabled = 0;
        lanec_ev_cs(s, LW(PULSES));
        lanec_mask(s, w[L(DAPBOOLS)]);
        (void)lanec_ts(s, L(DAPBOOLS));
        CALL(L(IBNKCALL), z + 5u);
    case L(NOMINIMP) + 6u: CALL(L(GOPIN), z + 1u);
    /* TRMTRACK: CA BITS9+7, MASK FLAGWRD0, EXTEND, BZF GOPIN, TC DOWNFLAG x3,
     * CA TRACKBIT, MASK FLAGWRD1, EXTEND, BZF GOPIN, TC POSTJUMP */
    case L(TRMTRACK):
        lanec_ev_ca(s, LW(BITS9_P_7));
        lanec_mask(s, w[L(FLAGWRD0)]);
        if (lanec_ev_zero(s)) JUMP(L(GOPIN));
        CALL(L(DOWNFLAG), z + 5u);
    case L(TRMTRACK) + 6u: case L(TRMTRACK) + 8u: CALL(L(DOWNFLAG), z + 1u);
    case L(TRMTRACK) + 10u:
        lanec_ev_ca(s, LW(TRACKBIT));
        lanec_mask(s, w[L(FLAGWRD1)]);
        if (lanec_ev_zero(s)) JUMP(L(GOPIN));
        CALL(L(POSTJUMP), z + 5u);
    /* DNEDUMP: CAF LDNDUMPI, TS DNTMGOTO, TC GOPIN */
    case L(DNEDUMP):
        lanec_ev_ca(s, LW(LDNDUMPI));
        (void)lanec_ts(s, L(DNTMGOTO));
        CALL(L(GOPIN), z + 3u);
    /* WMATRXNG: INHINT, CS RENDWBIT, MASK FLAGWRD5, TS FLAGWRD5, TC GOPIN */
    case L(WMATRXNG):
        io->interrupt_enabled = 0;
        lanec_ev_cs(s, LW(RENDWBIT));
        lanec_mask(s, w[L(FLAGWRD5)]);
        (void)lanec_ts(s, L(FLAGWRD5));
        CALL(L(GOPIN), z + 5u);
    /* SHOWSUM +4: CAF S+1, TS SKEEP6, CAF S+ZERO, TS SMODE, CA SELFADRS,
     * TS SELFRET, TC STSHOSUM */
    case L(SHOWSUM) + 4u:
        lanec_ev_ca(s, LW(S_P_1)); (void)lanec_ts(s, L(SKEEP6));
        lanec_ev_ca(s, LW(S_P_ZERO)); (void)lanec_ts(s, L(SMODE));
        lanec_ev_ca(s, LW(SELFADRS)); (void)lanec_ts(s, L(SELFRET));
        CALL(L(STSHOSUM), z + 7u);
    case L(SDISPLAY):                          /* LXCH SKEEP2, LXCH SKEEP3 */
        hi = w[L(SKEEP2)]; w[L(SKEEP2)] = lanec_ev_corrected(w[LANEC_L]); lanec_ev_set_l(s, lanec_a16(hi));
        lo = w[L(SKEEP3)]; w[L(SKEEP3)] = lanec_ev_corrected(w[LANEC_L]); lanec_ev_set_l(s, lanec_a16(lo));
        JUMP(L(NOKILL));
    case L(NOKILL):                            /* CA ADRS1, TS MPAC +2, CA VNCON, TC BANKCALL */
        lanec_ev_ca(s, LW(ADRS1)); (void)lanec_ts(s, L(MPAC) + 2u);
        lanec_ev_ca(s, LW(VNCON));
        CALL(L(BANKCALL), z + 4u);
    case L(NOKILL) + 5u: CALL(z + 3u, z + 1u);                      /* TC +3 */
    case L(NOKILL) + 6u: CALL(L(NXTBNK), z + 1u);
    case L(NOKILL) + 7u: CALL(L(NOKILL), z + 1u);
    case L(NOKILL) + 8u:                       /* CA SELFADRS, TS SKEEP1, TC ENDEXT */
        lanec_ev_ca(s, LW(SELFADRS)); (void)lanec_ts(s, L(SKEEP1));
        CALL(L(ENDEXT), z + 3u);
    case L(ENDSUMS):                           /* CA SKEEP6, EXTEND, BZF SELFCHK, TC STSHOSUM */
        lanec_ev_ca(s, w[L(SKEEP6)]);
        if (lanec_ev_zero(s)) JUMP(L(SELFCHK));
        CALL(L(STSHOSUM), z + 4u);
    default: return NOT_ENTRY;
    }
}

/* ------------------------------------------------------------------ CM */
#define C(X) LANEC_CM_##X
#define CW(X) LANEC_CM_##X##_WORD

static const uint16_t cm_entries[] = {
    C(CKMODCAD), C(CKMODCAD) + 1u, C(GOLOADLV), C(GOLOADLV) + 1u, C(V60), C(V61), C(V61) + 2u,
    C(V62), C(V62) + 2u, C(V62) + 4u, C(V63), C(V63) + 2u, C(V63) + 4u,
    C(SYSTEST), C(SYSTEST) + 1u, C(V92CONT), C(V92CONT) + 1u, C(V92CONT) + 5u,
    C(CKOPTVB), C(CKOPTVB) + 2u, C(CKOPTVB) + 3u, C(CKOPTVB) + 8u,
    C(CHAZFOGC), C(CHAZFOGC) + 2u, C(CHAZFOGC) + 3u, C(CHAZFOGC) + 7u, C(CHAZFOGC) + 9u,
    C(VB64), C(VB64) + 1u, C(VB64) + 2u, C(VB64) + 7u, C(ENATMA), C(ENATMA) + 2u,
    C(STROKON), C(STROKON) + 8u, C(STABLISH), C(STABLISH) + 12u, C(STABLISH) + 13u,
    C(CREWMANU), C(CREWMANU) + 1u, C(CREWMANU) + 2u, C(CREWMANU) + 6u,
    C(V82PERF), C(V82PERF) + 1u, C(V82PERF) + 3u, C(V83PERF), C(V83PERF) + 1u, C(V83PERF) + 5u, C(V83PERF) + 9u,
    C(V85PERF), C(V85PERF) + 1u, C(GOTOR21), C(GOTOR21) + 2u, C(GOTOR23), C(GOTOR23) + 2u, C(GOTOR23) + 3u,
    C(GOTOR23) + 19u, C(REGR21), C(REGR21) + 4u, C(R22ALARM), C(R22ALARM) + 2u, C(V86PERF),
    C(LEMVEC), C(LEMVEC) + 2u, C(CSMVEC), C(CSMVEC) + 2u, C(DNEDUMP), C(LFTFLGON), C(LFTFLGON) + 2u,
    C(CHKPOOH), C(EXDAPOFF), C(V89PERF), C(V89PERF) + 1u, C(V89PERF) + 2u, C(V89PERF) + 7u,
    C(WMATRXNG), C(WMATRXNG) + 2u, C(WMATRXNG) + 4u,
    C(SHOWSUM), C(SHOWSUM) + 1u, C(SHOWSUM) + 2u, C(SDISPLAY), C(NOKILL), C(NOKILL) + 5u,
    C(NOKILL) + 6u, C(NOKILL) + 7u, C(NOKILL) + 8u, C(ENDSUMS),
    C(SETPRFLG), C(SETPRFLG) + 2u, C(RESETPRF), C(RESETPRF) + 2u, C(ATTACHED), C(ATTACHED) + 4u,
    C(VERB94), C(VERB94) + 6u, C(VERB94) + 8u, C(VERB94) + 9u, C(VERB94) + 11u, C(V90PERF), C(V90PERF) + 1u, C(V90PERF) + 5u,
    C(VERB96), C(VERB96) + 2u, C(VERB96) + 4u, C(V52), C(V52) + 2u, C(V52) + 3u,
    C(V67), C(V67) + 1u, C(V67) + 5u, C(SETSURF), C(SETSURF) + 2u, C(RESTSRF), C(RESTSRF) + 2u
};

static uint16_t cm_findvac(apollo_overlay_state *s, uint16_t prio, uint16_t at)
{
    lanec_ev_ca(s, prio);
    CALL(C(FINDVAC), at + 1u);
}

static uint16_t cm_novac(apollo_overlay_state *s, uint16_t prio, uint16_t at)
{
    lanec_ev_ca(s, prio);
    CALL(C(NOVAC), at + 1u);
}

static uint16_t cm_step(apollo_overlay_state *s, lanec_io *io, uint16_t z)
{
    uint16_t *w = s->words, hi, lo, k;
    switch (z) {
    /* flag verbs */
    case C(V61): case C(V63) + 2u: case C(ENATMA): case C(GOTOR21): case C(LEMVEC): case C(WMATRXNG):
    case C(WMATRXNG) + 2u: case C(RESETPRF): case C(RESTSRF):
        CALL(C(DOWNFLAG), z + 1u);
    case C(V62): case C(V62) + 2u: case C(V63): case C(GOTOR23): case C(CSMVEC): case C(LFTFLGON):
    case C(SETPRFLG): case C(SETSURF): case C(VERB96): case C(VERB96) + 2u:
        CALL(C(UPFLAG), z + 1u);
    case C(V61) + 2u: case C(V62) + 4u: case C(V63) + 4u: case C(ENATMA) + 2u: case C(LFTFLGON) + 2u:
    case C(WMATRXNG) + 4u: case C(SETPRFLG) + 2u: case C(RESETPRF) + 2u:
        CALL(C(GOPIN), z + 1u);
    case C(LEMVEC) + 2u: case C(CSMVEC) + 2u: case C(SETSURF) + 2u: case C(RESTSRF) + 2u: JUMP(C(GOPIN));
    case C(VERB96) + 4u: w[LANEC_A] = 0; CALL(C(POSTJUMP), z + 2u);   /* CAF ZERO, POSTJUMP V37 */
    /* TC CHKPOOH, TC TESTXACT */
    case C(SYSTEST): case C(VB64): case C(CREWMANU): case C(V89PERF): case C(SHOWSUM):
        CALL(C(CHKPOOH), z + 1u);
    case C(VB64) + 1u: case C(CREWMANU) + 1u: case C(V82PERF): case C(V83PERF): case C(V85PERF):
    case C(GOTOR23) + 2u: case C(V89PERF) + 1u: case C(SHOWSUM) + 1u: case C(V90PERF): case C(V67):
        CALL(C(TESTXACT), z + 1u);
    case C(VB64) + 2u: case C(V89PERF) + 2u:   /* INHINT, CAF PRIOn, TC FINDVAC */
        io->interrupt_enabled = 0;
        return cm_findvac(s, z == C(VB64) + 2u ? CW(PRIO4) : CW(PRIO10), z + 2u);
    case C(CREWMANU) + 2u: return cm_findvac(s, CW(PRIO10), z + 1u);
    case C(V90PERF) + 1u: return cm_findvac(s, CW(PRIO7), z + 1u);
    case C(V67) + 1u: return cm_findvac(s, CW(PRIO5), z + 1u);
    case C(ATTACHED): return cm_findvac(s, CW(PRIO10), z + 1u);
    case C(VB64) + 7u: case C(CREWMANU) + 6u: case C(V83PERF) + 9u: case C(GOTOR23) + 19u: case C(REGR21) + 4u:
    case C(ATTACHED) + 4u: case C(V67) + 5u:
        CALL(C(ENDOFJOB), z + 1u);
    case C(V89PERF) + 7u: case C(V90PERF) + 5u: JUMP(C(ENDOFJOB));
    case C(V82PERF) + 1u:                      /* CAF PRIO7, TC PRIOCHNG */
        lanec_ev_ca(s, CW(PRIO7));
        CALL(C(PRIOCHNG), z + 2u);
    case C(V82PERF) + 3u: CALL(C(POSTJUMP), z + 1u);
    /* CKMODCAD: CA MODECADR, EXTEND, BZF TCQ, TC ALM/END (OPTCOARK enters at +1) */
    case C(CKMODCAD): lanec_ev_ca(s, w[C(MODECADR)]); JUMP(C(CKMODCAD) + 1u);
    case C(CKMODCAD) + 1u:
        if (lanec_ev_zero(s)) JUMP(C(TCQ));
        CALL(C(ALM_S_END), z + 3u);
    case C(GOLOADLV): CALL(C(FLASHOFF), z + 1u);
    case C(GOLOADLV) + 1u:
        lanec_ev_ca(s, CW(PINSUPBT));
        io->superbank = (uint16_t)(lanec_ev_corrected(w[LANEC_A]) & 0160u);
        CALL(C(POSTJUMP), z + 4u);
    /* V60: EXTEND, DCA CDUX, DXCH CPHIX, CA CDUZ, TS CPHIX +2, TC GOPIN */
    case C(V60):
        hi = w[C(CDUX)]; lo = w[C(CDUX) + 1u];
        w[LANEC_A] = lanec_a16(w[C(CPHIX)]); lanec_ev_set_l(s, lanec_a16(w[C(CPHIX) + 1u]));
        w[C(CPHIX)] = hi; w[C(CPHIX) + 1u] = lo;
        lanec_ev_ca(s, w[C(CDUZ)]);
        (void)lanec_ts(s, C(CPHIX) + 2u);
        CALL(C(GOPIN), z + 6u);
    /* SYSTEST +1: CA FLAGWRD1, MASK NOP01BIT, EXTEND, BZF V92CONT, TC POODOO */
    case C(SYSTEST) + 1u:
        lanec_ev_ca(s, w[C(FLAGWRD1)]);
        lanec_mask(s, CW(NOP01BIT));
        if (lanec_ev_zero(s)) JUMP(C(V92CONT));
        CALL(C(POODOO), z + 5u);
    case C(V92CONT): CALL(C(EXDAPOFF), z + 1u);                     /* returns via TC Q */
    case C(V92CONT) + 1u: return cm_findvac(s, CW(PRIO20), z + 1u);
    case C(V92CONT) + 5u: case C(CKOPTVB) + 8u: case C(CHAZFOGC) + 9u: CALL(C(GOPIN), z + 1u);
    /* EXDAPOFF: EXTEND, DCA IDLECADR, DXCH T5LOC, CS OCT60000, MASK FLAGWRD6,
     * TS FLAGWRD6, TC Q */
    case C(EXDAPOFF):
        w[LANEC_A] = lanec_a16(w[C(T5LOC)]); lanec_ev_set_l(s, lanec_a16(w[C(T5LOC) + 1u]));
        w[C(T5LOC)] = CW(IDLECADR); w[C(T5LOC) + 1u] = CW(IDLECADR_P_1);
        lanec_ev_cs(s, CW(OCT60000));
        lanec_mask(s, w[C(FLAGWRD6)]);
        (void)lanec_ts(s, C(FLAGWRD6));
        JUMP(lanec_ev_tc_q(s));                         /* TC Q leaves Q and runs the register */
    /* CHECKMM, MM nn: returns past the TC ALM/END that follows when in that program */
    case C(CKOPTVB): case C(CHAZFOGC): case C(V52): CALL(C(CHECKMM), z + 1u);
    case C(CKOPTVB) + 2u: case C(CHAZFOGC) + 2u: case C(V52) + 2u: case C(VERB94) + 8u:
        CALL(C(ALM_S_END), z + 1u);
    case C(CKOPTVB) + 3u: io->interrupt_enabled = 0; return cm_findvac(s, CW(PRIO16), z + 2u);
    case C(CHAZFOGC) + 3u: return cm_findvac(s, CW(PRIO16), z + 1u);
    case C(CHAZFOGC) + 7u: CALL(C(PHASCHNG), z + 1u);
    /* STROKON: CS FLAGWRD6, MASK OCT60000, EXTEND, BZMF ALM/END, CAF PRIO30, TC NOVAC */
    case C(STROKON):
        lanec_ev_cs(s, w[C(FLAGWRD6)]);
        lanec_mask(s, CW(OCT60000));
        if (lanec_ev_nonpositive(s)) JUMP(C(ALM_S_END));
        return cm_novac(s, CW(PRIO30), z + 5u);
    case C(STROKON) + 8u: CALL(C(GOPIN), z + 1u);
    /* STABLISH: CAF EBANK6, TS EBANK, CS FLAGWRD6, MASK OCT60000, EXTEND,
     * BZMF +8D, CAE DAPDATR1, MASK BIT14, EXTEND, BZMF +3, TC POSTJUMP */
    case C(STABLISH):
        lanec_ev_ca(s, CW(EBANK6));
        lanec_ev_set_eb(s, w[LANEC_A]);
        lanec_ev_cs(s, w[C(FLAGWRD6)]);
        lanec_mask(s, CW(OCT60000));
        if (lanec_ev_nonpositive(s)) JUMP(z + 13u);
        lanec_ev_ca(s, w[E(C(DAPDATR1))]);
        lanec_mask(s, CW(BIT14));
        if (lanec_ev_nonpositive(s)) JUMP(z + 12u);
        CALL(C(POSTJUMP), z + 11u);
    case C(STABLISH) + 12u: CALL(C(ALM_S_END), z + 1u);
    case C(STABLISH) + 13u: CALL(C(POSTJUMP), z + 1u);
    /* V83PERF +1: INHINT, CS FLAGWRD9, MASK R31FLBIT, ADS FLAGWRD9; +5 CAF PRIO5, TC NOVAC */
    case C(V83PERF) + 1u:
        io->interrupt_enabled = 0;
        lanec_ev_cs(s, w[C(FLAGWRD9)]);
        lanec_mask(s, CW(R31FLBIT));
        lanec_ev_ads(s, C(FLAGWRD9));
        JUMP(C(V83PERF) + 5u);
    case C(V83PERF) + 5u: return cm_novac(s, CW(PRIO5), z + 1u);
    /* V85PERF +1: INHINT, CS R31FLBIT, MASK FLAGWRD9, TS FLAGWRD9, TC V83PERF +5 */
    case C(V85PERF) + 1u:
        io->interrupt_enabled = 0;
        lanec_ev_cs(s, CW(R31FLBIT));
        lanec_mask(s, w[C(FLAGWRD9)]);
        (void)lanec_ts(s, C(FLAGWRD9));
        CALL(C(V83PERF) + 5u, z + 5u);
    case C(GOTOR21) + 2u: CALL(z + 3u, z + 1u);                       /* TC +3 */
    /* GOTOR23 +3: rendezvous and track flags, then R23 or R21 */
    case C(GOTOR23) + 3u:
        lanec_ev_ca(s, w[C(FLAGWRD0)]); lanec_mask(s, CW(RNDVZBIT));
        if (lanec_ev_zero(s)) JUMP(C(R22ALARM));
        lanec_ev_ca(s, w[C(FLAGWRD1)]); lanec_mask(s, CW(TRACKBIT));
        if (lanec_ev_zero(s)) JUMP(C(R22ALARM));
        lanec_ev_ca(s, w[C(FLAGWRD1)]); lanec_mask(s, CW(R23BIT));
        if (lanec_ev_zero(s)) JUMP(C(REGR21));
        return cm_novac(s, CW(PRIO16), z + 13u);
    case C(REGR21): return cm_novac(s, CW(PRIO16), z + 1u);
    case C(R22ALARM): CALL(C(ALARM), z + 1u);
    case C(R22ALARM) + 2u: CALL(C(ENDEXT), z + 1u);
    /* V86PERF: CAF EBANK7, XCH EBANK, CA NEGONE, TS MRKBUF1, TC GOPIN */
    case C(V86PERF):
        k = w[LANEC_EB];
        lanec_ev_set_eb(s, CW(EBANK7));
        w[LANEC_A] = k;
        lanec_ev_ca(s, CW(NEGONE));
        (void)lanec_ts(s, E(C(MRKBUF1)));
        CALL(C(GOPIN), z + 5u);
    case C(DNEDUMP):
        lanec_ev_ca(s, CW(LDNDUMPI));
        (void)lanec_ts(s, C(DNTMGOTO));
        CALL(C(GOPIN), z + 3u);
    /* CHKPOOH: CA MODREG, EXTEND, BZF TCQ, TCF ALM/END */
    case C(CHKPOOH):
        lanec_ev_ca(s, w[C(MODREG)]);
        if (lanec_ev_zero(s)) JUMP(C(TCQ));
        JUMP(C(ALM_S_END));
    /* SHOWSUM +2: as LM SHOWSUM +4 (no PRIOCHNG in Comanche) */
    case C(SHOWSUM) + 2u:
        lanec_ev_ca(s, CW(S_P_1)); (void)lanec_ts(s, C(SKEEP6));
        lanec_ev_ca(s, CW(S_P_ZERO)); (void)lanec_ts(s, C(SMODE));
        lanec_ev_ca(s, CW(SELFADRS)); (void)lanec_ts(s, C(SELFRET));
        CALL(C(STSHOSUM), z + 7u);
    case C(SDISPLAY):
        hi = w[C(SKEEP2)]; w[C(SKEEP2)] = lanec_ev_corrected(w[LANEC_L]); lanec_ev_set_l(s, lanec_a16(hi));
        lo = w[C(SKEEP3)]; w[C(SKEEP3)] = lanec_ev_corrected(w[LANEC_L]); lanec_ev_set_l(s, lanec_a16(lo));
        JUMP(C(NOKILL));
    case C(NOKILL):
        lanec_ev_ca(s, CW(ADRS1)); (void)lanec_ts(s, C(MPAC) + 2u);
        lanec_ev_ca(s, CW(VNCON));
        CALL(C(BANKCALL), z + 4u);
    case C(NOKILL) + 5u: CALL(z + 3u, z + 1u);
    case C(NOKILL) + 6u: CALL(C(NXTBNK), z + 1u);
    case C(NOKILL) + 7u: CALL(C(NOKILL), z + 1u);
    case C(NOKILL) + 8u:
        lanec_ev_ca(s, CW(SELFADRS)); (void)lanec_ts(s, C(SKEEP1));
        CALL(C(ENDEXT), z + 3u);
    case C(ENDSUMS):
        lanec_ev_ca(s, w[C(SKEEP6)]);
        if (lanec_ev_zero(s)) JUMP(C(SELFCHK));
        CALL(C(STSHOSUM), z + 4u);
    /* VERB94: CAF V94FLBIT, MASK FLAGWRD9, EXTEND, BZF ALM/END, TC DOWNFLAG;
     * +6 TC CHECKMM; +9 TC PHASCHNG; +11 TC CLEANOUT */
    case C(VERB94):
        lanec_ev_ca(s, CW(V94FLBIT));
        lanec_mask(s, w[C(FLAGWRD9)]);
        if (lanec_ev_zero(s)) JUMP(C(ALM_S_END));
        CALL(C(DOWNFLAG), z + 5u);
    case C(VERB94) + 6u: CALL(C(CHECKMM), z + 1u);
    case C(VERB94) + 9u: CALL(C(PHASCHNG), z + 1u);
    case C(VERB94) + 11u: CALL(C(CLEANOUT), z + 1u);
    /* V52 +3: CAF LANDBANK, TS EBANK, CS PRIO7, MASK LANDMARK, TS LANDMARK,
     * CA MARKSTAT, TS SR, CA SR, CA SR, MASK PRIO7, CS A, AD PRIO5,
     * ADS LANDMARK, TC GOPIN. SR shifts on the store and on each read. */
    case C(V52) + 3u:
        lanec_ev_ca(s, CW(LANDBANK));
        lanec_ev_set_eb(s, w[LANEC_A]);
        k = E(C(LANDMARK));
        lanec_ev_cs(s, CW(PRIO7));
        lanec_mask(s, w[k]);
        (void)lanec_ts(s, k);
        lanec_ev_ca(s, w[C(MARKSTAT)]);
        (void)lanec_ts(s, 021u); lanec_rewrite(s, 021u);
        lanec_ev_ca(s, w[021u]); lanec_rewrite(s, 021u);
        lanec_ev_ca(s, w[021u]); lanec_rewrite(s, 021u);
        lanec_mask(s, CW(PRIO7));
        w[LANEC_A] = (uint16_t)~w[LANEC_A];
        lanec_ad(s, CW(PRIO5));
        lanec_ev_ads(s, k);
        CALL(C(GOPIN), z + 14u);
    default: return NOT_ENTRY;
    }
}

static uint8_t listed(const uint16_t *list, uint16_t n, uint16_t z)
{
    uint16_t i;
    for (i = 0; i < n; ++i)
        if (list[i] == z) return 1;
    return 0;
}

#define COUNT(X) ((uint16_t)(sizeof(X) / sizeof((X)[0])))

const uint16_t *lanec_ev_short_entries(enum lanec_mission m, uint16_t *count)
{
    *count = m == LANEC_LM ? COUNT(lm_entries) : COUNT(cm_entries);
    return m == LANEC_LM ? lm_entries : cm_entries;
}

uint8_t lanec_ev_short_entry(enum lanec_mission m, uint16_t z)
{
    return m == LANEC_LM ? listed(lm_entries, COUNT(lm_entries), z) : listed(cm_entries, COUNT(cm_entries), z);
}

enum lanec_status lanec_ev_short_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, z = w[LANEC_Z];
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 043u || !lanec_ev_short_entry(m, z))
        return LANEC_OUT_OF_SCOPE;
    do {
        z = m == LANEC_LM ? lm_step(s, io, z) : cm_step(s, io, z);
        w[LANEC_Z] = z;
    } while (lanec_ev_short_entry(m, z));
    return LANEC_OK;
}
