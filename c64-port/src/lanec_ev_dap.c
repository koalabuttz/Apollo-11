/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS DAP data load, V48: Luminary099 R03 (DAPDATA1 in bank 20,
 * STIKLOAD through TRIMDONE in bank 01) and Comanche055 DAPDISP (bank 43)
 * with DAPDISP1 (bank 42). Each case is one basic block of the source,
 * entered at its address; see docs/LANEC-EV-DAP.md. */
#include "lanec_ev_dap.h"
#include "lanec_ev_internal.h"
#include "lanec_symbols.h"

#define NOT_ENTRY 0xffffu
#define CALL(T, R) return lanec_ev_call(s, (T), (uint16_t)(R))
#define JUMP(T) return (uint16_t)(T)
#define E(X) lanec_erasable(s, (X))
#define L(X) LANEC_LM_##X
#define LW(X) LANEC_LM_##X##_WORD
#define C(X) LANEC_CM_##X
#define CW(X) LANEC_CM_##X##_WORD

static const uint16_t lm20[] = {
    L(DAPDATA1), L(CHKDATA1), L(MSKDATR1), L(MSKDATR1) + 5u, L(MSKDATR1) + 6u, L(MSKDATR1) + 7u,
    L(MSKDATR1) + 8u, L(MSKDATR1) + 10u, L(FORCEONE), L(DPDAT1)
};
static const uint16_t lm01[] = {
    L(STIKLOAD), L(DAPDATA2), L(DAPDATA2) + 3u, L(DAPDATA2) + 4u, L(DAPDATA2) + 5u, L(DAPDATA2) + 6u,
    L(DAPDATA2) + 8u, L(ENDR03), L(ENDR03) + 3u, L(DAPDAT2), L(LEMALONE), L(LEMALONE) + 6u, L(DAPDATA3),
    L(DAPDATA3) + 4u, L(DAPDATA3) + 7u, L(DAPDATA3) + 8u, L(DAPDATA3) + 9u, L(DAPDATA3) + 10u,
    L(DAPDATA3) + 12u, L(DPDAT3), L(DPDAT3) + 5u, L(TRIMDONE), L(TRIMDONE) + 3u, L(TRIMDONE) + 4u,
    L(TRIMDONE) + 5u, L(TRIMDONE) + 6u, L(TRIMDONE) + 8u
};
static const uint16_t cm43[] = { C(DAPDISP), C(DAPDISP) + 5u, C(DAPDISP) + 6u };
static const uint16_t cm42[] = {
    C(DAPDISP1), C(DONOUN46), C(DONOUN46) + 3u, C(DONOUN46) + 4u, C(DONOUN46) + 5u, C(DONOUN46) + 6u,
    C(MAXOUT), C(DONOUN47), C(DONOUN47) + 3u, C(DONOUN47) + 4u, C(DONOUN47) + 5u, C(DONOUN47) + 6u,
    C(DONOUN47) + 17u, C(DONOUN48), C(DONOUN48) + 3u, C(DONOUN48) + 4u, C(DONOUN48) + 5u, C(MAXIN),
    C(MAXIN) + 2u
};

#define COUNT(X) ((uint16_t)(sizeof(X) / sizeof((X)[0])))

/* CAF Vnnnn, TC BANKCALL (a GOXDSPF/GOMARK CADR follows) */
static uint16_t display(apollo_overlay_state *s, uint16_t vn, uint16_t at)
{
    lanec_ev_ca(s, vn);
    CALL(LANEC_LM_BANKCALL, at + 2u);
}

