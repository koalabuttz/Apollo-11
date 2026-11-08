/* SPDX-License-Identifier: MIT */
/* The CM's thrust vector control (TVC), the autopilot of an SPS burn:
 * TVCINITIALIZE.agc (Comanche055 pp. 937-944, bank 17), "TVC DAP
 * INITIALIZATION AND STARTUP CALL". The faithful C is
 * c64-port/src/cm_tvc_init.c.
 *
 *   TVCDAPON, MRCLEAN, TVCINIT1 to TVCINIT4 (tvcinitialize.c): the start
 *     of the TVC DAP, in T5 interrupts ("T5LOC=2CADR(TVCDAPON,EBANK=BZERO),
 *     T5=.6SECT5", "CALLED BY "DOTVCON" IN P40"). It zeroes the DAP's
 *     erasables, then "PERFORMS TVCDAP INITIALIZATION (GAINS, TIMING
 *     PARAMETERS, FILTER VARIABLES, ETC.)", starts the needles (NEEDLER),
 *     calls TVCEXEC by WAITLIST and the DAP itself (DAPINIT) by T5.
 *   PRESWTCH, SWICHOVR (tvcinitialize.c): the switch to the low bandwidth
 *     gains and filter of the docked CSM/LM, from verb 46 or a restart.
 *   LOADCOEF, S40.15 (tvcinitialize.c): the filter coefficients loaded
 *     into N10 on, and the gains 1/CONACC and VARK.
 *
 * All basic code. Each function runs from a word the program can come
 * back to (an entry, the word after a call out of the file) until control
 * leaves the file's code (RESUME, IBNKCALL, WAITLIST, POSTJUMP, TC Q,
 * TC RTRNLOC), or reaches a word where the faithful port's run stops: a
 * key of its own, run by the function named for it. */
#ifndef READABLE_TVC_H
#define READABLE_TVC_H

#include "../agc/agc.h"

#if COMANCHE055

/* TVCINITIALIZE.agc -- tvcinitialize.c */
void tvcdapon(void);                /* TVCDAPON */
void mrclean(void);                 /* MRCLEAN */
void tvcinit1(void);                /* TVCINIT1 */
void tvcinit1_after_massprop(void); /* TVCINIT1 +5 */
void tvcinit1_after_loadcoef(void); /* TVCINIT1 +025 */
void tvcinit2(void);                /* TVCINIT2 */
void tvcinit2_repfrac(void);        /* TVCINIT2 +021 */
void tvcinit4(void);                /* TVCINIT4 */
void tvcinit4_after_waitlist(void); /* TVCINIT4 +017 */
void tvcinit4_t5(void);             /* TVCINIT4 +021 */
void preswtch(void);                /* PRESWTCH */
void preswtch_after_swichovr(void); /* PRESWTCH +1 */
void swichovr_restart(void);        /* SWICHOVR +5 */
void swichovr_after_s40_15(void);   /* SWICHOVR +026 */
void swichovr_after_loadcoef(void); /* SWICHOVR +043 */
void rtrnloc_return(void);          /* TC RTRNLOC, run as TC <return> */
void loadcoef(void);                /* LOADCOEF */
void loadcoef_dca6(void);           /* LOADCOEF +014 */
void loadcoef_n10_6(void);          /* LOADCOEF +017 */
void loadcoef_n10_8(void);          /* LOADCOEF +023 */
void s40_15(void);                  /* S40.15 */
void s40_15_switchover(void);       /* S40.15 +7 */

#endif

#endif
