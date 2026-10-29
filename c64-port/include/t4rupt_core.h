/* SPDX-License-Identifier: MIT */
#ifndef T4RUPT_CORE_H
#define T4RUPT_CORE_H
#include "agc_transfer.h"
/* The display and dispatch core of T4RUPT_PROGRAM.agc in both missions as
 * native routines of the control-transfer ABI (docs/T4RUPT-CORE.md): bank
 * 06,2000-2171 (LM) and 06,2000-2173 (CM), the same code but for the
 * erasable addresses of DSRUPTSW and IMODES33 and the T4JUMP table.
 *
 *   T4RUPT 06,2000   from the T4RUPT lead-in, inside the interrupt: the
 *                    120 ms phase (NORMT4, CDRVE, DSPOUT, DSPOUTSB/DSPSCAN/
 *                    DSPLAY or NODSPOUT, HANG20, SETTIME4, PROCEEDE) to
 *                    NOVAC (PROCKEY, on a PRO key change) or T4JUMP's
 *                    target, or the 20 ms quick-display phase (QUIKDSP,
 *                    QUIKRUPT, NODSPY, SYNCT4, QUIKOFF) to RESUME
 *   T4JUMP 06,2116   NOVAC's return: INDEX RUPTREG1; TCF +1
 *   LM 06,2121, 06,2125   RCSMONIT's returns: TCF RRAUTCHK
 *   CM OPTTEST 06,2130    TC IBNKCALL (CADR OPTDRIVE)
 * T4JUMP ends at LM RCSMONIT (TC, Q at the return), RRAUTCHK, IMUMON or
 * DAPT4S; CM OPTTEST's IBNKCALL, OPTMON, IMUMON or RESUME. WRITE OUT0 goes
 * to program->hardware->channel10, RXOR CHAN32 reads channel32.
 *
 * Domain: DSRUPTSW not above 8 when positive (RUPTREG1 indexes the
 * eight-entry T4JUMP table), DSPCNT 0-012 and no DSPTAB word -0, as the
 * display programs keep them; RUPTREG1 0-7 at T4JUMP. Anything else is
 * refused unchanged. */
extern const agc_routine_table lm_t4rupt_core_routines;
/* The same entries, packed by the flight store beside T4RUPT's radar step. */
extern const agc_routine_table lm_t4rupt_core_hot_routines;
extern const agc_routine_table cm_t4rupt_core_routines;
#endif
