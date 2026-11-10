/* SPDX-License-Identifier: MIT */
/* TRIM_GIMBAL_CONTROL_SYSTEM.agc, the LM's (Luminary099 pages 1472-1484,
 * bank 21; ACDT+C12 in bank 16): the descent engine's trim gimbal as an
 * attitude controller for the Q and R axes.
 *
 *   GTS       the Q,R-axes autopilot's alternative to the RCS jets: for
 *             each axis, the drive that steers attitude error, rate and
 *             offset acceleration to zero together (the sign of NEGUSUM),
 *             into NEGUQ or NEGUR, a drive reversal stopping the gimbal
 *             for a pass; then DTCB to CLOSEOUT, Q_R-AXIS's end of a pass
 *   ACDT+C12  the drive bits of channel 12 from NEGUQ and NEGUR, and the
 *             acceleration rates they give (QACCDOT, RACCDOT); TC Q
 *   TIMEGMBL  the drive times that null the offset accelerations AOSQ and
 *             AOSR (QGIMTIMR, RGIMTIMR), then ACDT+C12 by IBNKCALL
 *   timegmbl_return  where that IBNKCALL comes back: TC RUPTREG2
 *
 * GTSQRT and ROOTCYCL, entered only by TC from GTS, are functions of it.
 * The faithful C is c64-port/src/lm_trim_gimbal.c. */
#ifndef READABLE_TRIM_GIMBAL_H
#define READABLE_TRIM_GIMBAL_H

#include "../agc/names.h"

#if LUMINARY099
void gts(void);             /* GTS: from Q_R-AXIS by DTCB */
void acdt_c12(void);        /* ACDT+C12: by IBNKCALL, Q the return */
void timegmbl(void);        /* TIMEGMBL: by TC, Q the return */
void timegmbl_return(void); /* DONEYET +6, where ACDT+C12 returns */
#endif

#endif
