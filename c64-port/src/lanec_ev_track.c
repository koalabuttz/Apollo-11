/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS basic code around the interpretive tracking and
 * state-vector verbs: V69 (restart), LM V56 TRMTRAK1 (bank 42), V85
 * DSPRRLOS/RRLOSDSP (bank 40) and ATTACHIT (bank 43); CM V56 TRACKTRM and
 * CLEANOUT, SETVHFLG/RESETVHF, ATTACHIT/TACHEXIT and LMTOCMSV/LMTOCM
 * (bank 43). Each TC INTPRET hands the interpretive words that follow to the
 * interpreter; the word after their EXIT is an entry here. See
 * docs/LANEC-EV-TRACK.md. */
#include "lanec_ev_track.h"
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

static const uint16_t lm40[] = {
    L(DSPRRLOS), L(DSPRRLOS) + 4u, L(DSPRRLOS) + 6u, L(DSPRRLOS) + 9u, L(DSPRRLOS) + 10u, L(DSPRRLOS) + 11u,
    L(DSPRRLOS) + 12u, L(DSPRRLOS) + 14u, L(RRLOSDSP), L(RRLOSDSP) + 39u, L(RRLOSDSP) + 42u
};
static const uint16_t lm42[] = {
    L(TRMTRAK1), L(TRMTRAK1) + 2u, L(TRMTRAK1) + 4u, L(TRMTRAK1) + 6u, L(TRMTRAK1) + 10u, L(TRMTRAK1) + 12u,
    L(TRMTRAK1) + 15u, L(TRMTRAK1) + 17u, L(TRMTRAK1) + 18u
};
static const uint16_t lm43[] = { L(VERB69), L(ATTACHIT), L(ATTACHIT) + 10u, L(ATTACHIT) + 14u, L(ATTACHIT) + 22u };
static const uint16_t cm43[] = {
    C(VERB69), C(TRACKTRM), C(TRACKTRM) + 6u, C(TRACKTRM) + 12u, C(TRACKTRM) + 14u, C(TRACKTRM) + 16u,
    C(TRACKTRM) + 20u, C(TRACKTRM) + 26u, C(CLEANOUT), C(SETVHFLG), C(SETVHFLG) + 3u, C(RESETVHF),
    C(RESETVHF) + 3u, C(RESETVHF) + 4u, C(ATTACHIT), C(ATTACHIT) + 10u, C(TACHEXIT), C(TACHEXIT) + 10u,
    C(LMTOCMSV), C(LMTOCMSV) + 4u, C(LMTOCM), C(LMTOCM) + 10u, C(LMTOCM) + 14u
};

#define COUNT(X) ((uint16_t)(sizeof(X) / sizeof((X)[0])))

/* CAF TCPINAD, INDEX FIXLOC, TS QPRET (in the job's VAC area), TC POSTJUMP */
static uint16_t tcpin(apollo_overlay_state *s, uint16_t tcpinad, uint16_t fixloc, uint16_t qpret,
                      uint16_t postjump, uint16_t z)
{
    lanec_ev_ca(s, tcpinad);
    (void)lanec_ts(s, lanec_erasable(s, (uint16_t)(qpret + s->words[fixloc])));
    CALL(postjump, z + 4u);
}

