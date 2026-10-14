/* SPDX-License-Identifier: MIT */
#ifndef LM_DAPIDLER_H
#define LM_DAPIDLER_H
#include "agc_transfer.h"
#include "lm_flight_io.h"
/* Luminary099 DAPIDLER_PROGRAM.agc, all of it (bank 16, 02000-02212, and
 * bank 20, 02226-02446), as native routines of the control-transfer ABI;
 * io is p66_bias_io; channels 5, 6, 12, 14, 30 and 31 are the program's
 * agc_hardware (include/lm_flight_io.h).
 *
 *   CHEKBITS  16,2000  mode-select and IMU checks; JUMPDSP (DTCB to ALTDSPLY),
 *                      SHUTDOWN or MOREIDLE
 *   CHEKMORE  16,2016  PGNCS control check (channel 30 bit 10); MOREIDLE or
 *                      RETURN to CHEKBITS's caller
 *   DAPIDLER  16,2024  the T5RUPT idler: 1/ACCSET by NOVAC once, then CHECKUP
 *   CHECKUP   16,2041  (NOVAC's return) TC CHEKBITS
 *   16,2042            (CHEKBITS's return) ACCSOKAY test; STARTDAP's
 *                      TC IBNKCALL ZATTEROR, or MOREIDLE
 *   16,2050            (IBNKCALL's return) DAP initialization, SETTIME5,
 *                      TCF RESUME
 *   16,2161, 16,2163   (IBNKCALL returns from QERRCALC and CALCPERR) the
 *                      second call, SHUTDOWN: jets off, trim gimbal off,
 *                      T5ADR = DAPIDLER, SETTIME5, TCF RESUME
 *   ALTDSPLY  20,2226  attitude-error needles: alternation, DSPLYTOT with
 *                      OVERSUB2, NEEDLER initialization passes, the DAC loop
 *                      and channel 14 bits 13-15; RETNMORE (DTCB to
 *                      CHEKMORE)
 *
 * TC CHEKBITS from CHECKUP, MOREIDLE, SHUTDOWN, SETTIME5 and TC OVERSUB2 are
 * compiled where they are reached. TC NOVAC, TC IBNKCALL and TCF RESUME are
 * transfers. Refused (EXEC_OUT_OF_SCOPE, image unchanged): an entry outside
 * its bank or under an EB other than EBANK6 (EBANK= AOSQ / AK), a missing io
 * record, and a CHEKBITS or CHEKMORE return in Q that is not a TC into fixed
 * memory. */
extern const agc_routine_table lm_dapidler_routines;
#define LM_DI_FB16 034000u
#define LM_DI_FB20 040000u
#define LM_DI_EB 03000u
#define LM_DI_CHEKBITS 02000u
#define LM_DI_CHEKMORE 02016u
#define LM_DI_DAPIDLER 02024u
#define LM_DI_CHECKUP 02041u
#define LM_DI_ALTDSPLY 02226u
#define LM_DI_FIRST16 02000u
#define LM_DI_LAST16 02212u
#define LM_DI_FIRST20 02226u
#define LM_DI_LAST20 02446u
#define LM_DI_RESUME 05270u
#define LM_DI_IBNKCALL 04674u
#define LM_DI_NOVAC 05072u
#endif
