/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS alignment and radar-designate verb bodies, Luminary099
 * (bank 43, AURLOKON in bank 23) and Comanche055 (bank 43): V40 zero, V41
 * coarse align (IMU, RR or optics), V42 fine align, V43 attitude error
 * meters, V44 and RR designate, V60 LR position 2, the noun and radar-use
 * checks. Each case is one basic block of the source, entered at its
 * address; see docs/LANEC-EV-ALIGN.md for the source map. */
#include "lanec_ev_align.h"
#include "lanec_ev_internal.h"
#include "lanec_symbols.h"

#define NOT_ENTRY 0xffffu
#define CALL(T, R) return lanec_ev_call(s, (T), (uint16_t)(R))
#define JUMP(T) return (uint16_t)(T)
#define E(X) lanec_erasable(s, (X))
/* Entry keys: the address, plus 010000 for LM bank 23. */
#define B23 010000u

/* ------------------------------------------------------------------ LM */
#define L(X) LANEC_LM_##X
#define LW(X) LANEC_LM_##X##_WORD

static const uint16_t lm_entries[] = {
    L(VBZERO), L(VBZERO) + 1u, L(VBZERO) + 2u, L(IMUZEROK), L(IMUZEROK) + 1u, L(IMUZEROK) + 3u,
    L(IMUZEROK) + 5u, L(IMUZEROK) + 6u, L(RRZEROK), L(RRZEROK) + 1u, L(RWAITK), L(RWAITK) + 2u, L(RWAITK) + 3u,
    L(LRPOS2K), L(LRPOS2K) + 1u, L(LRPOS2K) + 3u, L(LRPOS2K) + 5u, L(LRPOS2K) + 6u, L(LRP2ALM), L(LRP2ALM) + 2u,
    L(VBCOARK), L(VBCOARK) + 1u, L(VBCOARK) + 2u, L(OP_S_INERT),
    L(IMUCOARK), L(IMUCOARK) + 1u, L(IMUCOARK) + 2u, L(IMUCOARK) + 5u, L(IMUCOARK) + 6u,
    L(ICORK2), L(ICORK2) + 3u, L(ICORK2) + 5u, L(ICORK2) + 7u, L(ICORK2) + 8u,
    L(RRDESNBK), L(RRDESNBK) + 1u, L(RRDESNBK) + 2u, L(RRDESNBK) + 10u, L(RRDESNBK) + 13u, L(RRDESNBK) + 14u,
    L(RRDESNBK) + 15u, L(RRDESNBK) + 17u, L(RRDESNBK) + 20u, L(RRDESNBK) + 24u,
    L(RRDESK2), L(RRDESK2) + 2u, L(RRDESK2) + 3u, L(RRDESK2) + 12u, L(RRDESK2) + 13u, L(RRDESK2) + 14u,
    L(RRDESK2) + 16u, L(RRDESEND), L(RRDESEND) + 9u, L(RRDESEND) + 12u, L(RRDESEND) + 14u,
    L(IMUFINEK), L(IMUFINEK) + 1u, L(IMUFINEK) + 2u, L(IMUFINEK) + 5u, L(IMUFINEK) + 6u, L(IMUFINEK) + 7u,
    L(IMUFINEK) + 10u, L(IMUFINEK) + 12u, L(IMUFINEK) + 14u, L(FINEK2), L(FINEK2) + 3u, L(FINEK2) + 5u,
    L(FINEK2) + 6u, L(RDRUSECK), L(CHECKRR), L(CKRNDBIT), L(CHECKP22),
    L(IMUATTCK), L(IMUATTCK) + 1u, L(IMUATTCK) + 13u, L(IMUATTCK) + 14u, L(IMUATTCK) + 17u,
    L(IMUATTCK) + 18u, L(IMUATTCK) + 19u, L(IMUATTCK) + 22u, L(IMUATTCK) + 29u,
    B23 | L(AURLOKON), B23 | (L(AURLOKON) + 1u), B23 | (L(AURLOKON) + 6u), B23 | (L(AURLOKON) + 9u),
    B23 | (L(AURLOKON) + 10u), B23 | (L(AURLOKON) + 11u), B23 | (L(AURLOKON) + 12u), B23 | (L(AURLOKON) + 14u),
    B23 | (L(AURLOKON) + 15u), B23 | (L(AURLOKON) + 21u), B23 | L(NOLOKON), B23 | (L(NOLOKON) + 2u),
    B23 | (L(NOLOKON) + 4u), B23 | L(AURLKON1)
};

