/* SPDX-License-Identifier: MIT */
/* ATTITUDE_MANEUVER_ROUTINE.agc, the LM's (Luminary099 pages 342-363):
 * KALCMANU's start, which finds the single rotation (its axis COF and angle
 * AM) that takes the LM from its present attitude to the one commanded,
 * and the matrix routines it and its steering use (CDUTODCM, DCMTOCDU,
 * MXM3, TRANSPOS, DELCOMP). The faithful C is
 * c64-port/src/lm_attitude_maneuver.c.
 *
 * Interpretive code, in bank 22, with a little basic code among it. The
 * interpretive code is in the listing's notation, one table for each run of
 * it between basic code (ATTITUDE_MANEUVER_CODES), run by the readable
 * interpreter. The entries below are the basic code's: KALCMAN3, the words
 * after each EXIT, READCDUK (an RTB routine), and the ends where control
 * leaves for ALARM, BANKCALL, WAITLIST and ENDOFJOB and the words they
 * return to. The binding is src/binding/attitude_maneuver_routines.c. */
#ifndef READABLE_ATTITUDE_MANEUVER_H
#define READABLE_ATTITUDE_MANEUVER_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
extern const struct interpretive_code *const ATTITUDE_MANEUVER_CODES[];
extern const unsigned ATTITUDE_MANEUVER_CODE_COUNT;

void kalcman3(void);
void trnspspd_after_exit(void); /* TRNSPSPD +1 */
void readcduk(void);
void toobadf_after_exit(void);  /* TOOBADF +1 */
void toobadf_after_alarm(void); /* TOOBADF +3 */
void nogo_minus2(void);         /* NOGO -2: KALCMANU_STEERING comes in here */
void nogo(void);
void nogo_after_stoprate(void); /* NOGO +2 */
void nogo_after_waitlist(void); /* NOGO +7 */
void toobadi_after_exit(void);  /* TOOBADI +1 */
#endif

#endif
