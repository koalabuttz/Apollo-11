/* SPDX-License-Identifier: MIT */
#ifndef LM_TRIM_GIMBAL_H
#define LM_TRIM_GIMBAL_H
#include "agc_transfer.h"
#include "lm_flight_io.h"
/* Luminary099 TRIM_GIMBAL_CONTROL_SYSTEM.agc, all of it (bank 21,
 * 03077-03645, and bank 16, 03735-03771), as native routines of the
 * control-transfer ABI; io is p66_bias_io; channel 12 is the program's
 * agc_hardware (include/lm_flight_io.h).
 *
 *   GTS       21,3077  the trim gimbal attitude control law for the R and Q
 *                      axes (DEL, NEGUSUM with GTSQRT and ROOTCYCL, the new
 *                      gimbal drives and drive reversals in channel 12);
 *                      DTCB to CLOSEOUT (Q_R-AXIS_RCS_AUTOPILOT)
 *   ACDT+C12  16,3736  QACCDOT/RACCDOT and the channel 12 drive bits from
 *                      NEGUQ/NEGUR; TC Q
 *   TIMEGMBL  21,3330  the drive times that null AOSQ/AOSR (Q(R)GIMTIMR,
 *                      NEGUQ/R, ALLOWGTS); TC IBNKCALL to ACDT+C12
 *   21,3435            (IBNKCALL's return) TC RUPTREG2 to TIMEGMBL's caller
 *
 * GTSQRT and ROOTCYCL are entered only by TC from GTS and are compiled at
 * their call sites, leaving Q as the original's TC Q and TC FUNCTION +1 do.
 * Refused (EXEC_OUT_OF_SCOPE, image unchanged): an entry outside its bank or
 * under an EB other than EBANK6 (EBANK= QDIFF / NEGUQ), a missing io record,
 * an ACDT+C12 return in Q or a TIMEGMBL return in RUPTREG2 that is not a TC
 * into fixed memory, and a GTS pass whose GTSQRT takes the zero root with
 * ACENTRAL/2 (ININDEX is the same cell) outside -2525 .. +1570, where
 * DOSHIFT's INDEX SHFTFLAG turns CA BIT15 into another order. Inside that
 * range the indexed CA reads 04735 + ACENTRAL/2 (erasable memory, or bank
 * 21 and fixed-fixed from src/lm_trim_gimbal_ca_table.inc). */
extern const agc_routine_table lm_trim_gimbal_routines;
#define LM_TG_FB21 042000u
#define LM_TG_FB16 034000u
#define LM_TG_EB 03000u
#define LM_TG_GTS 03077u
#define LM_TG_ACDTC12 03736u
#define LM_TG_TIMEGMBL 03330u
#define LM_TG_TIMERET 03435u
#define LM_TG_FIRST21 03077u
#define LM_TG_LAST21 03645u
#define LM_TG_FIRST16 03735u
#define LM_TG_LAST16 03771u
#define LM_TG_CLOSEOUT 03236u /* bank 17 */
#define LM_TG_IBNKCALL 04674u
#endif
