/* SPDX-License-Identifier: MIT */
#ifndef SP_ROUTINES_H
#define SP_ROUTINES_H
/* SINGLE_PRECISION_SUBROUTINES' SPSIN and SPCOS on the transfer ABI: entered
 * at their original addresses (Luminary099 SPCOS 05032, SPSIN 05033;
 * Comanche055 SPCOS 05074, SPSIN 05075), each runs the certified routine
 * (lm_single_trig, cm_single_trig) and its TC Q return (lm_single_trig_return,
 * cm_single_return), leaving Z at the caller's return. A return Q outside
 * 02000-07777 is refused before anything is written (EXEC_OUT_OF_SCOPE), as
 * is any entry state the certified routines refuse. Build src/sp_routines.c
 * with RK_LM=1 or RK_CM=1. */
#include "agc_transfer.h"
extern const agc_routine_table lm_sp_routines,cm_sp_routines;
#endif
