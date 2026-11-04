/* SPDX-License-Identifier: MIT */
/* GROUND_TRACKING_DETERMINATION_PROGRAM.agc, the LM's (Luminary099 pp.
 * 654-657): P21, the ground track program. The faithful C is
 * c64-port/src/lm_ground_track.c (and the older c64-port/src/
 * lm_p21_routines.c, with the same keys).
 *
 * P21's basic code (bank 24) asks the astronaut, by DSKY, which vehicle
 * (option 2: verb 04 noun 06 and a flashing verb 50 noun 06, through
 * GOPERF4) and for what time (verb 06 noun 34, through GOFLASH); its
 * interpretive code integrates that vehicle's state vector to the time
 * and works out the latitude, longitude and altitude there; the basic
 * code after its EXIT flashes them (verb 06 noun 43), and a recycle asks
 * again ten minutes later.
 *
 * The interpretive code is in the listing's notation (P21_CODE,
 * P21_RECYCLE_CODE), run by the readable interpreter. The entries below
 * are the basic code's: PROG21, P21PROG1, the words each BANKCALL comes
 * back to (terminate, proceed, a value loaded) and the word after the
 * EXIT. The binding is src/binding/ground_track_routines.c. */
#ifndef READABLE_GROUND_TRACK_H
#define READABLE_GROUND_TRACK_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
extern const struct interpretive_code P21_CODE, P21_RECYCLE_CODE;

void prog21(void);           /* PROG21 */
void prog21_terminate(void); /* PROG21 +5: GOPERF4's terminate */
void prog21_proceed(void);   /* PROG21 +6: its proceed */
void prog21_loaded(void);    /* PROG21 +7: its "R2 LOADED THROUGH DSKY" */
void p21prog1(void);         /* P21PROG1 */
void p21prog1_terminate(void);
void p21prog1_proceed(void);
void p21prog1_loaded(void);
void p21dsp_after_exit(void); /* P21DSP +21: after the EXIT */
void p21dsp_terminate(void);  /* P21DSP +24 */
void p21dsp_proceed(void);    /* P21DSP +25 */
void p21dsp_recycle(void);    /* P21DSP +26 */
#endif

#endif
