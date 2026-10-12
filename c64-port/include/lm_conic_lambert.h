/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_LAMBERT_H
#define LM_CONIC_LAMBERT_H
#include "lm_conic_timethet.h"
/* Luminary099 CONIC_SUBROUTINES Lambert subroutine: LAMBERT through TARGETV
 * (bank 12 03212-03546) with its slope iterator ITERATOR (12,2536-2643). It
 * reuses the time-theta milestone's GEOM, GETX and DELTIME, the Kepler
 * milestone's CHECKCTR (compiled inline at its RTB), and compiles LAMENTER
 * (12,2701-2731) for TARGETV. From the original interpretive entry frontier
 * (NEWOPS at LAMBERT after INITVEL's SETITCTR, 11,3765) to the caller's
 * NEWOPS frontier at C(RTNLAMB). 0 unsupported (image unchanged), 1
 * returned, 2 the UNIT/SQRT abort frontier (alarm 1302). */
uint8_t lm_conic_lambert(apollo_overlay_state *,p66_bias_io *);

/* Steps between original NEWOPS frontiers. The shared phases keep their
 * lm_conic_timethet_phase numbers (LTT_GEOM through LTT_AFTERDT, and the end
 * states LTT_RETURNED, LTT_ABORTED, LTT_FAILED); the phases of this file
 * follow them. */
enum lm_conic_lambert_phase {
 LLA_LAMBERT=LTT_FAILED+1, /* LAMBERT 12,3212 through CALL GEOM */
 LLA_AFTERGEOM,  /* 12,3235 (GEOM's return) through DSU BMN COGUPLIM */
 LLA_UPLIM,      /* UPLIM 12,3277 */
 LLA_MAXCOGA,    /* MAXCOGA 12,3302 through BOV LOLIM */
 LLA_MINCOGA,    /* MINCOGA 12,3320 through DLOAD COGA */
 LLA_NOGUESS,    /* NOGUESS 12,3467 through GOTO LAMBLOOP */
 LLA_LOLIM,      /* LOLIM 12,3503 DLOAD GOTO */
 LLA_LAMBLOOP,   /* LAMBLOOP 12,3330 through CALL GETX */
 LLA_AFTERGETX,  /* 12,3363 (GETX's return) through CALL DELTIME */
 LLA_AFTERDT,    /* 12,3373 (DELTIME's return) through CALL ITERATOR */
 LLA_AFTERITER,  /* 12,3407 (ITERATOR's return) through GOTO LAMBLOOP */
 LLA_NEGP,       /* NEGP 12,3417 DLOAD BPL */
 LLA_HIENERGY,   /* HIENERGY 12,3422 */
 LLA_COMMONLM,   /* COMMONLM 12,3426 through GOTO LAMBLOOP */
 LLA_BIGTIME,    /* BIGTIME 12,3437 */
 LLA_LOENERGY,   /* LOENERGY 12,3442 through GOTO COMMONLM */
 LLA_SUFFCHEK,   /* SUFFCHEK 12,3450 through BPL SETGO */
 LLA_360LAMB,    /* 360LAMB 12,3463 SETPD SETGO */
 LLA_INITV,      /* INITV 12,3506 through GOTO RTNLAMB */
 LLA_TARGETV,    /* TARGETV 12,3541, LAMENTER 12,2701, through GOTO RTNLAMB */
 LLA_ITERATOR,   /* ITERATOR 12,2536 through ABS SIGN */
 LLA_SGNCHECK,   /* SGNCHECK 12,2562 through STORE MAX */
 LLA_MINCHECK,   /* MINCHECK 12,2571 through GOTO DELOK */
 LLA_MODNGDEL,   /* MODNGDEL 12,2600 */
 LLA_FIRSTIME,   /* FIRSTIME 12,2606 */
 LLA_POSDEL,     /* POSDEL 12,2620 through STORE MIN */
 LLA_MAXCHECK,   /* MAXCHECK 12,2625 through BOV BMN */
 LLA_DELOK,      /* DELOK 12,2632 */
 LLA_NEWDEL,     /* NEWDEL 12,2634 STORE RVQ */
 LLA_MODPSDEL    /* MODPSDEL 12,2636 */
};
/* True for the end states. */
#define LM_CONIC_LAMBERT_DONE(phase) ((phase)>=LTT_RETURNED&&(phase)<=LTT_FAILED)
uint8_t lm_conic_lambert_step(apollo_overlay_state *,p66_bias_io *,uint8_t phase);
/* The next phase for this routine after a shared ltt_* step returned
 * `next`: GEOM's, GETX's and DELTIME's returns follow the return address
 * in LOC. */
uint8_t lla_remap(const apollo_overlay_state *,uint8_t next);
uint8_t lla_lambert(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_aftergeom(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_uplim(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_maxcoga(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_mincoga(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_noguess(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_lolim(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_lambloop(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_aftergetx(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_afterdt(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_afteriter(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_negp(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_hienergy(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_commonlm(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_bigtime(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_loenergy(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_suffchek(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_360lamb(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_initv(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_targetv(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_iterator(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_sgncheck(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_mincheck(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_modngdel(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_firstime(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_posdel(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_maxcheck(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_delok(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_newdel(apollo_overlay_state *,p66_bias_io *);
uint8_t lla_modpsdel(apollo_overlay_state *,p66_bias_io *);
#endif
