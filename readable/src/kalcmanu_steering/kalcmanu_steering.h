/* SPDX-License-Identifier: MIT */
/* KALCMANU_STEERING.agc (Luminary099 pages 365-369, Comanche055 pages
 * 414-419): the steering of an automatic attitude maneuver, new CDU angles
 * for the DAP each second, and its start and end. The faithful C is
 * c64-port/src/lm_kalcmanu_steering.c and cm_kalcmanu_steering.c.
 *
 * Basic code (the LM's in bank 22; the CM's in banks 20, 21 and 22, and
 * STOPRATE in fixed-fixed memory) round one stretch of interpretive code
 * (NEWANGL's, and MANUSTAT's EXIT), which is in the listing's notation,
 * run by the readable interpreter. The entries below are the basic
 * code's: the routines WAITLIST and FINDVAC start (UPDTCALL, NEWDELHI,
 * MANUSTOP), the words after each EXIT, the words BANKCALL, IBNKCALL,
 * WAITLIST, FINDVAC and SPVAC return to, and the CM's subroutines other
 * files call (STOPRATE, ZEROEROR, LOADCDUD, LOADYZ). The binding is
 * src/binding/kalcmanu_steering_routines.c. */
#ifndef READABLE_KALCMANU_STEERING_H
#define READABLE_KALCMANU_STEERING_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

extern const struct interpretive_code NEWANGL_CODE;
extern const struct interpretive_code MANUSTAT_CODE;

void newdelhi(void);
void newangl(void);
void newangl_after_exit(void);     /* NEWANGL +025 (the CM's INCRDCDU -1) */
void tmanuchk_after_timechk(void); /* TMANUCHK +1 */
void manustat_after_exit(void);    /* MANUSTAT +1 */
void updtcall(void);
void updtcall_after_findvac(void); /* UPDTCALL +4 */
void manustop(void);
#if LUMINARY099
void newdelhi_after_isitauto(void); /* NEWDELHI +2 */
void manustal_after_waitlist(void); /* MANUSTAL +4 */
void manucall_after_waitlist(void); /* MANUCALL +4 */
void goodmanu(void);
void goodmanu_after_spvac(void); /* GOODMANU +5 */
#else
void manustal_after_waitlist(void); /* MANUSTAL +3 */
void contmanu(void);
void manucall_after_waitlist(void); /* MANUCALL +3 */
void manustop_after_stopyz(void);   /* MANUSTOP +1 */
void endroll(void);
void endmanu(void);
void endmanu_after_spvac(void); /* ENDMANU +5 */
void stoprate(void);
void zeroeror(void);
void loadcdud(void);
void loadyz(void);
#endif

#endif
