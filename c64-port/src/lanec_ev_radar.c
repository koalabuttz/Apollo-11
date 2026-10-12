/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS radar test verbs, Luminary099 bank 43: V63 R04 (sample the
 * rendezvous or landing radar once a second and display it), V78 R77 (LR
 * spurious-return test) and V79 R77END. Each case is one basic block of the
 * source, entered at its address; see docs/LANEC-EV-RADAR.md. */
#include "lanec_ev_radar.h"
#include "lanec_ev_internal.h"
#include "lanec_symbols.h"

#define NOT_ENTRY 0xffffu
#define CALL(T, R) return lanec_ev_call(s, (T), (uint16_t)(R))
#define JUMP(T) return (uint16_t)(T)
#define E(X) lanec_erasable(s, (X))
#define L(X) LANEC_LM_##X
#define LW(X) LANEC_LM_##X##_WORD

static const uint16_t entries[] = {
    L(R77), L(R77) + 1u, L(R77) + 7u, L(R04), L(R04) + 1u, L(R04) + 2u, L(R04Z),
    L(R04A), L(R04A) + 2u, L(R04A) + 5u, L(R04A) + 6u, L(R04A) + 7u, L(R04A) + 8u, L(R04A) + 10u,
    L(R04A) + 11u, L(R04X) - 1u, L(R04X), L(R04B), L(R04B) + 5u, L(R04B) + 10u, L(R04B) + 11u, L(R04B) + 12u,
    L(R04C), L(R04C) + 4u, L(R04C) + 8u,
    L(R04RR), L(R04RR) + 3u, L(R04RR) + 4u, L(R04RR) + 5u, L(R04RR) + 6u, L(R04RR) + 9u, L(R04RR) + 10u,
    L(R04RR) + 11u, L(R04LR), L(R04LR) + 3u, L(R04LR) + 4u, L(R04LR) + 5u, L(R04LR) + 6u, L(R04LR) + 9u,
    L(R04LR) + 10u, L(R04LR) + 11u, L(R04Y), L(R04Y) + 5u, L(R04K), L(R04L),
    L(R04END), L(R04END) + 5u, L(R04END) + 11u, L(R77END), L(R77END) + 7u, L(R77END) + 9u
};

/* CA/CS flag word, MASK bit, CCS A: 1 when the masked value is positive */
static uint8_t positive(apollo_overlay_state *s, uint16_t value, uint16_t bit, uint8_t complement)
{
    if (complement) lanec_ev_cs(s, value); else lanec_ev_ca(s, value);
    lanec_mask(s, bit);
    return (uint8_t)(lanec_ccs_a(s) == 0);
}

/* CAF Vnnnn, TC BANKCALL (GOMARKF, GOMARKFR or GOMARK4 follows) */
static uint16_t display(apollo_overlay_state *s, uint16_t vn, uint16_t at)
{
    lanec_ev_ca(s, vn);
    CALL(L(BANKCALL), at + 2u);
}

