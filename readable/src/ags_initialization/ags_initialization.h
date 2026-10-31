/* SPDX-License-Identifier: MIT */
/* AGS_INITIALIZATION.agc, the LM's only: R47, AGS initialization
 * (Luminary099 pages 206-210). The faithful C is c64-port/src/lm_ags_init.c.
 *
 * Verb 47 starts it. It shows the AGS clock's zero (AGSK) and the crew
 * either set a new zero at their ENTER, key one in (V25), or proceed: then
 * the LM's and CM's state vectors, brought to the present and turned into
 * the AGS's units and IMU axes, go down the AGS downlist for 20 seconds
 * (SCALEVEC), and the IMU's CDUs are zeroed unless the IMU is in use.
 *
 * The basic code's entries are below: the routine's start and each word a
 * call out of the file returns to. Its interpretive code is the tables
 * AGSVCALC_CODE, AGSVCALC_CODE_2 and SCALEVEC_CODE, in the listing's
 * notation, which the readable interpreter runs. */
#ifndef READABLE_AGS_INITIALIZATION_H
#define READABLE_AGS_INITIALIZATION_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
void agsinit(void);
void agsinit_after_alarm(void);        /* AGSINIT +6 */
void agsdispk_after_terminate(void);   /* AGSDISPK +3, GOMARKF's returns */
void agsdispk_after_proceed(void);     /* AGSDISPK +4 */
void agsdispk_after_enter(void);       /* AGSDISPK +5 */
void agsvcalc_after_exit(void);        /* AGSVCALC +5, after SET EXIT */
void agsvcalc_after_exdspret(void);    /* AGSVCALC +010 */
void agsvcalc_after_second_exit(void); /* AGSVCALC +034, after EXIT */
void agsvcalc_after_delayjob(void);    /* AGSVCALC +041 */
void ckstall_after_delayjob(void);     /* CKSTALL +7 */
void ckstall_after_imuzero(void);      /* CKSTALL +012 */
void ckstall_after_imustall(void);     /* CKSTALL +014 */
void agsend(void);
void agsend_after_downflag(void);  /* AGSEND +2 */
void agsend_after_terminate(void); /* AGSEND +5, GOMARK3's returns */
void agsend_after_proceed(void);   /* AGSEND +6 */
void agsend_after_enter(void);     /* AGSEND +7 */

/* The interpretive code: AGSVCALC's two runs (around its basic display
 * call) and SCALEVEC. */
extern const struct interpretive_code AGSVCALC_CODE, AGSVCALC_CODE_2, SCALEVEC_CODE;
#endif

#endif
