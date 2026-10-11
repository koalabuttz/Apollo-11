/* SPDX-License-Identifier: MIT */
#ifndef LM_EPHEMERIS_H
#define LM_EPHEMERIS_H
#include "p66_bias.h"
#include "fixed_data.h"
#include "agc_word.h"
/* Luminary LSPOS (=LUNPOS) and LUNVEL from the original interpretive entry
 * frontier (NEWOPS, LOC 03664 or 03775, bank 15) to the actual caller NEWOPS
 * frontier selected by QPRET. 0 unsupported (image unchanged), 1 returned,
 * 2 original UNIT SQRT abort frontier. */
uint8_t lm_ephemeris(apollo_overlay_state *,p66_bias_io *);

/* The same routine as a chain of steps between original NEWOPS frontiers, so
 * that each step can live in its own overlay module. lm_ephemeris admits the
 * entry image and then runs lm_ephemeris_step from LME_ENTRY until it
 * returns LME_RETURNED, LME_ABORTED or LME_FAILED. A step's phase argument
 * must be the value the previous step returned for the same image; the image
 * itself (LOC, X1/X2, MPAC, the push-down list) carries every continuation. */
enum lm_ephemeris_phase {
 LME_ENTRY,      /* LSPOS 03664 or LUNVEL 03775 */
 LME_POSITA,     /* POSITA 03700, through the POSITB loop */
 LME_POSITD,     /* POSITD 03722, through POSITE and BPL POSITA */
 LME_POSITF,     /* POSITF 03743, lunar angles */
 LME_MOON_VDEF,  /* COS VDEF 03752 */
 LME_MOON_UNIT,  /* MXV KONMAT UNIT; STORE VMOON (03753) */
 LME_SUN_ANGLES, /* DLOAD KONMAT +2 03756, solar angles */
 LME_SUN_VDEF,   /* COS VDEF 03763 */
 LME_SUN_UNIT,   /* MXV KONMAT UNIT; STORE VSUN; RVQ (03764) */
 LME_RETURNED,   /* caller NEWOPS frontier */
 LME_ABORTED,    /* UNIT SQRT abort frontier */
 LME_FAILED      /* outside the qualified primitive contracts */
};
uint8_t lm_ephemeris_step(apollo_overlay_state *,p66_bias_io *,uint8_t phase);
/* The phases themselves (module entry points); each returns the next phase. */
uint8_t lme_entry(apollo_overlay_state *,p66_bias_io *);
uint8_t lme_posita(apollo_overlay_state *,p66_bias_io *);
uint8_t lme_positd(apollo_overlay_state *);
uint8_t lme_positf(apollo_overlay_state *);
uint8_t lme_moon_vdef(apollo_overlay_state *);
uint8_t lme_moon_unit(apollo_overlay_state *,p66_bias_io *);
uint8_t lme_sun_angles(apollo_overlay_state *);
uint8_t lme_sun_vdef(apollo_overlay_state *);
uint8_t lme_sun_unit(apollo_overlay_state *,p66_bias_io *);
#endif
