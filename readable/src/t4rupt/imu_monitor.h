/* SPDX-License-Identifier: MIT */
/* T4RUPT, part 2: the IMU monitor (T4RUPT_PROGRAM.agc, IMUMON to
 * LAMPTEST).
 *
 * Once a second T4JUMP comes to IMUMON, which watches the IMU's status
 * bits (channel 30) and channel 33's flip-flops, runs the IMU's turn-on and
 * cage sequences, keeps the TEMP, ISS WARNING, GIMBAL LOCK and NO ATT
 * lamps, and watches the middle gimbal angle for gimbal lock. It ends at
 * RESUME, or leaves for a routine in another file (ALARM, WAITLIST,
 * IBNKCALL, ZEROICDU and the rest), which comes back to one of the
 * entries below.
 *
 * Each function below is an entry: a label, or the word after a call out
 * of the file, where that routine returns (the name says which). */
#ifndef READABLE_IMU_MONITOR_H
#define READABLE_IMU_MONITOR_H

#include "../agc/names.h"

/* The calls whose place differs between the missions: ISSUP's TC SETISSW
 * (the LM's comes after seven words that set DRIFTFLG) and IMUCAGE's TC
 * CAGESUB1 (the LM turns the engine off in eight words, the CM in three). */
#if LUMINARY099
#define ISSUP_TC_SETISSW    ((word)(ISSUP + 015u))
#define IMUCAGE_TC_CAGESUB1 ((word)(IMUCAGE + 021u))
#else
#define ISSUP_TC_SETISSW    ((word)(ISSUP + 6u))
#define IMUCAGE_TC_CAGESUB1 ((word)(IMUCAGE + 013u))
#endif

/* Channel 30 and the turn-on sequence (pages "IMUMON" and "TNONTEST"). */
void imumon(void);
void proctnon_after_alarm(void);    /* PROCTNON +014 ("+3") */
void proctnon_after_waitlist(void); /* PROCTNON +021: TCF C33TEST */
void endtnon(void);                 /* the 90-second task */
void unz2(void);                    /* a task */
void unz2_after_zeroicdu(void);     /* UNZ2 +1 */
void issup(void);                   /* VARDELAY's task */
void issup_after_waitlist(void);    /* TCF TASKOVER */
void isszero_after_noattoff(void);  /* ISSZERO +2 */
void isszero_after_zeroicdu(void);  /* ISSZERO +6 */
void isszero_after_waitlist(void);  /* ISSZERO +012: TCF C33TEST */

/* Gimbal lock (GLOCKMON). */
void glockchk_after_setcoars(void); /* GLOCKCHK +015 */
void nogimrun(void);                /* also WAITLIST's return */

/* The routines of the channel 30 bits. */
void iturnon_after_alarm(void);    /* ITURNON +022: TCF NXTIFAIL */
void imucage_after_rndrefdr(void); /* IMUCAGE_TC_CAGESUB1 +3 */
void imuop_after_rndrefdr(void);   /* IMUOP +7 */
void imuop_after_alarm(void);      /* IMUOP +020: TCF NXTIFAIL */

/* The routines of the channel 33 bits. */
void pipfail_after_alarm(void);  /* PIPFAIL +020: TCF NXTFL33 */
void dntmfast_after_alarm(void); /* DNTMFAST +4: TCF NXTFL33 */
void uptmfast_after_alarm(void); /* UPTMFAST +4: TCF NXTFL33 */

/* SETISSW, the ISS WARNING lamp: true if it returned by TC Q, false if it
 * left for VARALARM (which comes back to isswon_after_varalarm). */
bool setissw(void);
void isswon_after_varalarm(void); /* ISSWON +3 */

#endif