static uint16_t step(apollo_overlay_state *s, lanec_io *io, uint16_t z)
{
    uint16_t *w = s->words;
    switch (z) {
    case L(R77): case L(R04): CALL(L(RDRUSECK), z + 1u);
    /* R77 +1: CA FLAGWRD3, MASK R04FLBIT, CCS A, TC ALM/END, TC UPFLAG */
    case L(R77) + 1u:
        if (positive(s, w[L(FLAGWRD3)], LW(R04FLBIT), 0)) CALL(L(ALM_S_END), z + 4u);
        CALL(L(UPFLAG), z + 5u);
    case L(R77) + 7u: JUMP(L(R04Z));
    case L(R04) + 1u: CALL(L(TESTXACT), z + 1u);
    case L(R04) + 2u: CALL(L(UPFLAG), z + 1u);
    /* R04Z: EBANK4, sample period, counters, LR position bits of RADMODES from
     * channel 33, then R77 (R04FLAG clear) to R04K or R04 on to R04A */
    case L(R04Z):
        lanec_ev_ca(s, LW(EBANK4)); lanec_ev_set_eb(s, w[LANEC_A]);
        lanec_ev_ca(s, LW(1SEC_P_1)); (void)lanec_ts(s, E(L(RSAMPDT)));
        w[LANEC_A] = 0; (void)lanec_ts(s, E(L(RTSTLOC))); (void)lanec_ts(s, E(L(RFAILCNT)));
        io->interrupt_enabled = 0;
        lanec_ev_cs(s, LW(LRPOSCAL)); lanec_mask(s, w[L(RADMODES)]); (void)lanec_ts(s, L(RADMODES));
        lanec_ev_ca(s, LW(LRPOSCAL)); lanec_ev_rand(s, io, 033u); lanec_ev_ads(s, L(RADMODES));
        io->interrupt_enabled = 1;
        if (positive(s, w[L(FLAGWRD3)], LW(R04FLBIT), 1)) JUMP(L(R04K));
        lanec_ev_ca(s, LW(ONE)); (void)lanec_ts(s, L(OPTIONX) + 1u);
        JUMP(L(R04A));
    case L(R04A): lanec_ev_ca(s, LW(BIT3)); (void)lanec_ts(s, L(OPTIONX)); JUMP(z + 2u);
    case L(R04A) + 2u: return display(s, LW(V04N12X), z);
    case L(R04A) + 5u: case L(R04B) + 10u: case L(R04RR) + 3u: case L(R04RR) + 9u: case L(R04LR) + 3u:
    case L(R04LR) + 9u:
        JUMP(L(R04END));
    case L(R04A) + 6u: JUMP(z + 5u);                                   /* TCF +5 */
    case L(R04A) + 7u: JUMP(L(R04A) + 2u);
    case L(R04A) + 8u: lanec_ev_ca(s, LW(BIT3)); CALL(L(BLANKET), z + 2u);
    case L(R04A) + 10u: CALL(L(ENDOFJOB), z + 1u);
    case L(R04A) + 11u: lanec_ev_ca(s, w[L(OPTIONX) + 1u]); JUMP(L(R04X) - 1u);
    case L(R04X) - 1u: (void)lanec_ts(s, E(L(RTSTDEX))); JUMP(L(R04X));
    /* R04X: CAF SIX, MASK RTSTDEX, CCS A, TCF R04L, TS RTSTBASE (RR: 0) */
    case L(R04X):
        if (positive(s, LW(SIX), w[E(L(RTSTDEX))], 0)) JUMP(L(R04L));
        (void)lanec_ts(s, E(L(RTSTBASE)));
        JUMP(L(R04B));
    /* R04B: CAF BIT2, EXTEND, RAND CHAN33, EXTEND, BZF R04C; +5 CAF 201R04,
     * TS DSPTEM1, CAF V50N25X, BANKCALL GOMARK4 */
    case L(R04B):
        lanec_ev_ca(s, LW(BIT2));
        lanec_ev_rand(s, io, 033u);
        if (lanec_ev_zero(s)) JUMP(L(R04C));
        JUMP(z + 5u);
    case L(R04B) + 5u:
        lanec_ev_ca(s, LW(201R04)); (void)lanec_ts(s, L(DSPTEM1));
        return display(s, LW(V50N25X), z + 2u);
    case L(R04B) + 11u: JUMP(L(R04B));
    case L(R04B) + 12u: JUMP(z - 7u);                                  /* TCF -7 */
    /* R04C: CAF BIT14, EXTEND, WOR CHAN12, CAF TWO; +4 TS RTSTMAX, TC WAITLIST */
    case L(R04C):
        lanec_ev_ca(s, LW(BIT14));
        lanec_ev_wor(s, io, 012u);
        lanec_ev_ca(s, LW(TWO));
        JUMP(z + 4u);
    case L(R04C) + 4u: (void)lanec_ts(s, E(L(RTSTMAX))); CALL(L(WAITLIST), z + 2u);
    /* R04C +8: RELINT, R04FLAG clear (R77) -> GOPIN, RTSTDEX picks LR or RR */
    case L(R04C) + 8u:
        io->interrupt_enabled = 1;
        if (positive(s, w[L(FLAGWRD3)], LW(R04FLBIT), 1)) JUMP(L(GOPIN));
        if (positive(s, LW(SIX), w[E(L(RTSTDEX))], 0)) JUMP(L(R04LR));
        JUMP(L(R04RR));
    case L(R04RR): return display(s, LW(V16N72), z);
    case L(R04RR) + 6u: return display(s, LW(V16N78), z);
    case L(R04LR): return display(s, LW(V16N66), z);
    case L(R04LR) + 6u: return display(s, LW(V16N67), z);
    case L(R04RR) + 4u: case L(R04LR) + 4u: JUMP(z + 2u);              /* TCF +2 */
    case L(R04RR) + 5u: case L(R04RR) + 11u: JUMP(L(R04RR));
    case L(R04LR) + 5u: case L(R04LR) + 11u: JUMP(L(R04LR));
    case L(R04RR) + 10u: case L(R04LR) + 10u: JUMP(L(R04Y));
    /* R04Y: stop sampling, wait 2 s; +5 restart sampling and step RTSTDEX */
    case L(R04Y):
        w[LANEC_A] = 0; (void)lanec_ts(s, E(L(RSAMPDT)));
        lanec_ev_ca(s, LW(2SECS));
        CALL(L(BANKCALL), z + 4u);
    case L(R04Y) + 5u:
        lanec_ev_ca(s, LW(1SEC_P_1)); (void)lanec_ts(s, E(L(RSAMPDT)));
        w[LANEC_A] = 0; (void)lanec_ts(s, E(L(RTSTLOC)));
        if (positive(s, LW(SIX), w[E(L(RTSTDEX))], 0)) w[LANEC_A] = (uint16_t)~lanec_a16(LW(ONE));  /* CS ONE */
        lanec_ad(s, LW(TWO));
        JUMP(L(R04X) - 1u);
    case L(R04K): lanec_ev_ca(s, LW(250MS_P_1)); (void)lanec_ts(s, E(L(RSAMPDT))); JUMP(L(R04L));
    case L(R04L):
        lanec_ev_ca(s, LW(TWO)); (void)lanec_ts(s, E(L(RTSTBASE)));
        lanec_ev_ca(s, LW(SIX));
        JUMP(L(R04C) + 4u);
    /* R04END: stop sampling, wait, disable RR auto tracker enable, clear R04FLAG */
    case L(R04END):
        w[LANEC_A] = 0; (void)lanec_ts(s, E(L(RSAMPDT)));
        lanec_ev_ca(s, LW(BIT8));
        CALL(L(BANKCALL), z + 4u);
    case L(R04END) + 5u:
        io->interrupt_enabled = 0;
        lanec_ev_cs(s, LW(BIT14));
        lanec_ev_wand(s, io, 012u);
        CALL(L(DOWNFLAG), z + 5u);
    case L(R04END) + 11u: CALL(L(ENDEXT), z + 1u);
    case L(R77END):
        lanec_ev_ca(s, LW(EBANK4)); lanec_ev_set_eb(s, w[LANEC_A]);
        w[LANEC_A] = 0; (void)lanec_ts(s, E(L(RSAMPDT)));
        lanec_ev_ca(s, LW(BIT6));
        CALL(L(BANKCALL), z + 6u);
    case L(R77END) + 7u: CALL(L(DOWNFLAG), z + 1u);
    case L(R77END) + 9u: JUMP(L(GOPIN));
    default: return NOT_ENTRY;
    }
}

#define COUNT(X) ((uint16_t)(sizeof(X) / sizeof((X)[0])))

const uint16_t *lanec_ev_radar_entries(uint16_t *count)
{
    *count = COUNT(entries);
    return entries;
}

static uint8_t entry(uint16_t z)
{
    uint16_t i;
    for (i = 0; i < COUNT(entries); ++i)
        if (entries[i] == z) return 1;
    return 0;
}

enum lanec_status lanec_ev_radar_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, z = w[LANEC_Z];
    if (m != LANEC_LM || !lanec_registers_valid(s) || lanec_current_bank(s, io) != 043u || !entry(z))
        return LANEC_OUT_OF_SCOPE;
    /* R04X -1 and R04C +4 start with a TS of the A their callers leave: never overflowed. */
    if ((z == L(R04X) - 1u || z == L(R04C) + 4u) && ((w[LANEC_A] >> 14) == 1u || (w[LANEC_A] >> 14) == 2u))
        return LANEC_OUT_OF_SCOPE;
    do {
        z = step(s, io, z);
        w[LANEC_Z] = z;
    } while (entry(z));
    return LANEC_OK;
}