static uint16_t lm_step(apollo_overlay_state *s, lanec_io *io, uint16_t z)
{
    uint16_t *w = s->words;
    switch (z) {
    /* DAPDATA1: DAPDATR1 = DAPBOOLS display bits, bit 14 = APSFLAG complement */
    case L(DAPDATA1):
        lanec_ev_ca(s, LW(BOOLSMSK)); lanec_mask(s, w[L(DAPBOOLS)]); (void)lanec_ts(s, L(DAPDATR1));
        lanec_ev_cs(s, w[L(FLGWRD10)]); lanec_mask(s, LW(APSFLBIT));
        switch (lanec_ccs_a(s)) {
        case 0: lanec_ev_ca(s, LW(BIT14)); /* fall through */
        case 1: lanec_ev_ads(s, L(DAPDATR1)); /* fall through */
        default: break;
        }
        JUMP(L(CHKDATA1));
    case L(CHKDATA1):
        lanec_ev_ca(s, w[L(DAPDATR1)]); lanec_mask(s, LW(BIT13_M_14));
        if (lanec_ev_zero(s)) JUMP(L(FORCEONE));
        lanec_ev_ca(s, w[L(DAPDATR1)]);
        JUMP(L(MSKDATR1));
    case L(MSKDATR1):
        lanec_mask(s, LW(DSPLYMSK)); (void)lanec_ts(s, L(DAPDATR1));
        return display(s, LW(V01N46), z + 2u);
    case L(MSKDATR1) + 5u: JUMP(L(ENDEXT));
    case L(MSKDATR1) + 6u: JUMP(L(DPDAT1));
    case L(MSKDATR1) + 7u: JUMP(L(CHKDATA1));
    case L(MSKDATR1) + 8u: lanec_ev_ca(s, LW(REVCNT)); CALL(L(BLANKET), z + 2u);
    case L(MSKDATR1) + 10u: JUMP(L(ENDOFJOB));
    case L(FORCEONE): lanec_ev_ca(s, LW(BIT13)); lanec_ev_ads(s, L(DAPDATR1)); JUMP(L(MSKDATR1));
    /* DPDAT1: APSFLAG, DAPBOOLS, MASS, NJETSFLAG and RATEINDX from the display */
    case L(DPDAT1):
        io->interrupt_enabled = 0;
        lanec_ev_cs(s, LW(APSFLBIT)); lanec_mask(s, w[L(FLGWRD10)]); lanec_ev_set_l(s, w[LANEC_A]);
        lanec_ev_cs(s, w[L(DAPDATR1)]); lanec_mask(s, LW(BIT14));
        switch (lanec_ccs_a(s)) {
        case 0: lanec_ev_ca(s, LW(APSFLBIT)); /* fall through */
        case 1: lanec_ad(s, w[LANEC_L]); /* fall through */
        case 2: (void)lanec_ts(s, L(FLGWRD10)); /* fall through */
        default: break;
        }
        lanec_ev_cs(s, w[L(DAPDATR1)]); lanec_mask(s, LW(BIT13_M_14));
        switch (lanec_ccs_a(s)) {
        case 0: lanec_ev_cs(s, LW(CSMDOCKD)); /* fall through */
        case 1: lanec_ad(s, LW(BOOLSMSK)); /* fall through */
        default: break;
        }
        lanec_mask(s, w[L(DAPDATR1)]); lanec_ev_set_l(s, w[LANEC_A]);
        lanec_ev_cs(s, LW(BOOLSMSK)); lanec_mask(s, w[L(DAPBOOLS)]); lanec_ad(s, w[LANEC_L]);
        (void)lanec_ts(s, L(DAPBOOLS));
        lanec_mask(s, LW(CSMDOCKD));
        switch (lanec_ccs_a(s)) {
        case 0: lanec_ev_ca(s, w[L(CSMMASS)]); /* fall through */
        case 1: lanec_ad(s, w[L(LEMMASS)]); /* fall through */
        default: break;
        }
        /* TS MASS skips CAE DAPBOOLS when it overflows */
        if (!lanec_ts(s, L(MASS))) lanec_ev_ca(s, w[L(DAPBOOLS)]);
        lanec_mask(s, LW(ACC4OR2X));
        if (lanec_ev_zero(s)) {
            lanec_ev_cs(s, w[L(FLAGWRD1)]); lanec_mask(s, LW(BIT15)); lanec_ev_ads(s, L(FLAGWRD1));
        } else {
            lanec_ev_cs(s, LW(BIT15)); lanec_mask(s, w[L(FLAGWRD1)]); (void)lanec_ts(s, L(FLAGWRD1));
        }
        lanec_ev_ca(s, w[L(DAPBOOLS)]); lanec_mask(s, LW(THREE));
        w[LANEC_A] = lanec_ev_add16(w[LANEC_A], w[LANEC_A]);
        (void)lanec_ts(s, L(RATEINDX));
        CALL(L(POSTJUMP), z + 052u);
    /* STIKLOAD: stick sensitivity and rate deadband, divided by 10 when docked */
    case L(STIKLOAD):
        lanec_ev_ca(s, LW(EBANK6)); lanec_ev_set_eb(s, w[LANEC_A]);
        lanec_ev_ca(s, LW(RHCSCALE)); lanec_mask(s, w[L(DAPBOOLS)]);
        switch (lanec_ccs_a(s)) {
        case 0: lanec_ev_ca(s, LW(NORMAL)); /* fall through */
        case 1: lanec_ad(s, LW(FINE)); /* fall through */
        case 2: (void)lanec_ts(s, E(L(STIKSENS))); /* fall through */
        default: break;
        }
        lanec_ev_ca(s, LW(_M_0_D_6D_S_S)); (void)lanec_ts(s, E(L(_M_RATEDB)));
        lanec_ev_ca(s, LW(CSMDOCKD)); lanec_mask(s, w[L(DAPBOOLS)]);
        if (!lanec_ev_zero(s)) {
            lanec_ev_ca(s, w[E(L(STIKSENS))]); lanec_ev_mp(s, LW(1_S_10)); (void)lanec_ts(s, E(L(STIKSENS)));
            lanec_ev_ca(s, LW(_M_0_D_3D_S_S)); (void)lanec_ts(s, E(L(_M_RATEDB)));
        }
        io->interrupt_enabled = 1;
        JUMP(L(DAPDATA2));
    case L(DAPDATA2): return display(s, LW(V0647), z);
    case L(DAPDATA2) + 3u: JUMP(L(ENDR03));
    case L(DAPDATA2) + 4u: JUMP(L(DAPDAT2));
    case L(DAPDATA2) + 5u: JUMP(L(DAPDATA2));
    case L(DAPDATA2) + 6u: case L(DAPDATA3) + 10u: lanec_ev_ca(s, LW(BIT3)); CALL(L(BLANKET), z + 2u);
    case L(DAPDATA2) + 8u: case L(DAPDATA3) + 12u: case L(DPDAT3) + 5u: case L(TRIMDONE) + 8u:
        JUMP(L(ENDOFJOB));
    case L(ENDR03): io->interrupt_enabled = 0; CALL(L(IBNKCALL), z + 2u);
    case L(ENDR03) + 3u: JUMP(L(ENDEXT));
    /* DAPDAT2: masses must exceed their minimums, then MASS and RESTORDB */
    case L(DAPDAT2):
        lanec_ev_cs(s, w[L(FLGWRD10)]); lanec_mask(s, LW(APSFLBIT));
        switch (lanec_ccs_a(s)) {
        case 0: lanec_ev_ca(s, LW(MINLMD)); /* fall through */
        case 1: lanec_ad(s, LW(MINMINLM)); /* fall through */
        case 2: lanec_ad(s, w[L(LEMMASS)]); break;
        default: break;
        }
        if (lanec_ev_nonpositive(s)) JUMP(L(DAPDATA2));
        lanec_ev_ca(s, w[L(DAPBOOLS)]); lanec_mask(s, LW(CSMDOCKD));
        if (lanec_ev_zero(s)) JUMP(L(LEMALONE));
        lanec_ev_cs(s, LW(MINCSM)); lanec_ad(s, w[L(CSMMASS)]);
        if (lanec_ev_nonpositive(s)) JUMP(L(DAPDATA2));
        lanec_ev_ca(s, w[L(CSMMASS)]);
        JUMP(L(LEMALONE));
    case L(LEMALONE): {
        uint16_t hi, lo;
        lanec_ad(s, w[L(LEMMASS)]);
        w[LANEC_L] = 0;
        hi = w[L(MASS)]; lo = w[L(MASS) + 1u];
        w[L(MASS) + 1u] = 0; w[L(MASS)] = lanec_ev_corrected(w[LANEC_A]);
        w[LANEC_L] = lanec_ev_sx(lo); w[LANEC_A] = lanec_ev_sx(hi);
        io->interrupt_enabled = 0;
        CALL(L(IBNKCALL), z + 5u);
    }
    case L(LEMALONE) + 6u: io->interrupt_enabled = 1; JUMP(L(DAPDATA3));
    case L(DAPDATA3):
        lanec_ev_cs(s, w[L(FLGWRD10)]); lanec_mask(s, LW(APSFLBIT));
        if (lanec_ev_zero(s)) JUMP(L(ENDEXT));
        JUMP(z + 4u);
    case L(DAPDATA3) + 4u: return display(s, LW(V06N48), z);
    case L(DAPDATA3) + 7u: CALL(L(ENDEXT), z + 1u);
    case L(DAPDATA3) + 8u: JUMP(L(DPDAT3));
    case L(DAPDATA3) + 9u: JUMP(z - 5u);                               /* TCF -5 */
    case L(DPDAT3): lanec_ev_ca(s, LW(BIT1)); io->interrupt_enabled = 0; CALL(L(WAITLIST), z + 3u);
    case L(TRIMDONE): return display(s, LW(V50N48), z);
    case L(TRIMDONE) + 3u: case L(TRIMDONE) + 4u: case L(TRIMDONE) + 5u: CALL(L(ENDEXT), z + 1u);
    case L(TRIMDONE) + 6u: lanec_ev_ca(s, LW(OCT24)); CALL(L(BLANKET), z + 2u);
    default: return NOT_ENTRY;
    }
}

