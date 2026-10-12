/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS V55 clock update (Luminary099 R33 in bank 42, Comanche055
 * ALINTIME in bank 43) and the Luminary099 V43 ATTCK2 meter task (bank 42).
 * Each case is one basic block of the source, entered at its address; see
 * docs/LANEC-EV-TIME.md. */
#include "lanec_ev_time.h"
#include "lanec_ev_internal.h"
#include "lanec_symbols.h"

#define CALL(T, R) return lanec_ev_call(s, (T), (uint16_t)(R))
#define JUMP(T) return (uint16_t)(T)
#define L(X) LANEC_LM_##X
#define LW(X) LANEC_LM_##X##_WORD
#define C(X) LANEC_CM_##X
#define CW(X) LANEC_CM_##X##_WORD
#define NOT_ENTRY 0xffffu

static const uint16_t lm[] = {
    L(R33), L(R33) + 2u, L(R33) + 5u, L(R33) + 6u, L(R33) + 7u, L(UPDATIME), L(UPDATIME) + 9u, L(UPDTMEND),
    L(ATTCK2), L(ATTCK2) + 1u
};
static const uint16_t cm[] = {
    C(ALINTIME), C(ALINTIME) + 1u, C(ALINTIME) + 4u, C(ALINTIME) + 5u, C(ALINTIME) + 6u, C(UPDATIME),
    C(UPDATIME) + 9u, C(UPDTMEND)
};

#define COUNT(X) ((uint16_t)(sizeof(X) / sizeof((X)[0])))

/* CS DEC23, AD MPAC, BZF UPDATIME: only V23 (data in) updates the clock */
static uint8_t data_in(apollo_overlay_state *s, uint16_t dec23, uint16_t mpac)
{
    lanec_ev_cs(s, dec23);
    lanec_ad(s, s->words[mpac]);
    return lanec_ev_zero(s);
}

/* UPDATIME to TC TPAGREE: MPAC = TIME2,TIME1 + DSPTEM2 +1,+2, MPAC +2 = 0,
 * TIME2,TIME1 = 0 */
static void updatime(apollo_overlay_state *s, lanec_io *io, uint16_t mpac, uint16_t time2, uint16_t dsptem2)
{
    io->interrupt_enabled = 0;
    s->words[LANEC_A] = 0;
    (void)lanec_ts(s, (uint16_t)(mpac + 2u));
    s->words[LANEC_L] = 0;
    lanec_ev_dxch(s, time2);
    lanec_ev_dxch(s, mpac);
    lanec_ev_dxch(s, (uint16_t)(dsptem2 + 1u));
    lanec_ev_das(s, mpac);
}

/* after TPAGREE: DXCH MPAC, DAS TIME2, RELINT */
static void newclock(apollo_overlay_state *s, lanec_io *io, uint16_t mpac, uint16_t time2)
{
    lanec_ev_dxch(s, mpac);
    lanec_ev_das(s, time2);
    io->interrupt_enabled = 1;
}

static uint16_t lm_step(apollo_overlay_state *s, lanec_io *io, uint16_t z)
{
    uint16_t *w = s->words, q;
    switch (z) {
    case L(R33): lanec_ev_ca(s, LW(PRIO7)); CALL(L(PRIOCHNG), z + 2u);
    case L(R33) + 2u: lanec_ev_ca(s, LW(VNLODDT)); CALL(L(BANKCALL), z + 2u);
    case L(R33) + 5u: case L(R33) + 6u: case L(UPDTMEND): CALL(L(ENDEXT), z + 1u);
    case L(R33) + 7u:
        if (data_in(s, LW(DEC23), L(MPAC))) JUMP(L(UPDATIME));
        CALL(L(ENDEXT), z + 5u);
    case L(UPDATIME): updatime(s, io, L(MPAC), L(TIME2), L(DSPTEM2)); CALL(L(TPAGREE), z + 9u);
    case L(UPDATIME) + 9u: newclock(s, io, L(MPAC), L(TIME2)); JUMP(L(UPDTMEND));
    /* ATTCK2: THETAD x, y, z times 0.1 into CDUZCMD, CDUYCMD, CDUXCMD, then
     * enable the CDU drives */
    case L(ATTCK2): lanec_ev_ca(s, LW(TWO)); JUMP(z + 1u);
    case L(ATTCK2) + 1u:
        w[LANEC_Q] = w[LANEC_A];
        lanec_ev_ca(s, w[L(THETAD) + w[LANEC_A]]);
        lanec_ev_mp(s, LW(ATTSCALE));
        q = w[LANEC_Q];
        {
            uint16_t old = w[L(CDUXCMD) + q];
            w[L(CDUXCMD) + q] = lanec_ev_corrected(w[LANEC_A]);
            w[LANEC_A] = lanec_ev_sx(old);
        }
        if (lanec_ccs(s, w[LANEC_Q]) == 0) JUMP(z);
        lanec_ev_ca(s, LW(13_14_15));
        lanec_ev_wor(s, io, 014u);
        JUMP(L(TASKOVER));
    default: return NOT_ENTRY;
    }
}

static uint16_t cm_step(apollo_overlay_state *s, lanec_io *io, uint16_t z)
{
    switch (z) {
    case C(ALINTIME): CALL(C(TESTXACT), z + 1u);
    case C(ALINTIME) + 1u: lanec_ev_ca(s, CW(VNLODDT)); CALL(C(BANKCALL), z + 2u);
    case C(ALINTIME) + 4u: case C(ALINTIME) + 5u: case C(UPDTMEND): CALL(C(ENDEXT), z + 1u);
    case C(ALINTIME) + 6u:
        if (data_in(s, CW(DEC23), C(MPAC))) JUMP(C(UPDATIME));
        CALL(C(ENDEXT), z + 5u);
    case C(UPDATIME): updatime(s, io, C(MPAC), C(TIME2), C(DSPTEM2)); CALL(C(TPAGREE), z + 9u);
    case C(UPDATIME) + 9u: newclock(s, io, C(MPAC), C(TIME2)); JUMP(C(UPDTMEND));
    default: return NOT_ENTRY;
    }
}

const uint16_t *lanec_ev_time_entries(enum lanec_mission m, uint16_t *count)
{
    if (m == LANEC_LM) { *count = COUNT(lm); return lm; }
    *count = COUNT(cm);
    return cm;
}

static uint8_t entry(enum lanec_mission m, uint16_t z)
{
    uint16_t i, n;
    const uint16_t *e = lanec_ev_time_entries(m, &n);
    for (i = 0; i < n; ++i)
        if (e[i] == z) return 1;
    return 0;
}

enum lanec_status lanec_ev_time_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, z = w[LANEC_Z];
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != (m == LANEC_LM ? 042u : 043u) || !entry(m, z))
        return LANEC_OUT_OF_SCOPE;
    /* ATTCK2 +1 is entered with the axis index 0, 1 or 2 its loop leaves in A */
    if (m == LANEC_LM && z == L(ATTCK2) + 1u && w[LANEC_A] > 2u) return LANEC_OUT_OF_SCOPE;
    do {
        z = m == LANEC_LM ? lm_step(s, io, z) : cm_step(s, io, z);
        w[LANEC_Z] = z;
    } while (entry(m, z));
    return LANEC_OK;
}
