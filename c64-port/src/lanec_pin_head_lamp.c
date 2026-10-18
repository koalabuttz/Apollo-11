/* SPDX-License-Identifier: MIT */
/* Pinball lamp test (bank 41), Luminary099 and Comanche055: VBTSTLTS to
 * TC WAITLIST (TSTLTS1's loop leaving at each TC TSTLTS1), TSTLTS2 to
 * TC NOVAC and TSTLTS3 to TC BANKCALL (DSPMM). Comanche's VBTSTLTS first
 * calls CHKPOOH through BANKCALL and goes on at VBTSTLTS +2. Each entry runs
 * to the next call and leaves as the original does there, so WAITLIST, NOVAC,
 * BANKCALL and the loop run as their own routines. See
 * docs/LANEC-PIN-HEADS.md. */
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"
#include "lanec_ev_internal.h"

typedef char same_words[(LANEC_LM_VBTSTLTS == LANEC_CM_VBTSTLTS && LANEC_LM_TSTLTS2 == LANEC_CM_TSTLTS2 &&
    LANEC_LM_TSTLTS3 == LANEC_CM_TSTLTS3 && LANEC_LM_DSPTAB == LANEC_CM_DSPTAB && LANEC_LM_ERCNT == LANEC_CM_ERCNT &&
    LANEC_LM_NOUT == LANEC_CM_NOUT && LANEC_LM_TSTLTS1_BANK == 041u && LANEC_CM_TSTLTS1_BANK == 041u) ? 1 : -1];

#define DSALMOUT 011u
#define CHAN12 012u
#define CHAN13 013u
#define BIT1 1u
#define BIT4 010u
#define BIT10 01000u
#define BIT15 040000u
#define TEN 012u
#define ELEVEN 013u
#define FULLDSP 05675u
#define FULLDSP1 07675u
#define TSTCON1 00175u
#define TSTCON3 00115u
#define SHOLTS 00764u
#define CHRPRIO 030000u
#define PRIO15 015000u
#define PRIO16 016000u
#define DSPTAB LANEC_LM_DSPTAB
#define ERCNT LANEC_LM_ERCNT
#define NOUT LANEC_LM_NOUT

static void ts(apollo_overlay_state *s, uint16_t k) { (void)lanec_ts(s, k); }  /* no TS here overflows */

/* TSTLTS1 with A (the count) 0..10: TS ERCNT, CS FULLDSP, INDEX ERCNT,
 * TS DSPTAB, CCS ERCNT, TC TSTLTS1; at zero on through TC WAITLIST. */
static void tstlts1(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, uint16_t z)
{
    uint16_t *w = s->words;
    ts(s, ERCNT);
    lanec_ev_cs(s, FULLDSP);
    ts(s, (uint16_t)(DSPTAB + w[ERCNT]));
    if (lanec_ccs(s, w[ERCNT]) == 0) {
        lanec_tc(s, z, (uint16_t)(z + 6u));
        return;
    }
    lanec_ev_cs(s, FULLDSP1);
    ts(s, DSPTAB + 1u); ts(s, DSPTAB + 4u); ts(s, DSPTAB + 6u);
    lanec_ev_ca(s, ELEVEN);
    ts(s, NOUT);
    if (m == LANEC_LM) io->interrupt_enabled = 1;   /* RELINT, CAF SHOLTS, INHINT */
    lanec_ev_ca(s, SHOLTS);
    if (m == LANEC_LM) io->interrupt_enabled = 0;
    lanec_tc(s, m == LANEC_LM ? LANEC_LM_WAITLIST : LANEC_CM_WAITLIST, (uint16_t)(z + (m == LANEC_LM ? 020u : 016u)));
}

