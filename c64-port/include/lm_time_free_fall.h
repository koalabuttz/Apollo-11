/* SPDX-License-Identifier: MIT */
#ifndef LM_TIME_FREE_FALL_H
#define LM_TIME_FREE_FALL_H
#include "p66_bias.h"
#include "fixed_data.h"
#include "agc_word.h"
/* Luminary099 TIME_OF_FREE_FALL (bank 27 words 03360-03772): TFFCONIC,
 * TFFCONMU, TFFRP/RA, CALCTPER and CALCTFF, from the original interpretive
 * entry frontier (NEWOPS after the caller's CALL, LOC = the entry, FBANK 27)
 * to the caller's NEWOPS frontier selected by QPRET. 0 unsupported (image
 * unchanged), 1 returned, 2 original SQRT abort frontier (alarm 1302). */
uint8_t lm_time_free_fall(apollo_overlay_state *,p66_bias_io *);

/* The same routine as a chain of steps between original NEWOPS frontiers
 * (one overlay module each). A step's phase argument must be the value the
 * previous step returned for the same image; the image (LOC, X1/X2, MPAC,
 * the push-down list) carries every continuation. */
enum lm_time_free_fall_phase {
 LTF_ENTRY,     /* TFFCONIC 03360: STORE TFF/RTMU */
 LTF_CONMU,     /* TFFCONMU 03361 through STORE TFFRTALF */
 LTF_CONMU_END, /* DSQ SIGN 03421 through BZE, 1/ALPHA and RVQ */
 LTF_RPRA,      /* TFFRP/RA 03427 through BPL +3 */
 LTF_MAXRA,     /* MAXRA 03461 */
 LTF_RAPO,      /* STORE RAPO 03463; RVQ */
 LTF_CALCTPER,  /* CALCTPER 03465: SETGO TFFSW */
 LTF_CALCTFF,   /* CALCTFF 03470: CLEAR TFFSW */
 LTF_TERMS,     /* STORE RTERM 03472 through BOFF TFFSW */
 LTF_QTERM,     /* +2 03515 BMN SQRT through STORE TFFQ1 */
 LTF_XTERM,     /* BMN BDSU 03526 through DMP BDSU */
 LTF_SAVEDEN,   /* SAVEDEN 03535 through DAD BOV */
 LTF_FALLBACK,  /* DLOAD PDDL 03541 through BMN TFFEL1 and RVQ */
 LTF_INBOUND,   /* INBOUND 03550 through GOTO SAVEDEN */
 LTF_XTEST,     /* TFFXTEST 03561 through DLOAD BOV */
 LTF_SERIES,    /* DDV STADR 03573 through STORE TFFX */
 LTF_POLY,      /* RTB DMP 03611 through DCOMP */
 LTF_SCALE,     /* DMP DDV 03631 through SL* DAD */
 LTF_ENDTFF,    /* ENDTFF 03641 through RVQ */
 LTF_NEGTFF,    /* NEGTFF 03645 */
 LTF_MAXTFF1,   /* MAXTFF1 03650 */
 LTF_MAXTFF,    /* MAXTFF 03651 */
 LTF_TFFELL,    /* TFFELL 03653 */
 LTF_TFFEL1,    /* TFFEL1 03655 through STORE TFFX */
 LTF_ELLIPSE,   /* RTB DMP 03674 through GOTO ENDTFF */
 LTF_RETURNED,  /* caller NEWOPS frontier */
 LTF_ABORTED,   /* SQRT abort frontier */
 LTF_FAILED     /* outside the qualified primitive contracts */
};
/* R300K 2DEC 6464778 B-29 (27,3766). */
extern const agc_word lm_time_free_fall_r300k[2];
uint8_t lm_time_free_fall_step(apollo_overlay_state *,p66_bias_io *,uint8_t phase);
/* The phases themselves (module entry points); each returns the next phase. */
uint8_t ltf_entry(apollo_overlay_state *);
uint8_t ltf_conmu(apollo_overlay_state *);
uint8_t ltf_conmu_end(apollo_overlay_state *);
uint8_t ltf_rpra(apollo_overlay_state *);
uint8_t ltf_maxra(apollo_overlay_state *);
uint8_t ltf_rapo(apollo_overlay_state *);
uint8_t ltf_calctper(apollo_overlay_state *,p66_bias_io *);
uint8_t ltf_calctff(apollo_overlay_state *,p66_bias_io *);
uint8_t ltf_terms(apollo_overlay_state *,p66_bias_io *);
uint8_t ltf_qterm(apollo_overlay_state *);
uint8_t ltf_xterm(apollo_overlay_state *);
uint8_t ltf_saveden(apollo_overlay_state *);
uint8_t ltf_fallback(apollo_overlay_state *);
uint8_t ltf_inbound(apollo_overlay_state *);
uint8_t ltf_xtest(apollo_overlay_state *);
uint8_t ltf_series(apollo_overlay_state *);
uint8_t ltf_poly(apollo_overlay_state *);
uint8_t ltf_scale(apollo_overlay_state *);
uint8_t ltf_endtff(apollo_overlay_state *);
uint8_t ltf_negtff(apollo_overlay_state *);
uint8_t ltf_maxtff1(apollo_overlay_state *);
uint8_t ltf_maxtff(apollo_overlay_state *);
uint8_t ltf_tffell(apollo_overlay_state *);
uint8_t ltf_tffel1(apollo_overlay_state *,p66_bias_io *);
uint8_t ltf_ellipse(apollo_overlay_state *);
#endif