static uint16_t cm_step(apollo_overlay_state *s, lanec_io *io, uint16_t z)
{
    uint16_t *w = s->words;
    switch (z) {
    /* DAPDISP: refused while the TVC DAP runs (FLAGWRD6 bits 15-14 = 10) */
    case C(DAPDISP):
        lanec_ev_cs(s, w[C(FLAGWRD6)]); lanec_mask(s, CW(OCT60000));
        if (lanec_ev_nonpositive(s)) JUMP(z + 5u);
        CALL(C(ALM_S_END), z + 5u);
    case C(DAPDISP) + 5u: CALL(C(TESTXACT), z + 1u);
    case C(DAPDISP) + 6u: CALL(C(BANKCALL), z + 1u);
    case C(DAPDISP1):
        lanec_ev_ca(s, CW(EBANK6)); lanec_ev_set_eb(s, w[LANEC_A]);
        lanec_ev_ca(s, CW(PRIO10));
        CALL(C(PRIOCHNG), z + 4u);
    case C(DONOUN46): lanec_ev_ca(s, CW(V04N46)); CALL(C(BANKCALL), z + 2u);
    case C(DONOUN47): lanec_ev_ca(s, CW(V06N47)); CALL(C(BANKCALL), z + 2u);
    case C(DONOUN48): lanec_ev_ca(s, CW(V0648)); CALL(C(BANKCALL), z + 2u);
    case C(DONOUN46) + 3u: case C(DONOUN47) + 3u: case C(DONOUN48) + 3u: case C(DONOUN48) + 4u:
        CALL(C(ENDEXT), z + 1u);
    case C(DONOUN46) + 4u: case C(DONOUN47) + 4u: CALL(z + 2u, z + 1u);   /* TC +2 */
    case C(DONOUN46) + 5u: CALL(C(DONOUN46), z + 1u);
    case C(DONOUN47) + 5u: CALL(C(DONOUN47), z + 1u);
    case C(DONOUN48) + 5u: CALL(C(DONOUN48), z + 1u);
    /* DAPDATR1 bit 4: maximum deadband flag */
    case C(DONOUN46) + 6u:
        lanec_ev_ca(s, w[E(C(DAPDATR1))]); lanec_mask(s, CW(BIT4));
        switch (lanec_ccs_a(s)) {
        case 0: JUMP(C(MAXIN));
        default: CALL(C(DOWNFLAG), z + 5u);                         /* BIT4 is never negative */
        }
    case C(MAXOUT): CALL(C(BANKCALL), z + 1u);
    /* mass properties unless the configuration is 0, 3, 4 or 7 */
    case C(DONOUN47) + 6u:
        lanec_ev_ca(s, w[E(C(DAPDATR1))]); lanec_mask(s, CW(PRIO30));
        if (lanec_ev_zero(s)) JUMP(C(DONOUN48));
        w[LANEC_A] = (uint16_t)~w[LANEC_A]; lanec_mask(s, CW(PRIO30));
        if (lanec_ev_zero(s)) JUMP(C(DONOUN48));
        io->interrupt_enabled = 0;
        CALL(C(IBNKCALL), z + 012u);
    case C(DONOUN47) + 17u: io->interrupt_enabled = 1; CALL(C(BANKCALL), z + 2u);
    case C(MAXIN): CALL(C(UPFLAG), z + 1u);
    case C(MAXIN) + 2u: CALL(C(MAXOUT), z + 1u);
    default: return NOT_ENTRY;
    }
}