enum lanec_status lanec_pin_head_lamp_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, z = w[LANEC_Z];
    uint16_t loop = m == LANEC_LM ? LANEC_LM_TSTLTS1 : LANEC_CM_TSTLTS1;
    uint16_t imodes33 = m == LANEC_LM ? LANEC_LM_IMODES33 : LANEC_CM_IMODES33;
    uint16_t imodes30 = m == LANEC_LM ? LANEC_LM_IMODES30 : LANEC_CM_IMODES30;
    LANEC_ONE_MISSION(m);
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 041u) return LANEC_OUT_OF_SCOPE;
    if (z == LANEC_LM_VBTSTLTS && m == LANEC_CM) {   /* TC BANKCALL, CADR CHKPOOH */
        lanec_tc(s, LANEC_CM_BANKCALL, (uint16_t)(z + 1u));
        return LANEC_OK;
    }
    if (z == (m == LANEC_LM ? LANEC_LM_VBTSTLTS : LANEC_CM_VBTSTLTS + 2u)) {
        io->interrupt_enabled = 0;                          /* INHINT */
        lanec_ev_cs(s, BIT1); lanec_mask(s, w[imodes33]); lanec_ad(s, BIT1); ts(s, imodes33);
        lanec_ev_ca(s, TSTCON1); lanec_ev_wor(s, io, DSALMOUT);
        lanec_ev_ca(s, m == LANEC_LM ? 040674u : 040650u); ts(s, DSPTAB + 11u);   /* CAF TSTCON2 */
        lanec_ev_ca(s, BIT10); lanec_ev_wor(s, io, CHAN13);
        lanec_ev_ca(s, TEN);
        tstlts1(s, io, m, loop);
        return LANEC_OK;
    }
    if (z == loop) {
        if (w[LANEC_A] > TEN) return LANEC_OUT_OF_SCOPE;
        tstlts1(s, io, m, loop);
        return LANEC_OK;
    }
    if (z == LANEC_LM_TSTLTS2) {   /* CAF CHRPRIO, TC NOVAC */
        lanec_ev_ca(s, CHRPRIO);
        lanec_tc(s, m == LANEC_LM ? LANEC_LM_NOVAC : LANEC_CM_NOVAC, (uint16_t)(z + 2u));
        return LANEC_OK;
    }
    if (z == LANEC_LM_TSTLTS3) {
        lanec_ev_cs(s, TSTCON3);
        io->interrupt_enabled = 0;                          /* INHINT */
        lanec_ev_wand(s, io, DSALMOUT);
        lanec_ev_cs(s, BIT10); lanec_ev_wand(s, io, CHAN13);
        lanec_ev_ca(s, BIT4); lanec_ev_rand(s, io, CHAN12); lanec_ad(s, BIT15); ts(s, DSPTAB + 11u);
        lanec_ev_cs(s, 016001u); lanec_mask(s, w[imodes33]); lanec_ad(s, PRIO16); ts(s, imodes33);   /* 13-11,1 */
        lanec_ev_cs(s, 055000u); lanec_mask(s, w[imodes30]); lanec_ad(s, PRIO15); ts(s, imodes30);
        if (m == LANEC_LM) {   /* CS RFAILS2, MASK RADMODES, AD RCDUFBIT, TS RADMODES */
            lanec_ev_cs(s, 0330u); lanec_mask(s, w[LANEC_LM_RADMODES]); lanec_ad(s, 0100u); ts(s, LANEC_LM_RADMODES);
        } else {               /* CS OPTMODES, MASK BIT7, ADS OPTMODES */
            lanec_ev_cs(s, w[LANEC_CM_OPTMODES]); lanec_mask(s, 0100u); lanec_ev_ads(s, LANEC_CM_OPTMODES);
        }
        io->interrupt_enabled = 1;                          /* RELINT */
        lanec_tc(s, m == LANEC_LM ? LANEC_LM_BANKCALL : LANEC_CM_BANKCALL, (uint16_t)(z + (m == LANEC_LM ? 032u : 031u)));
        return LANEC_OK;
    }
    return LANEC_OUT_OF_SCOPE;
}
