/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_TIMERAD_H
#define LM_CONIC_TIMERAD_H
#include "lm_conic_timethet.h"
/* Luminary099 CONIC_SUBROUTINES time-radius and apsides subroutines:
 * TIMERAD with TERMNVEC, CIRCULAR and BADR2 (bank 12 03547-03673) and
 * APSIDES with GETECC and INFINAPO (12,3674-3741). Both reuse the time-theta
 * milestone's PARAM/GEOM, and TIMERAD its GETX, COMMNOUT and DELTIME. From
 * the original interpretive entry frontier (NEWOPS at TIMERAD after the P32
 * CSI STCALL, or at APSIDES after PERIAPO's STCALL) to the caller's NEWOPS
 * frontier at C(RTNTR)/C(RTNAPSE). 0 unsupported (image unchanged), 1
 * returned, 2 the UNIT/SQRT abort frontier (alarm 1302), 3 the ABTCONIC
 * frontier (POODOO with alarm 607: CIRCULAR, COGAFLAG or INFINFLG). */
uint8_t lm_conic_timerad(apollo_overlay_state *,p66_bias_io *);

/* Steps between original NEWOPS frontiers. The shared phases keep their
 * lm_conic_timethet_phase numbers (LTT_PARAM through LTT_ABTCONIC, and the
 * end states LTT_RETURNED, LTT_ABORTED, LTT_CONICABORT, LTT_FAILED); the
 * phases of this file follow them. */
enum lm_conic_timerad_phase {
 LTR_TIMERAD=LTT_FAILED+1, /* TIMERAD 12,3547 through CALL PARAM */
 LTR_AFTERPARAM, /* 12,3561 (PARAM's return) through CLEAR APSESW */
 LTR_TERMNVEC,   /* TERMNVEC 12,3630 through CALL GETX */
 LTR_SOLN,       /* 12,3655 (GETX's return) CLRGO SOLNSW COMMNOUT */
 LTR_CIRCULAR,   /* CIRCULAR 12,3660 SETPD SETGO */
 LTR_BADR2,      /* BADR2 12,3664 through SETGO TERMNVEC */
 LTR_APSIDES,    /* APSIDES 12,3674 through CALL PARAM */
 LTR_GETECC,     /* 12,3706 (PARAM's return) BOV, GETECC through GOTO RTNAPSE */
 LTR_INFINAPO    /* INFINAPO 12,3737 DLOAD GOTO */
};
/* True for the end states. */
#define LM_CONIC_TIMERAD_DONE(phase) ((phase)>=LTT_RETURNED&&(phase)<=LTT_FAILED)
uint8_t lm_conic_timerad_step(apollo_overlay_state *,p66_bias_io *,uint8_t phase);
/* The next phase for this routine after a shared ltt_* step returned
 * `next`: PARAM's and GETX's returns follow the return address in LOC. */
uint8_t ltr_remap(const apollo_overlay_state *,uint8_t next);
uint8_t ltr_timerad(apollo_overlay_state *,p66_bias_io *);
uint8_t ltr_afterparam(apollo_overlay_state *,p66_bias_io *);
uint8_t ltr_termnvec(apollo_overlay_state *,p66_bias_io *);
uint8_t ltr_soln(apollo_overlay_state *,p66_bias_io *);
uint8_t ltr_circular(apollo_overlay_state *,p66_bias_io *);
uint8_t ltr_badr2(apollo_overlay_state *,p66_bias_io *);
uint8_t ltr_apsides(apollo_overlay_state *,p66_bias_io *);
uint8_t ltr_getecc(apollo_overlay_state *,p66_bias_io *);
uint8_t ltr_infinapo(apollo_overlay_state *,p66_bias_io *);
#endif
