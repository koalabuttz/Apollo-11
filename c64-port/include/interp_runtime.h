/* SPDX-License-Identifier: MIT */
#ifndef INTERP_RUNTIME_H
#define INTERP_RUNTIME_H
/* Runtime interpreter (src/lm_interp_runtime.c, src/cm_interp_runtime.c):
 * INTERPRETER.agc run over the mission's rope image (include/rope_image.h),
 * one NEWOPS slot per call.
 *
 * lm_/cm_interp_runtime is the routine for any interpretive point (Z at
 * NEWOPS with LOC under BANKSET, or a pending DANZIG): a program uses it
 * where no compiled routine is registered. The basic tables hold the
 * interpreter's own basic entry points (SWRETURN, INTPRET, DANZIG).
 *
 * Contract as the compiled routines': canonical image, ordinary erasable
 * words, coherent banks, NEWJOB +0 or positive, FIXLOC a VAC area; the io
 * record is the mission's (p66_bias_io for LM, cm_native_io for CM). Refuses
 * (EXEC_OUT_OF_SCOPE) when the rope image is not loaded, LOC names erasable
 * memory, or the order is CCALL
 * or the unassigned address code 035. */
#include "agc_transfer.h"
#include "p66_bias.h"
extern const agc_routine lm_interp_runtime, cm_interp_runtime;
extern const agc_routine_table lm_interp_runtime_basic, cm_interp_runtime_basic;
#endif