/* V85 (bank 40): RR LOS display job */
static uint16_t lm40_step(apollo_overlay_state *s, uint16_t z)
{
    uint16_t *w = s->words;
    switch (z) {
    case L(DSPRRLOS): lanec_ev_ca(s, LW(PRIO5)); CALL(L(FINDVAC), z + 2u);
    case L(DSPRRLOS) + 4u: lanec_ev_ca(s, LW(PRIO4)); CALL(L(PRIOCHNG), z + 2u);
    case L(DSPRRLOS) + 6u: lanec_ev_ca(s, LW(V16N56)); CALL(L(BANKCALL), z + 2u);
    case L(DSPRRLOS) + 9u: case L(DSPRRLOS) + 10u: case L(DSPRRLOS) + 11u: CALL(L(B5OFF), z + 1u);
    case L(DSPRRLOS) + 12u: lanec_ev_ca(s, LW(BIT3)); CALL(L(BLANKET), z + 2u);
    case L(DSPRRLOS) + 14u: CALL(L(ENDOFJOB), z + 1u);
    case L(RRLOSDSP): {
        uint16_t lo = w[L(CDUT) + 1u], hi = w[L(CDUT)];
        w[LANEC_L] = lanec_ev_sx(lo); w[LANEC_A] = lanec_ev_sx(hi);   /* DCA CDUT */
        lanec_ev_dxch(s, L(MPAC));
        CALL(L(INTPRET), z + 4u);
    }
    case L(RRLOSDSP) + 39u: lanec_ev_ca(s, LW(1SEC)); CALL(L(BANKCALL), z + 2u);
    case L(RRLOSDSP) + 42u:
        lanec_ev_ca(s, LW(BIT5)); lanec_mask(s, w[L(EXTVBACT)]);
        if (lanec_ccs_a(s) == 0) CALL(L(RRLOSDSP), z + 4u);          /* BIT5 is never negative */
        CALL(L(ENDEXT), z + 5u);
    default: return NOT_ENTRY;
    }
}

static uint16_t lm_step(apollo_overlay_state *s, lanec_io *io, uint16_t z)
{
    switch (z) {
    case L(VERB69): CALL(L(VERB69), z + 1u);                       /* TC to itself */
    /* V56: flags down, INTSTALL (interpretive), kill group 2, stop rates */
    case L(TRMTRAK1): case L(TRMTRAK1) + 2u: case L(TRMTRAK1) + 4u: CALL(L(DOWNFLAG), z + 1u);
    case L(TRMTRAK1) + 6u: case L(ATTACHIT): CALL(L(INTPRET), z + 1u);
    case L(TRMTRAK1) + 10u: CALL(L(PHASCHNG), z + 1u);
    case L(TRMTRAK1) + 12u: io->interrupt_enabled = 0; CALL(L(IBNKCALL), z + 2u);
    case L(TRMTRAK1) + 15u: CALL(L(IBNKCALL), z + 1u);
    case L(TRMTRAK1) + 17u: CALL(L(CLRADMOD), z + 1u);
    case L(TRMTRAK1) + 18u:
        lanec_ev_cs(s, LW(BIT14)); lanec_ev_wand(s, io, 012u);
        CALL(L(POSTJUMP), z + 4u);
    /* ATTACHIT, after its EXIT: our state vector into the other's, then
     * TACHEXIT's update and the return through PINBRNCH */
    case L(ATTACHIT) + 10u: lanec_ev_ca(s, LW(OCT51)); CALL(L(GENTRAN), z + 2u);
    case L(ATTACHIT) + 14u: io->interrupt_enabled = 1; CALL(L(INTPRET), z + 2u);
    case L(ATTACHIT) + 22u: return tcpin(s, LW(TCPINAD), L(FIXLOC), L(QPRET), L(POSTJUMP), z);
    default: return NOT_ENTRY;
    }
}

