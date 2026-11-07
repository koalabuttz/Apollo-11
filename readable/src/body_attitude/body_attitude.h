/* SPDX-License-Identifier: MIT */
/* CM_BODY_ATTITUDE.agc (Comanche055 pp. 883-889, bank 37): the entry's
 * body attitude, every two seconds. SERVICER's average-G pass ends at
 * CM/POSE ("COME HERE VIA AVEGEXIT."), which works out from the state the
 * CM's attitude to the air stream, for the entry guidance and the entry
 * autopilot:
 *
 *   CM/POSE, CM/TRIO (cm_pose.c, pp. 883-886, interpretive): -VREL, the
 *     velocity relative to the turning atmosphere, and the "TRAJECTORY
 *     TRIAD" UXA/2, UYA/2, UZA/2 from it; the body axes UBX/2, UBY/2,
 *     UBZ/2 from the gimbal angles saved at PIPUP time; and from both the
 *     roll, the sideslip BETA, the angle of attack ALFA and the flight
 *     path angle GAMA.
 *   CM/ATUP, REDOPOSE, CM/POSE3, CORANGOV (cm_atup.c, pp. 887-889,
 *     basic): the "BASIC SUBROUTINE TO UPDATE ATTITUDE ANGLES" (and GAMA
 *     DOT), restart protected, and on at POSEXIT: "ENDEXIT, STARTENT, OR
 *     SCALEPOP." (REENTRY_CONTROL.agc).
 *
 * The constants (-KVSCALE, TCDU, REPOSADR, KWE from REENTRY_CONTROL.agc,
 * LIMITS from the fixed-fixed pool) are the rope's. The faithful C is
 * c64-port/src/cm_body_attitude.c.
 *
 * Each basic function runs from a word the program can resume at (an
 * entry, or REDOPOSE, where a restart comes back) to its TC INTPRET (run
 * on to NEWOPS). The interpretive code is the listing's words, run by the
 * readable interpreter. */
#ifndef READABLE_BODY_ATTITUDE_H
#define READABLE_BODY_ATTITUDE_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if COMANCHE055

/* CM/POSE and CM/TRIO -- cm_pose.c */
void cm_pose(void); /* CM/POSE */
extern const struct interpretive_code CM_POSE_CODE;

/* CM/ATUP to CORANGOV -- cm_atup.c */
void cm_atup(void);  /* CM/ATUP (CM/POSE's EXIT) */
void redopose(void); /* REDOPOSE */
extern const struct interpretive_code CM_POSE3_CODE;

#endif

#endif
