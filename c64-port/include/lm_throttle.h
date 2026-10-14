/* SPDX-License-Identifier: MIT */
#ifndef LM_THROTTLE_H
#define LM_THROTTLE_H
#include "agc_transfer.h"
#include "lm_flight_io.h"
/* Luminary099 THROTTLE_CONTROL_ROUTINES (bank 31, 02216-02417) and the
 * LUNAR_LANDING_GUIDANCE_EQUATIONS FASTCHNG it calls (31,3740) as native
 * routines of the control-transfer ABI; io is p66_bias_io, channel 14 the
 * program's agc_hardware (include/lm_flight_io.h).
 *
 *   THROTTLE   31,2216  acceleration from ABDELV, then THROTTLE +3
 *   THROTTLE+3 31,2221  return saved in RTNHOLD; TC MASSMULT (AFDUMP)
 *   31,2224             FP = present thrust; TC MASSMULT for /AFC/
 *   31,2230             FC, FCODD, FWEIGHT and the critical-band logic up to
 *                       TC FASTCHNG (or past it when TS PIFPSET overflows)
 *   31,2303             PIF, DOIT (THRUST, channel 14 bit 4, TTHROT) and
 *                       the FWEIGHT compensation; TC RTNHOLD
 *   FLATOUT    31,2370  4096 pulses through WHATOUT
 *   WHATOUT    31,2371  PIFPSET = A, FCOLD = PIF = -0, DOIT; TC RTNHOLD
 *   MASSMULT   31,2400  force = MPAC x MASS x SCALEFAC with TPAGREE and the
 *                       POSMAX limit; TC BUF
 *   FASTCHNG   31,3740  PHSNAME3 = return; TC A
 *
 * MASSMULT's TC DMP (twice) and TC TPAGREE, which call the INTERPRETER's
 * fixed-fixed basic subroutines DMP/DMPSUB and TPAGREE/BRANCH, are compiled
 * inline with the caller's ADRES operand as a constant. A return word (BUF,
 * RTNHOLD, A at FASTCHNG's TC A) that is not a TC into fixed memory has no
 * producer and is EXEC_OUT_OF_SCOPE, as is any entry outside bank 31. */
extern const agc_routine_table lm_throttle_routines;
#define LM_THR_FB 062000u
#define LM_THR_THROTTLE 02216u
#define LM_THR_THROTTLE3 02221u
#define LM_THR_FLATOUT 02370u
#define LM_THR_WHATOUT 02371u
#define LM_THR_MASSMULT 02400u
#define LM_THR_FASTCHNG 03740u
#define LM_THR_FIRST 02216u
#define LM_THR_LAST 02417u
#define LM_THR_RTNHOLD 01607u
#define LM_THR_BUF 0130u
#endif
