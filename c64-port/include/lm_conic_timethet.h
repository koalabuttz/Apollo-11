/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_TIMETHET_H
#define LM_CONIC_TIMETHET_H
#include "p66_bias.h"
#include "fixed_data.h"
#include "agc_word.h"
/* Luminary099 CONIC_SUBROUTINES time-theta subroutine: TIMETHET and
 * COMMNOUT (bank 12 02732-02766), PARAM and GEOM (bank 4 03064-03164),
 * GETX with its iteration, series and 360-degree paths (12,2767-3211) and
 * NEWSTATE/LAMENTER (12,2653-2731), with DELTIME from the Kepler milestone.
 * From the original interpretive entry frontier (NEWOPS at TIMETHET after
 * one of its six callers' CALL, QPRET the caller's return) to the NEWOPS
 * frontier at C(RTNTT). 0 unsupported (image unchanged), 1 returned, 2 the
 * UNIT/SQRT abort frontier (alarm 1302), 3 the ABTCONIC frontier (POODOO
 * with alarm 607: COGAFLAG or INFINFLG set). */
uint8_t lm_conic_timethet(apollo_overlay_state *,p66_bias_io *);

/* The same routine as a chain of steps between original NEWOPS frontiers
 * (one overlay module each). A step's phase argument must be the value the
 * previous step returned for the same image; the image carries every
 * continuation (LOC, X1/X2, QPRET, RTNTT, RTNPRM, the push-down list). */
enum lm_conic_timethet_phase {
 LTT_ENTRY,      /* TIMETHET 12,2732 through CALL PARAM */
 LTT_PARAM,      /* PARAM 4,3064 through SSP CALL GEOM */
 LTT_GEOM,       /* GEOM 4,3130 through UNIT BOV COLINEAR */
 LTT_UNITNORM,   /* UNITNORM 4,3154 through SIGN RVQ */
 LTT_COLINEAR,   /* COLINEAR 4,3160 */
 LTT_HAVENORM,   /* HAVENORM 4,3162 (NORMSW set; PARAM clears it) */
 LTT_PARAM2,     /* 4,3075 (GEOM's return) through GOTO RTNPRM */
 LTT_AFTERPARAM, /* 12,2744 BOV CALL COGAOVFL GETX */
 LTT_COGAOVFL,   /* COGAOVFL 12,2764 SETGO */
 LTT_GETX,       /* GETX 12,2767 through SL2R BOV 360CHECK */
 LTT_WLOOP,      /* WLOOP 12,3013 through BDDV BOV INFINITY */
 LTT_POLYCOEF,   /* POLYCOEF 12,3031: EXIT, POLY, TC INTPRET, PUSH BON */
 LTT_XCOMMON,    /* XCOMMON 12,3066 through RVQ */
 LTT_RESETX2,    /* RESETX2 12,3120 */
 LTT_360CHECK,   /* 360CHECK 12,3122 */
 LTT_INVRSEQN,   /* INVRSEQN 12,3127 */
 LTT_1WLOOP,     /* 1/WLOOP 12,3151 through SLR* GOTO POLYCOEF */
 LTT_TRUE360X,   /* TRUE360X 12,3172 */
 LTT_INFINITY,   /* INFINITY 12,3205 through SET RVQ */
 LTT_COMMNOUT,   /* COMMNOUT 12,2747 through CLEAR CALL DELTIME */
 LTT_DELTIME,    /* DELTIME (src/lm_conic_kepler.c lck_deltime) */
 LTT_DELTIME2,   /* (lck_deltime2) */
 LTT_DELTAIL,    /* (lck_deltail, its RVQ back to 12,2756) */
 LTT_AFTERDT,    /* 12,2756 BON CALL RVSW RTNTT NEWSTATE */
 LTT_NEWSTATE,   /* NEWSTATE 12,2653 through LAMENTER's VSL8 RVQ */
 LTT_FINAL,      /* 12,2762 GOTO RTNTT */
 LTT_ABTCONIC,   /* ABTCONIC 12,3742: EXIT, TC POODOO */
 LTT_RETURNED,   /* caller NEWOPS frontier at C(RTNTT) */
 LTT_ABORTED,    /* UNIT/SQRT abort frontier (1302) */
 LTT_CONICABORT, /* ABTCONIC's POODOO frontier (607) */
 LTT_FAILED      /* outside the qualified primitive contracts */
};
uint8_t lm_conic_timethet_step(apollo_overlay_state *,p66_bias_io *,uint8_t phase);
/* The phases themselves (module entry points); each returns the next phase. */
uint8_t ltt_entry(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_param(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_geom(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_unitnorm(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_colinear(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_havenorm(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_param2(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_afterparam(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_cogaovfl(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_getx(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_wloop(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_polycoef(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_xcommon(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_resetx2(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_360check(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_invrseqn(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_1wloop(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_true360x(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_infinity(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_commnout(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_deltime(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_deltime2(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_deltail(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_afterdt(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_newstate(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_final(apollo_overlay_state *,p66_bias_io *);
uint8_t ltt_abtconic(apollo_overlay_state *,p66_bias_io *);
#endif
