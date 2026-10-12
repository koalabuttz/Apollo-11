/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_KEPLER_H
#define LM_CONIC_KEPLER_H
#include "p66_bias.h"
#include "fixed_data.h"
#include "agc_word.h"
/* Luminary099 CONIC_SUBROUTINES Kepler subroutine (bank 12 words
 * 02023-02652): KEPLERN with DELTIME and CHECKCTR, from the original
 * interpretive entry frontier (NEWOPS at KEPLERN after KEPPREP's GOTO,
 * X1 = -2 Earth or -10 Moon, KEPRTN = KEPPREP's QPRET) to the NEWOPS
 * frontier at C(KEPRTN). 0 unsupported (image unchanged), 1 returned,
 * 2 original UNIT or SQRT abort frontier (alarm 1302). */
uint8_t lm_conic_kepler(apollo_overlay_state *,p66_bias_io *);

/* The same routine as a chain of steps between original NEWOPS frontiers
 * (one overlay module each). A step's phase argument must be the value the
 * previous step returned for the same image; the image (LOC, X1/X2, MPAC,
 * QPRET, the push-down list) carries every continuation. */
enum lm_conic_kepler_phase {
 LCK_ENTRY,     /* KEPLERN 02023 through BPL 1REV / SQRT GOTO STOREMAX */
 LCK_1REV,      /* 1REV 02073 */
 LCK_STOREMAX,  /* STOREMAX 02077 through BOV BMN MODDONE */
 LCK_PERIODCH,  /* PERIODCH 02112 */
 LCK_MODDONE,   /* MODDONE 02124 through DSU BPL BADX */
 LCK_STORBNDS,  /* STORBNDS 02140 */
 LCK_STOREMIN,  /* STOREMIN 02152 */
 LCK_DXCOMP,    /* DXCOMP 02155 through STORE DELX */
 LCK_KEPLOOP,   /* KEPLOOP 02166 through STCALL XI DELTIME */
 LCK_DELTIME,   /* DELTIME 02421: EXIT, POLY S(XI), TC INTPRET, STODL */
 LCK_DELTIME2,  /* EXIT 02453: POLY C(XI), TC INTPRET */
 LCK_DELTAIL,   /* DMP SRR* 02503 through RVQ */
 LCK_KEPRET,    /* BOV BDSU 02177 (DELTIME's return) */
 LCK_NDXCHNGE,  /* NDXCHNGE 02240 */
 LCK_POSDELX,   /* POSDELX 02246 */
 LCK_PDXCHNGE,  /* PDXCHNGE 02274 */
 LCK_NEWDELX,   /* NEWDELX 02260 through STORE TC */
 LCK_BRNCHCTR,  /* BRNCHCTR 02267: RTB BHIZ CHECKCTR, GOTO KEPLOOP */
 LCK_BADX,      /* BADX 02302 */
 LCK_TIMEOVFL,  /* TIMEOVFL 02311 */
 LCK_NEGTOVFL,  /* NEGTOVFL 02330 */
 LCK_CMNTOVFL,  /* CMNTOVFL 02315 */
 LCK_KEPCONVG,  /* KEPCONVG 02333 through GOTO KEPRTN */
 LCK_RETURNED,  /* caller NEWOPS frontier at C(KEPRTN) */
 LCK_ABORTED,   /* UNIT/SQRT abort frontier */
 LCK_FAILED     /* outside the qualified primitive contracts */
};
uint8_t lm_conic_kepler_step(apollo_overlay_state *,p66_bias_io *,uint8_t phase);
/* The phases themselves (module entry points); each returns the next phase. */
uint8_t lck_entry(apollo_overlay_state *);
uint8_t lck_1rev(apollo_overlay_state *);
uint8_t lck_storemax(apollo_overlay_state *);
uint8_t lck_periodch(apollo_overlay_state *);
uint8_t lck_moddone(apollo_overlay_state *);
uint8_t lck_storbnds(apollo_overlay_state *);
uint8_t lck_storemin(apollo_overlay_state *);
uint8_t lck_dxcomp(apollo_overlay_state *);
uint8_t lck_keploop(apollo_overlay_state *);
uint8_t lck_deltime(apollo_overlay_state *,p66_bias_io *);
uint8_t lck_deltime2(apollo_overlay_state *,p66_bias_io *);
uint8_t lck_deltail(apollo_overlay_state *);
uint8_t lck_kepret(apollo_overlay_state *);
uint8_t lck_ndxchnge(apollo_overlay_state *);
uint8_t lck_posdelx(apollo_overlay_state *);
uint8_t lck_pdxchnge(apollo_overlay_state *);
uint8_t lck_newdelx(apollo_overlay_state *);
uint8_t lck_brnchctr(apollo_overlay_state *);
uint8_t lck_badx(apollo_overlay_state *);
uint8_t lck_timeovfl(apollo_overlay_state *);
uint8_t lck_negtovfl(apollo_overlay_state *);
uint8_t lck_cmntovfl(apollo_overlay_state *);
uint8_t lck_kepconvg(apollo_overlay_state *);
#endif