const uint16_t *lanec_ev_dap_entries(enum lanec_mission m, uint16_t bank, uint16_t *count)
{
    if (m == LANEC_LM && bank == 020u) { *count = COUNT(lm20); return lm20; }
    if (m == LANEC_LM && bank == 01u) { *count = COUNT(lm01); return lm01; }
    if (m == LANEC_CM && bank == 043u) { *count = COUNT(cm43); return cm43; }
    if (m == LANEC_CM && bank == 042u) { *count = COUNT(cm42); return cm42; }
    *count = 0;
    return lm20;
}

static uint8_t entry(enum lanec_mission m, uint16_t bank, uint16_t z)
{
    uint16_t i, n;
    const uint16_t *e = lanec_ev_dap_entries(m, bank, &n);
    for (i = 0; i < n; ++i)
        if (e[i] == z) return 1;
    return 0;
}

enum lanec_status lanec_ev_dap_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, z = w[LANEC_Z], bank;
    if (!lanec_registers_valid(s)) return LANEC_OUT_OF_SCOPE;
    bank = lanec_current_bank(s, io);
    if (!entry(m, bank, z)) return LANEC_OUT_OF_SCOPE;
    do {
        z = m == LANEC_LM ? lm_step(s, io, z) : cm_step(s, io, z);
        w[LANEC_Z] = z;
    } while (entry(m, bank, z));
    return LANEC_OK;
}