/* CCS of a flag word masked to one bit (CS or CA first): +0 skips one word */
static uint8_t flag_set(apollo_overlay_state *s, uint16_t value, uint16_t bit, uint8_t complement)
{
    if (complement) lanec_ev_cs(s, value); else lanec_ev_ca(s, value);
    lanec_mask(s, bit);
    return (uint8_t)(lanec_ccs_a(s) == 0);
}

static uint16_t lm_step(apollo_overlay_state *s, lanec_io *io, uint16_t key)
{
    uint16_t *w = s->words, z = (uint16_t)(key & 07777u), k;
    switch (key) {
    /* VBZERO and VBCOARK: TC OP/INERT, then the noun 20 or 72 return */
    case L(VBZERO): case L(VBCOARK): CALL(L(OP_S_INERT), z + 1u);
    case L(VBZERO) + 1u: CALL(L(IMUZEROK), z + 1u);
    case L(VBZERO) + 2u: CALL(L(RRZEROK), z + 1u);
    case L(VBCOARK) + 1u: CALL(L(IMUCOARK), z + 1u);
    case L(VBCOARK) + 2u: CALL(L(RRDESNBK), z + 1u);
    /* OP/INERT: CS OCT24, AD NOUNREG, EXTEND, BZF TCQ, AD RRIMUDIF, EXTEND, BZF Q+1, TC ALM/END */
    case L(OP_S_INERT):
        lanec_ev_cs(s, LW(OCT24));
        lanec_ad(s, w[L(NOUNREG)]);
        if (lanec_ev_zero(s)) JUMP(L(TCQ));
        lanec_ad(s, LW(RRIMUDIF));
        if (lanec_ev_zero(s)) JUMP(L(Q_P_1));
        CALL(L(ALM_S_END), z + 8u);
    /* IMUZEROK: TC CKMODCAD, BANKCALL IMUZERO, BANKCALL IMUSTALL, TC +1, TC GOPIN */
    case L(IMUZEROK): case L(IMUCOARK): case L(IMUFINEK): CALL(L(CKMODCAD), z + 1u);
    case L(IMUZEROK) + 1u: case L(IMUZEROK) + 3u: case L(RRZEROK) + 1u: case L(RWAITK): case L(LRPOS2K) + 1u:
    case L(LRPOS2K) + 3u: case L(ICORK2) + 3u: case L(ICORK2) + 5u: case L(RRDESNBK) + 15u: case L(RRDESK2):
    case L(IMUFINEK) + 10u: case L(IMUFINEK) + 12u: case L(FINEK2) + 3u:
        CALL(L(BANKCALL), z + 1u);
    case L(IMUZEROK) + 5u: CALL(z + 1u, z + 1u);                      /* TC +1 */
    case L(IMUZEROK) + 6u: case L(RWAITK) + 3u: case L(LRPOS2K) + 6u: case L(LRP2ALM) + 2u:
        CALL(L(GOPIN), z + 1u);
    /* RRZEROK, LRPOS2K, RRDESNBK: TC RDRUSECK (returns through TC Q) */
    case L(RRZEROK): case L(LRPOS2K): case L(RRDESNBK): CALL(L(RDRUSECK), z + 1u);
    case L(RWAITK) + 2u: JUMP(z + 1u);                                 /* TCF +1 */
    case L(LRPOS2K) + 5u: CALL(L(LRP2ALM), z + 1u);
    case L(LRP2ALM): CALL(L(ALARM), z + 1u);                          /* OCT 523 follows */
    /* IMUCOARK +1: TC TESTXACT; +2 CAF VNLODCDU, BANKCALL GOXDSPF; +5 TC TERMEXTV; +6 TCF ICORK2 */
    case L(IMUCOARK) + 1u: case L(RRDESNBK) + 1u: case L(IMUFINEK) + 1u: case L(IMUATTCK) + 13u:
        CALL(L(TESTXACT), z + 1u);
    case L(IMUCOARK) + 2u: case L(IMUATTCK) + 14u: lanec_ev_ca(s, LW(VNLODCDU)); CALL(L(BANKCALL), z + 2u);
    case L(IMUCOARK) + 5u: case L(RRDESNBK) + 13u: case L(IMUFINEK) + 5u: CALL(L(ENDEXT), z + 1u);
    case L(IMUCOARK) + 6u: JUMP(L(ICORK2));
    case L(ICORK2): lanec_ev_ca(s, LW(IMUCOARV)); CALL(L(BANKCALL), z + 2u);
    case L(ICORK2) + 7u: case L(ICORK2) + 8u: case L(IMUFINEK) + 14u: case L(FINEK2) + 5u: case L(FINEK2) + 6u:
        CALL(L(ENDEXT), z + 1u);
    /* RRDESNBK +2: CA RNDVZBIT, MASK FLAGWRD0, CCS A, TCF XACTALM; CS OCT41000,
     * INHINT, MASK RADMODES, TS RADMODES; +10 CAF VNLDRCDU, BANKCALL GOXDSPF */
    case L(RRDESNBK) + 2u:
        lanec_ev_ca(s, LW(RNDVZBIT));
        lanec_mask(s, w[L(FLAGWRD0)]);
        if (lanec_ccs_a(s) == 0) JUMP(L(XACTALM));
        lanec_ev_cs(s, LW(OCT41000));
        io->interrupt_enabled = 0;
        lanec_mask(s, w[L(RADMODES)]);
        (void)lanec_ts(s, L(RADMODES));
        JUMP(z + 8u);
    case L(RRDESNBK) + 10u: lanec_ev_ca(s, LW(VNLDRCDU)); CALL(L(BANKCALL), z + 2u);
    case L(RRDESNBK) + 14u: JUMP(z - 4u);                             /* TCF -4 */
    case L(RRDESNBK) + 17u: lanec_ev_ca(s, LW(IMUCOARV)); CALL(L(BANKCALL), z + 2u);   /* OPTCOARV */
    case L(RRDESNBK) + 20u: lanec_ev_ca(s, LW(PRIO20)); CALL(L(FINDVAC), z + 2u);
    case L(RRDESNBK) + 24u: JUMP(L(ENDEXT));
    /* RRDESK2 +2: TC +1; +3 CA PRIORITY, MASK LOW9, CCS A, INDEX A, TS A
     * (frees the job's VAC area), COM, ADS PRIORITY, BANKCALL RADSTALL */
    case L(RRDESK2) + 2u: CALL(z + 1u, z + 1u);
    case L(RRDESK2) + 3u:
        lanec_ev_ca(s, w[L(PRIORITY)]);
        lanec_mask(s, LW(LOW9));
        if (lanec_ccs_a(s) == 0) {
            k = w[LANEC_A];
            (void)lanec_ts(s, k);                                      /* INDEX A, TS A */
        }
        w[LANEC_A] = (uint16_t)~w[LANEC_A];
        lanec_ev_ads(s, L(PRIORITY));
        CALL(L(BANKCALL), z + 8u);
    case L(RRDESK2) + 12u: CALL(z + 2u, z + 1u);                      /* TC +2 */
    case L(RRDESK2) + 13u: case L(RRDESK2) + 16u: CALL(L(ENDOFJOB), z + 1u);
    case L(RRDESK2) + 14u: CALL(L(ALARM), z + 1u);                    /* OCT 503 follows */
    /* RRDESEND: CCS RADMODES, TCF GOPIN, TCF GOPIN, TCF +1; CS OCT41000, INHINT,
     * MASK RADMODES, TS RADMODES, TC CLRADMOD */
    case L(RRDESEND):
        if (lanec_ccs(s, w[L(RADMODES)]) < 2u) JUMP(L(GOPIN));
        lanec_ev_cs(s, LW(OCT41000));
        io->interrupt_enabled = 0;
        lanec_mask(s, w[L(RADMODES)]);
        (void)lanec_ts(s, L(RADMODES));
        CALL(L(CLRADMOD), z + 9u);
    case L(RRDESEND) + 9u: lanec_ev_ca(s, LW(1SEC)); CALL(L(BANKCALL), z + 2u);
    case L(RRDESEND) + 12u: CALL(L(DOWNFLAG), z + 1u);
    case L(RRDESEND) + 14u: JUMP(L(GOPIN));
    /* IMUFINEK +2: CAF VNLODGYR, BANKCALL GOXDSPF; +6 TC +1; +7 CAF IMUFINEV, BANKCALL EXDSPRET */
    case L(IMUFINEK) + 2u: lanec_ev_ca(s, LW(VNLODGYR)); CALL(L(BANKCALL), z + 2u);
    case L(IMUFINEK) + 6u: case L(IMUATTCK) + 18u: CALL(z + 1u, z + 1u);
    case L(IMUFINEK) + 7u: lanec_ev_ca(s, LW(IMUFINEV)); CALL(L(BANKCALL), z + 2u);
    case L(FINEK2): lanec_ev_ca(s, LW(LGYROBIN)); CALL(L(BANKCALL), z + 2u);
    /* RDRUSECK: radar-use checks, then TC Q */
    case L(RDRUSECK):
        if (flag_set(s, w[L(FLAGWRD3)], LW(NR29FBIT), 1)) CALL(L(ALM_S_END), z + 4u);
        if (flag_set(s, w[L(FLAGWRD5)], LW(R77FLBIT), 0)) CALL(L(ALM_S_END), z + 8u);
        if (flag_set(s, w[L(FLAGWRD7)], LW(V37FLBIT), 1)) JUMP(L(CHECKRR));
        /* LRBYBIT is bit 15: the masked value is +0 or negative, and CCS of a
         * negative value skips two words, onto CHECKRR +1 (MASK TRACKBIT, CCS A) */
        lanec_ev_cs(s, w[L(FLGWRD11)]);
        lanec_mask(s, LW(LRBYBIT));
        switch (lanec_ccs_a(s)) {
        case 0: CALL(L(ALM_S_END), z + 16u);
        case 1: JUMP(L(CHECKRR));
        default:
            lanec_mask(s, LW(TRACKBIT));
            JUMP(lanec_ccs_a(s) == 0 ? L(CHECKP22) : L(CKRNDBIT));
        }
    case L(CHECKRR):
        if (flag_set(s, w[L(FLAGWRD1)], LW(TRACKBIT), 1)) JUMP(L(CHECKP22));
        JUMP(L(CKRNDBIT));
    case L(CKRNDBIT):
        if (flag_set(s, w[L(FLAGWRD0)], LW(RNDVZBIT), 0)) CALL(L(ALM_S_END), z + 4u);
        JUMP(L(CHECKP22));
    case L(CHECKP22):                          /* CS MODREG, AD DEC22, EXTEND, BZF ALM/END, TC Q */
        lanec_ev_cs(s, w[L(MODREG)]);
        lanec_ad(s, LW(DEC22));
        if (lanec_ev_zero(s)) JUMP(L(ALM_S_END));
        JUMP(LANEC_Q);
    /* IMUATTCK: TC CHKPOOH; +1 coarse-align and needles mode checks */
    case L(IMUATTCK): CALL(L(CHKPOOH), z + 1u);
    case L(IMUATTCK) + 1u:
        lanec_ev_ca(s, LW(BITS4_5));
        lanec_ev_rand(s, io, 012u);
        if (lanec_ccs_a(s) == 0) JUMP(L(ALM_S_END));
        lanec_ev_ca(s, LW(BIT13_M_14));
        lanec_ev_rxor(s, io, 031u);
        lanec_mask(s, LW(BIT13_M_14));
        if (lanec_ev_zero(s)) JUMP(z + 12u);
        JUMP(L(ALM_S_END));
    case L(IMUATTCK) + 17u: CALL(L(ENDEXT), z + 1u);
    case L(IMUATTCK) + 19u: lanec_ev_ca(s, LW(V43K)); CALL(L(BANKCALL), z + 2u);
    case L(IMUATTCK) + 22u:                    /* CAF BIT6, EXTEND, WOR CHAN12, CAF TWO, TC WAITLIST */
        lanec_ev_ca(s, LW(BIT6));
        lanec_ev_wor(s, io, 012u);
        lanec_ev_ca(s, LW(TWO));
        CALL(L(WAITLIST), z + 5u);
    case L(IMUATTCK) + 29u: JUMP(L(ENDEXT));
    /* AURLOKON (bank 23): TC MAKECADR; TS DESRET, CAF TWO, TS OPTIONX +1, CAF SIX, TS OPTIONX */
    case B23 | L(AURLOKON): CALL(L(MAKECADR), z + 1u);
    case B23 | (L(AURLOKON) + 1u):
        if (!lanec_ts(s, L(DESRET))) lanec_ev_ca(s, LW(TWO));          /* an overflow skips CAF TWO */
        (void)lanec_ts(s, L(OPTIONX) + 1u);                            /* A is TWO or +-1 */
        lanec_ev_ca(s, LW(SIX));
        (void)lanec_ts(s, L(OPTIONX));
        JUMP(z + 5u);
    case B23 | (L(AURLOKON) + 6u): lanec_ev_ca(s, LW(V04N1272)); CALL(L(BANKCALL), z + 2u);
    case B23 | (L(AURLOKON) + 9u): JUMP(L(ENDEXT));
    case B23 | (L(AURLOKON) + 10u): JUMP(z + 5u);                     /* TCF +5 */
    case B23 | (L(AURLOKON) + 11u): JUMP(z - 5u);                     /* TCF -5 */
    case B23 | (L(AURLOKON) + 12u): lanec_ev_ca(s, LW(BIT3)); CALL(L(BLANKET), z + 2u);
    case B23 | (L(AURLOKON) + 14u): CALL(L(ENDOFJOB), z + 1u);
    case B23 | (L(AURLOKON) + 15u):            /* CA OPTIONX +1, MASK BIT2, CCS A, TCF NOLOKON, TC UPFLAG */
        lanec_ev_ca(s, w[L(OPTIONX) + 1u]);
        lanec_mask(s, LW(BIT2));
        if (lanec_ccs_a(s) == 0) JUMP(L(NOLOKON));
        CALL(L(UPFLAG), z + 5u);
    case B23 | (L(AURLOKON) + 21u): JUMP(L(AURLKON1));
    case B23 | L(NOLOKON): CALL(L(DOWNFLAG), z + 1u);
    case B23 | (L(NOLOKON) + 2u): case B23 | (L(NOLOKON) + 4u): CALL(L(UPFLAG), z + 1u);
    case B23 | L(AURLKON1):                    /* RELINT, CA DESRET, TCF BANKJUMP */
        io->interrupt_enabled = 1;
        lanec_ev_ca(s, w[L(DESRET)]);
        JUMP(L(BANKJUMP));
    default: return NOT_ENTRY;
    }
}

