/* SPDX-License-Identifier: MIT */
#ifndef LM_INITVEL_H
#define LM_INITVEL_H
#include "lm_conic_lambert.h"
/* Luminary099 INITVEL (P34-35_P74-75, bank 11 02000-02274): the required
 * initial velocity for a transfer of given time between RINIT and RTARG,
 * with SETITCTR (ORBITAL_INTEGRATION 11,3765-3775) compiled inline at its
 * CALL. LAMBERT is the lm-conic-lambert milestone's steps. From the
 * original interpretive entry frontier (NEWOPS at INITVEL or HAVEGUES after
 * one of its Luminary callers' CALL, or at 11,2155 and 11,2200 where
 * INTSTALL and INTEGRVS return) to the caller's NEWOPS frontier at
 * C(NORMEX). 0 unsupported (image unchanged), 1 returned, 2 the UNIT/SQRT
 * abort frontier (alarm 1302), 3 the precision-integration frontier (NEWOPS
 * at INTSTALL 13,3414 or INTEGRVS 13,3107, which this milestone does not
 * port). */
uint8_t lm_initvel(apollo_overlay_state *,p66_bias_io *);

/* Steps between original NEWOPS frontiers. LAMBERT and its shared conic
 * phases keep their lm_conic_lambert and lm_conic_timethet numbers and end
 * states (LTT_RETURNED, LTT_ABORTED, LTT_FAILED). */
enum lm_initvel_phase {
 LIV_INITVEL=LLA_MODPSDEL+1, /* INITVEL 11,2000 SET, then HAVEGUES */
 LIV_HAVEGUES,      /* HAVEGUES 11,2002 through STORE RTMAG */
 LIV_INITVEL1,      /* INITVEL1 11,2026 through STORE COZY4 */
 LIV_INITVEL2,      /* INITVEL2 11,2062 through STORE RTARG1 */
 LIV_INITVEL3,      /* INITVEL3 11,2105 through CALL SETITCTR and its GOTO LAMBERT */
 LIV_AFTERLAMBERT,  /* 11,2145 (LAMBERT's return) through CALL INTSTALL */
 LIV_AFTERINTSTALL, /* 11,2155 (INTSTALL's return) through STCALL INTEGRVS */
 LIV_AFTERINTEGRVS, /* 11,2200 (INTEGRVS' return) through GOTO INITVEL2 */
 LIV_INITVEL6,      /* INITVEL6 11,2225 VLOAD STORE */
 LIV_INITVEL7,      /* INITVEL7 11,2230 through STCALL RTARG NORMEX */
 LIV_EXTERNAL       /* end state: NEWOPS at INTSTALL or INTEGRVS */
};
/* The first phase of an admitted entry, or 0 outside the contract. */
uint8_t lm_initvel_entry(const apollo_overlay_state *,const p66_bias_io *);
/* True for the end states. */
#define LM_INITVEL_DONE(phase) (((phase)>=LTT_RETURNED&&(phase)<=LTT_FAILED)||(phase)==LIV_EXTERNAL)
uint8_t lm_initvel_step(apollo_overlay_state *,p66_bias_io *,uint8_t phase);
/* The next phase after a LAMBERT step returned `next`: LAMBERT's return to
 * 11,2145 continues INITVEL. */
uint8_t liv_remap(const apollo_overlay_state *,uint8_t next);
uint8_t liv_initvel(apollo_overlay_state *,p66_bias_io *);
uint8_t liv_havegues(apollo_overlay_state *,p66_bias_io *);
uint8_t liv_initvel1(apollo_overlay_state *,p66_bias_io *);
uint8_t liv_initvel2(apollo_overlay_state *,p66_bias_io *);
uint8_t liv_initvel3(apollo_overlay_state *,p66_bias_io *);
uint8_t liv_afterlambert(apollo_overlay_state *,p66_bias_io *);
uint8_t liv_afterintstall(apollo_overlay_state *,p66_bias_io *);
uint8_t liv_afterintegrvs(apollo_overlay_state *,p66_bias_io *);
uint8_t liv_initvel6(apollo_overlay_state *,p66_bias_io *);
uint8_t liv_initvel7(apollo_overlay_state *,p66_bias_io *);
#endif
