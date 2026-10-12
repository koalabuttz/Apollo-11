/* SPDX-License-Identifier: MIT */
/* Pinball verb 07 (DSPDPDEC/ENDDPDEC) and channel-noun load (CHANLOAD),
 * Luminary099 and Comanche055. See docs/LANEC-PIN-GAPS.md. */
#include "lanec_pin_gaps.h"
#include "lanec_symbols.h"

typedef struct { uint16_t dspdpdec, dspalarm, tpagree, dsp2dec, chanload, loadlv; } places;
static const places lm = { LANEC_LM_DSPDPDEC, LANEC_LM_DSPALARM, LANEC_LM_TPAGREE, LANEC_LM_DSP2DEC,
                           LANEC_LM_CHANLOAD, LANEC_LM_LOADLV };
static const places cm = { LANEC_CM_DSPDPDEC, LANEC_CM_DSPALARM, LANEC_CM_TPAGREE, LANEC_CM_DSP2DEC,
                           LANEC_CM_CHANLOAD, LANEC_CM_LOADLV };
typedef char same_erasable[(LANEC_LM_MIXBR == LANEC_CM_MIXBR && LANEC_LM_NOUNADD == LANEC_CM_NOUNADD &&
    LANEC_LM_NOUNCADR == LANEC_CM_NOUNCADR && LANEC_LM_DSPCOUNT == LANEC_CM_DSPCOUNT &&
    LANEC_LM_ENTEXIT == LANEC_CM_ENTEXIT && LANEC_LM_MPAC == LANEC_CM_MPAC &&
    LANEC_LM_DSPDPDEC_BANK == 040u && LANEC_CM_DSPDPDEC_BANK == 040u &&
    LANEC_LM_CHANLOAD_BANK == 041u && LANEC_CM_CHANLOAD_BANK == 041u &&
    LANEC_LM_ENDDPDEC == LANEC_LM_DSPDPDEC + 14u && LANEC_CM_ENDDPDEC == LANEC_CM_DSPDPDEC + 14u) ? 1 : -1];
#define MIXBR LANEC_LM_MIXBR
#define NOUNADD LANEC_LM_NOUNADD
#define NOUNCADR LANEC_LM_NOUNCADR
#define DSPCOUNT LANEC_LM_DSPCOUNT
#define ENTEXIT LANEC_LM_ENTEXIT
#define MPAC LANEC_LM_MPAC
#define R1D1 016u
#define SEVEN 7u
#define LOW9 0777u

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
    if (z != x || (w[MIXBR] != 1u && w[MIXBR] != 2u) || k < 010u || k == 01377u || k > 01776u)
        return LANEC_OUT_OF_SCOPE;
    if (w[MIXBR] == 2u) {                                   /* mixed noun: TC +0 indexed to TC DSPALARM */
        lanec_tc(s, (uint16_t)(x + 3u), (uint16_t)(x + 2u));
        lanec_tc(s, p->dspalarm, (uint16_t)(x + 4u));
        return LANEC_OK;
    }
    lanec_tc(s, (uint16_t)(x + 2u), (uint16_t)(x + 2u));    /* normal noun: TC +0 indexed to TC +2 */
    lanec_tc(s, (uint16_t)(x + 4u), (uint16_t)(x + 3u));
    hi = w[lanec_erasable(s, k)];                           /* DCA */
    lo = w[lanec_erasable(s, (uint16_t)(k + 1u))];
    lanec_rewrite(s, (uint16_t)(k + 1u)); lanec_rewrite(s, k);
    old_hi = w[MPAC]; old_lo = w[MPAC + 1u];                /* DXCH MPAC */
    w[MPAC] = hi; w[MPAC + 1u] = lo;
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

enum lanec_status lanec_pin_gaps_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t z = s->words[LANEC_Z];
    if (z == p->chanload && lanec_current_bank(s, io) == 041u) return lanec_pin_chanload(s, io, m);
    return lanec_pin_dspdpdec(s, io, m);
}