/* ------------------------------------------------------------------ CM */
#define C(X) LANEC_CM_##X
#define CW(X) LANEC_CM_##X##_WORD

static const uint16_t cm_entries[] = {
    C(VBZERO), C(VBZERO) + 1u, C(VBZERO) + 2u, C(IMUZEROK), C(IMUZEROK) + 1u, C(IMUZEROK) + 3u,
    C(IMUZEROK) + 5u, C(IMUZEROK) + 6u, C(OP_S_INERT), C(VBCOARK), C(VBCOARK) + 1u, C(VBCOARK) + 2u,
    C(IMUCOARK), C(IMUCOARK) + 1u, C(IMUCOARK) + 2u, C(IMUCOARK) + 5u, C(IMUCOARK) + 6u,
    C(ICORK2), C(ICORK2) + 3u, C(ICORK2) + 5u, C(ICORK2) + 7u, C(ICORK2) + 8u,
    C(OPTCOARK), C(OPTCOARK) + 2u, C(OPTCOARK) + 3u, C(OPTCOARK) + 9u, C(OPTCOARK) + 11u, C(OPTCOARK) + 17u,
    C(OPTC1), C(OPTC1) + 3u, C(OPTC1) + 4u, C(OPTC1) + 5u, C(OPTC1) + 12u, C(OPTC1) + 15u,
    C(IMUFINEK), C(IMUFINEK) + 1u, C(IMUFINEK) + 2u, C(IMUFINEK) + 5u, C(IMUFINEK) + 6u, C(IMUFINEK) + 7u,
    C(IMUFINEK) + 10u, C(IMUFINEK) + 12u, C(IMUFINEK) + 14u, C(FINEK2), C(FINEK2) + 3u, C(FINEK2) + 5u,
    C(FINEK2) + 6u, C(IMUATTCK), C(IMUATTCK) + 1u, C(IMUATTCK) + 7u, C(IMUATTCK) + 8u, C(IMUATTCK) + 14u,
    C(IMUATTCK) + 15u, C(IMUATTCK) + 16u, C(IMUATTCK) + 20u, C(IMUATTCK) + 22u, C(TRMATTCK), C(TRMATTCK) + 1u,
    C(TRMATTCK) + 2u, C(ATTCK1), C(ATTCK1) + 7u, C(CKLFTBTS), C(XACT0Q)
};

