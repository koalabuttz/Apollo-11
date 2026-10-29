/* SPDX-License-Identifier: MIT */
#ifndef LM_PERIAPO_H
#define LM_PERIAPO_H
#include "lm_conic_timerad.h"
/* Luminary099 apsides display subroutines of P34-35_P74-75 (bank 23
 * 02316-02431): PERIAPO1, PERIAPO and SETRAD/SETRADX, LEMSTORE/ACTIVE,
 * CSMSTORE/PASSIVE, VECSHIFT and SHIFTR1, and R30's MAXCHK (22,3636-3644).
 * PERIAPO's STCALL XXXALT APSIDES runs the lm-conic-timerad milestone's
 * APSIDES steps. From the original interpretive entry frontier (NEWOPS at
 * the routine after one of its Luminary callers' CALL, QPRET that caller's
 * return) to the caller's NEWOPS frontier at C(QPRET). 0 unsupported (image
 * unchanged), 1 returned, 2 the UNIT/SQRT abort frontier (alarm 1302), 3 the
 * ABTCONIC frontier (POODOO with alarm 607). */
uint8_t lm_periapo(apollo_overlay_state *,p66_bias_io *);

/* Steps between original NEWOPS frontiers. APSIDES and its shared conic
 * phases keep their lm_conic_timerad/lm_conic_timethet numbers and end
 * states (LTT_RETURNED, LTT_ABORTED, LTT_CONICABORT, LTT_FAILED). */
enum lm_periapo_phase {
 LPA_PERIAPO1=LTR_INFINAPO+1, /* PERIAPO1 23,2316 through PERIAPO's CALL SETRAD */
 LPA_PERIAPO,     /* PERIAPO 23,2326 STQ CALL NORMEX SETRAD */
 LPA_SETRAD,      /* SETRAD 23,2344 through SETRADX's DLOAD RVQ */
 LPA_AFTERSETRAD, /* 23,2331 (SETRAD's return) STCALL XXXALT APSIDES */
 LPA_AFTERAPSIDES,/* 23,2333 (APSIDES' return) through PUSH GOTO NORMEX */
 LPA_LEMSTORE,    /* LEMSTORE 23,2376 VLOAD BOFF, ACTIVE through RVQ */
 LPA_CSMSTORE,    /* CSMSTORE 23,2406 VLOAD BOFF, PASSIVE through RVQ */
 LPA_VECSHIFT,    /* VECSHIFT 23,2416 through RVQ */
 LPA_SHIFTR1,     /* SHIFTR1 23,2426 LXA,2 SL* RVQ */
 LPA_MAXCHK       /* MAXCHK 22,3636 through either RVQ */
};
/* The first phase of an admitted entry, or 0 outside the contract. */
uint8_t lm_periapo_entry(const apollo_overlay_state *,const p66_bias_io *);
/* True for the end states. */
#define LM_PERIAPO_DONE(phase) ((phase)>=LTT_RETURNED&&(phase)<=LTT_FAILED)
uint8_t lm_periapo_step(apollo_overlay_state *,p66_bias_io *,uint8_t phase);
/* The next phase after an APSIDES step returned `next`: APSIDES' return to
 * 23,2333 continues PERIAPO. */
uint8_t lpa_remap(const apollo_overlay_state *,uint8_t next);
uint8_t lpa_periapo1(apollo_overlay_state *,p66_bias_io *);
uint8_t lpa_periapo(apollo_overlay_state *,p66_bias_io *);
uint8_t lpa_setrad(apollo_overlay_state *,p66_bias_io *);
uint8_t lpa_aftersetrad(apollo_overlay_state *,p66_bias_io *);
uint8_t lpa_afterapsides(apollo_overlay_state *,p66_bias_io *);
uint8_t lpa_lemstore(apollo_overlay_state *,p66_bias_io *);
uint8_t lpa_csmstore(apollo_overlay_state *,p66_bias_io *);
uint8_t lpa_vecshift(apollo_overlay_state *,p66_bias_io *);
uint8_t lpa_shiftr1(apollo_overlay_state *,p66_bias_io *);
uint8_t lpa_maxchk(apollo_overlay_state *,p66_bias_io *);
#endif