static uint16_t cm_step(apollo_overlay_state *s, lanec_io *io, uint16_t z)
{
    uint16_t *w = s->words;
    switch (z) {
    case C(VERB69): CALL(C(VERB69), z + 1u);
    /* V56: rendezvous and track flags down, stop rates, INTSTALL, restart */
    case C(TRACKTRM):
        lanec_ev_ca(s, CW(RNDVZBIT)); lanec_mask(s, w[C(FLAGWRD0)]);
        if (lanec_ev_zero(s)) JUMP(C(GOPIN));
        CALL(C(DOWNFLAG), z + 5u);
    case C(TRACKTRM) + 6u:
        lanec_ev_ca(s, CW(TRACKBIT)); lanec_mask(s, w[C(FLAGWRD1)]);
        if (lanec_ev_zero(s)) JUMP(C(GOPIN));
        CALL(C(DOWNFLAG), z + 5u);
    case C(TRACKTRM) + 12u: case C(TRACKTRM) + 14u: CALL(C(DOWNFLAG), z + 1u);
    case C(TRACKTRM) + 16u:
        lanec_ev_ca(s, CW(EBANK6)); lanec_ev_set_eb(s, w[LANEC_A]);
        io->interrupt_enabled = 0;
        CALL(C(STOPRATE), z + 4u);
    case C(TRACKTRM) + 20u:
        lanec_ev_ca(s, CW(NEGONE)); (void)lanec_ts(s, E(C(OPTIND)));
        CALL(C(INTPRET), z + 3u);
    case C(TRACKTRM) + 26u: CALL(C(2PHSCHNG), z + 1u);
    case C(CLEANOUT): io->interrupt_enabled = 0; CALL(C(POSTJUMP), z + 2u);
    case C(SETVHFLG): case C(RESETVHF): case C(ATTACHIT): case C(LMTOCM): CALL(C(INTPRET), z + 1u);
    case C(SETVHFLG) + 3u: case C(RESETVHF) + 4u: CALL(C(GOPIN), z + 1u);
    case C(RESETVHF) + 3u: CALL(C(TRFAILOF), z + 1u);
    case C(ATTACHIT) + 10u: case C(LMTOCM) + 10u: lanec_ev_ca(s, CW(OCT51)); CALL(C(GENTRAN), z + 2u);
    case C(LMTOCM) + 14u: JUMP(C(TACHEXIT));
    case C(TACHEXIT): io->interrupt_enabled = 1; CALL(C(INTPRET), z + 2u);
    case C(TACHEXIT) + 10u: return tcpin(s, CW(TCPINAD), C(FIXLOC), C(QPRET), C(POSTJUMP), z);
    case C(LMTOCMSV): lanec_ev_ca(s, CW(PRIO10)); CALL(C(FINDVAC), z + 2u);
    case C(LMTOCMSV) + 4u: CALL(C(ENDOFJOB), z + 1u);
    default: return NOT_ENTRY;
    }
}

const uint16_t *lanec_ev_track_entries(enum lanec_mission m, uint16_t bank, uint16_t *count)
{
    if (m == LANEC_LM && bank == 040u) { *count = COUNT(lm40); return lm40; }
    if (m == LANEC_LM && bank == 042u) { *count = COUNT(lm42); return lm42; }
    if (m == LANEC_LM && bank == 043u) { *count = COUNT(lm43); return lm43; }
    if (m == LANEC_CM && bank == 043u) { *count = COUNT(cm43); return cm43; }
    *count = 0;
    return lm40;
}

static uint8_t entry(enum lanec_mission m, uint16_t bank, uint16_t z)
{
    uint16_t i, n;
    const uint16_t *e = lanec_ev_track_entries(m, bank, &n);
    for (i = 0; i < n; ++i)
        if (e[i] == z) return 1;
    return 0;
}

enum lanec_status lanec_ev_track_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, z = w[LANEC_Z], bank, verb69 = m == LANEC_LM ? L(VERB69) : C(VERB69);
    if (!lanec_registers_valid(s)) return LANEC_OUT_OF_SCOPE;
    bank = lanec_current_bank(s, io);
    if (!entry(m, bank, z)) return LANEC_OUT_OF_SCOPE;
    /* TCPIN's indexed TS stays a TS of erasable memory only for FIXLOC +
     * QPRET below 02000 */
    if (bank == 043u && z == (m == LANEC_LM ? L(ATTACHIT) + 22u : C(TACHEXIT) + 10u)
        && (uint16_t)(w[m == LANEC_LM ? L(FIXLOC) : C(FIXLOC)] + (m == LANEC_LM ? L(QPRET) : C(QPRET))) >= 02000u)
        return LANEC_OUT_OF_SCOPE;
    do {
        z = m == LANEC_CM ? cm_step(s, io, z) : bank == 040u ? lm40_step(s, z) : lm_step(s, io, z);
        w[LANEC_Z] = z;
    } while (z != verb69 && entry(m, bank, z));
    return LANEC_OK;
}
