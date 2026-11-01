/* SPDX-License-Identifier: MIT */
/* KALCMANU_STEERING.agc, the LM's (Luminary099 pages 365-369): the steering
 * of an automatic attitude maneuver, new CDU angles for the DAP each
 * second, and its start and end. The faithful C is
 * c64-port/src/lm_kalcmanu_steering.c.
 *
 * Basic code in bank 22 round one stretch of interpretive code (NEWANGL's,
 * and MANUSTAT's EXIT), which is in the listing's notation, run by the
 * readable interpreter. The entries below are the basic code's: the
 * routines WAITLIST and FINDVAC start (UPDTCALL, NEWDELHI, MANUSTOP), the
 * words after each EXIT, and the words BANKCALL, WAITLIST, FINDVAC and
 * SPVAC return to. The binding is src/binding/kalcmanu_steering_routines.c. */
#ifndef READABLE_KALCMANU_STEERING_H
#define READABLE_KALCMANU_STEERING_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
extern const struct interpretive_code NEWANGL_CODE;
extern const struct interpretive_code MANUSTAT_CODE;

void newdelhi(void);
void newdelhi_after_isitauto(void); /* NEWDELHI +2 */
void newangl(void);
void newangl_after_exit(void);      /* NEWANGL +025 */
void tmanuchk_after_timechk(void);  /* TMANUCHK +1 */
void manustal_after_waitlist(void); /* MANUSTAL +4 */
void manustat_after_exit(void);     /* MANUSTAT +1 */
void manucall_after_waitlist(void); /* MANUCALL +4 */
void updtcall(void);
void updtcall_after_findvac(void); /* UPDTCALL +4 */
void manustop(void);
void goodmanu(void);
void goodmanu_after_spvac(void); /* GOODMANU +5 */
#endif

#endif