static uint16_t cm_step(apollo_overlay_state *s, lanec_io *io, uint16_t z)
{
    uint16_t *w = s->words, a, l, k;
    switch (z) {
    case C(VBZERO): case C(VBCOARK): CALL(C(OP_S_INERT), z + 1u);
    case C(VBZERO) + 1u: CALL(C(IMUZEROK), z + 1u);
    case C(VBZERO) + 2u: CALL(C(ALM_S_END), z + 1u);
    case C(VBCOARK) + 1u: CALL(C(IMUCOARK), z + 1u);
    case C(VBCOARK) + 2u: CALL(C(OPTCOARK), z + 1u);
    /* OP/INERT: CS OCT24, AD NOUNREG, EXTEND, BZF XACT0Q, INCR Q, AD OPIMDIFF,
     * EXTEND, BZF XACT0Q, TC ALM/END */
    case C(OP_S_INERT):
        lanec_ev_cs(s, CW(OCT24));
        lanec_ad(s, w[C(NOUNREG)]);
        if (lanec_ev_zero(s)) JUMP(C(XACT0Q));
        w[LANEC_Q] = lanec_ev_add16(w[LANEC_Q], 1u);
        lanec_ad(s, CW(OPIMDIFF));
        if (lanec_ev_zero(s)) JUMP(C(XACT0Q));
        CALL(C(ALM_S_END), z + 9u);
    case C(XACT0Q): JUMP(LANEC_Q);                                     /* TC Q */
    case C(IMUZEROK): case C(IMUCOARK): case C(IMUFINEK): CALL(C(CKMODCAD), z + 1u);
    case C(IMUZEROK) + 1u: case C(IMUZEROK) + 3u: case C(ICORK2) + 3u: case C(ICORK2) + 5u:
    case C(IMUFINEK) + 10u: case C(IMUFINEK) + 12u: case C(FINEK2) + 3u:
        CALL(C(BANKCALL), z + 1u);
    case C(IMUZEROK) + 5u: case C(OPTC1) + 4u: case C(IMUFINEK) + 6u: case C(IMUATTCK) + 15u:
        CALL(z + 1u, z + 1u);                                          /* TC +1 */
    case C(IMUZEROK) + 6u: case C(TRMATTCK) + 2u: CALL(C(GOPIN), z + 1u);
    case C(IMUCOARK) + 1u: case C(IMUFINEK) + 1u: case C(OPTCOARK) + 2u: case C(IMUATTCK) + 7u:
        CALL(C(TESTXACT), z + 1u);
    case C(IMUCOARK) + 2u: lanec_ev_ca(s, CW(VNLODCDU)); CALL(C(BANKCALL), z + 2u);
    case C(IMUCOARK) + 5u: case C(IMUFINEK) + 5u: case C(OPTC1) + 3u: case C(OPTCOARK) + 17u:
        CALL(C(ENDEXT), z + 1u);
    case C(IMUCOARK) + 6u: JUMP(C(ICORK2));
    case C(ICORK2): lanec_ev_ca(s, CW(IMUCOARV)); CALL(C(BANKCALL), z + 2u);
    case C(ICORK2) + 7u: case C(ICORK2) + 8u: case C(IMUFINEK) + 14u: case C(FINEK2) + 5u: case C(FINEK2) + 6u:
    case C(OPTC1) + 15u:
        CALL(C(ENDEXT), z + 1u);
    /* OPTCOARK: CA OPTCADR, TC CKMODCAD +1; +2 TC TESTXACT; +3 CAF EBANK5, TS EBANK, CCS SWSAMPLE */
    case C(OPTCOARK): lanec_ev_ca(s, w[C(OPTCADR)]); CALL(C(CKMODCAD) + 1u, z + 2u);
    case C(OPTCOARK) + 3u:
        lanec_ev_ca(s, CW(EBANK5));
        lanec_ev_set_eb(s, w[LANEC_A]);
        switch (lanec_ccs(s, w[C(SWSAMPLE)])) {
        case 0: CALL(z + 8u, z + 4u);                                  /* TC +5 */
        case 1: lanec_tc(s, z + 5u, z + 5u); CALL(C(FALTON), z + 6u);  /* TC +1, TC FALTON */
        case 2: CALL(C(FALTON), z + 6u);
        default: CALL(C(ALARM), z + 7u);                               /* OCT 00115 follows */
        }
    case C(OPTCOARK) + 9u: CALL(C(ALARM), z + 1u);                    /* FALTON returns here */
    case C(OPTCOARK) + 11u:                    /* CCS OPTIND, TC OPTC1 (x3), TC ALARM */
        k = lanec_ccs(s, w[C(OPTIND)]);
        if (k < 3u) CALL(C(OPTC1), z + 2u + k);
        CALL(C(ALARM), z + 5u);
    /* OPTC1: CAF VNLD0CDU, BANKCALL GOXDSPF; +3 TC TERMEXTV; +4 TC +1; +5 CA SAC,
     * TS DESOPTS, CA PAC, TS DESOPTT, CAF OPTCOARV, BANKCALL EXDSPRET; +12 CAF ONE,
     * TS OPTIND, TC ENDEXTVB */
    case C(OPTC1): lanec_ev_ca(s, CW(VNLD0CDU)); CALL(C(BANKCALL), z + 2u);
    case C(OPTC1) + 5u:
        lanec_ev_ca(s, w[E(C(SAC))]); (void)lanec_ts(s, C(DESOPTS));
        lanec_ev_ca(s, w[E(C(PAC))]); (void)lanec_ts(s, C(DESOPTT));
        lanec_ev_ca(s, CW(IMUCOARV));
        CALL(C(BANKCALL), z + 6u);
    case C(OPTC1) + 12u:
        lanec_ev_ca(s, CW(ONE)); (void)lanec_ts(s, C(OPTIND));
        CALL(C(ENDEXT), z + 3u);
    case C(IMUFINEK) + 2u: lanec_ev_ca(s, CW(VNLODGYR)); CALL(C(BANKCALL), z + 2u);
    case C(IMUFINEK) + 7u: lanec_ev_ca(s, CW(IMUFINEV)); CALL(C(BANKCALL), z + 2u);
    case C(FINEK2): lanec_ev_ca(s, CW(LGYROBIN)); CALL(C(BANKCALL), z + 2u);
    /* IMUATTCK: TC CHKPOOH; +1 CAF OCTAL30, EXTEND, RAND CHAN12, CCS A, TCF ALM/END, TC CKLFTBTS */
    case C(IMUATTCK): CALL(C(CHKPOOH), z + 1u);
    case C(IMUATTCK) + 1u:
        lanec_ev_ca(s, CW(OCTAL30));
        lanec_ev_rand(s, io, 012u);
        if (lanec_ccs_a(s) == 0) JUMP(C(ALM_S_END));
        CALL(C(CKLFTBTS), z + 6u);
    case C(IMUATTCK) + 8u:                     /* CS OCT50, EXTEND, WAND CHAN12, CAF VNLODCDU, BANKCALL */
        lanec_ev_cs(s, CW(OCT50));
        lanec_ev_wand(s, io, 012u);
        lanec_ev_ca(s, CW(VNLODCDU));
        CALL(C(BANKCALL), z + 5u);
    case C(IMUATTCK) + 14u: JUMP(C(TRMATTCK));
    case C(IMUATTCK) + 16u:                    /* CAF EBANK6, TS EBANK, BANKCALL NEEDLE11 */
        lanec_ev_ca(s, CW(EBANK6));
        lanec_ev_set_eb(s, w[LANEC_A]);
        CALL(C(BANKCALL), z + 3u);
    case C(IMUATTCK) + 20u: CALL(C(BANKCALL), z + 1u);
    case C(IMUATTCK) + 22u: lanec_ev_ca(s, CW(TWO)); CALL(C(WAITLIST), z + 2u);
    case C(TRMATTCK): CALL(C(CKLFTBTS), z + 1u);
    case C(TRMATTCK) + 1u: JUMP(C(ENDEXT));
    /* ATTCK1 (task): EXTEND, DCA THETAD, DXCH AK, CAE THETAD +2, TS AK +2, TC IBNKCALL */
    case C(ATTCK1):
        a = w[E(C(THETAD))]; l = w[E(C(THETAD) + 1u)];
        w[LANEC_A] = lanec_a16(w[E(C(AK))]); lanec_ev_set_l(s, lanec_a16(w[E(C(AK) + 1u)]));
        w[E(C(AK))] = a; w[E(C(AK) + 1u)] = l;
        lanec_ev_ca(s, w[E(C(THETAD) + 2u)]);
        (void)lanec_ts(s, E(C(AK) + 2u));
        CALL(C(IBNKCALL), z + 6u);
    case C(ATTCK1) + 7u: CALL(C(TASKOVER), z + 1u);
    /* CKLFTBTS: CAF GRRBKBIT, MASK FLAGWRD5, CCS A, TC Q; CAF BIT5, EXTEND, RAND CHAN30,
     * CCS A, TCF Q+1, (XACT0Q) TC Q */
    case C(CKLFTBTS):
        lanec_ev_ca(s, CW(GRRBKBIT));
        lanec_mask(s, w[C(FLAGWRD5)]);
        if (lanec_ccs_a(s) == 0) JUMP(LANEC_Q);
        lanec_ev_ca(s, CW(BIT5));
        lanec_ev_rand(s, io, 030u);
        if (lanec_ccs_a(s) == 0) JUMP(C(Q_P_1));
        JUMP(C(XACT0Q));
    default: return NOT_ENTRY;
    }
}

static uint8_t listed(const uint16_t *list, uint16_t n, uint16_t key)
{
    uint16_t i;
    for (i = 0; i < n; ++i)
        if (list[i] == key) return 1;
    return 0;
}

#define COUNT(X) ((uint16_t)(sizeof(X) / sizeof((X)[0])))

const uint16_t *lanec_ev_align_entries(enum lanec_mission m, uint16_t *count)
{
    *count = m == LANEC_LM ? COUNT(lm_entries) : COUNT(cm_entries);
    return m == LANEC_LM ? lm_entries : cm_entries;
}

static uint16_t key_of(enum lanec_mission m, uint16_t bank, uint16_t z)
{
    if (bank == 043u) return z;
    return m == LANEC_LM && bank == 023u ? (uint16_t)(B23 | z) : NOT_ENTRY;
}

static uint8_t entry(enum lanec_mission m, uint16_t key)
{
    return m == LANEC_LM ? listed(lm_entries, COUNT(lm_entries), key) : listed(cm_entries, COUNT(cm_entries), key);
}

enum lanec_status lanec_ev_align_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, bank, key, k;
    if (!lanec_registers_valid(s)) return LANEC_OUT_OF_SCOPE;
    bank = lanec_current_bank(s, io);
    key = key_of(m, bank, w[LANEC_Z]);
    if (key == NOT_ENTRY || !entry(m, key)) return LANEC_OUT_OF_SCOPE;
    /* RRDESK2 +3 stores into the job's VAC word: only an ordinary unswitched one. */
    if (m == LANEC_LM && key == L(RRDESK2) + 3u) {
        k = (uint16_t)(w[L(PRIORITY)] & LW(LOW9));
        if (k != 0 && k < 062u) return LANEC_OUT_OF_SCOPE;
    }
    do {
        key = m == LANEC_LM ? lm_step(s, io, key) : cm_step(s, io, key);
        w[LANEC_Z] = (uint16_t)(key & 07777u);
        if (key == NOT_ENTRY) break;
        key = key_of(m, bank, (uint16_t)(key & 07777u));
    } while (key != NOT_ENTRY && entry(m, key));
    return LANEC_OK;
}
