/* SPDX-License-Identifier: MIT */
#ifndef INTERP_CUT_H
#define INTERP_CUT_H
/* Truncated cases ('c64_cuts', tools/interp_routine_differential.py): a case
 * whose whole run reaches more routines than one C64 diagnostic holds runs
 * instead to its cut, the first NEWOPS with LOC at that physical address,
 * where the yaAGC oracle run of the same case also stops. */
#include "interp_case.h"
/* irc_run to the cut (0: no cut, the same as irc_run). IRC_STOPPED only at
 * the cut, with Z at the interpreter fetch point; IRC_FAILED when the run
 * ends anywhere else. */
unsigned irc_run_cut(const agc_native_program *, apollo_overlay_state *, const irc_case *, uint16_t cut,
                     uint16_t max_steps, uint16_t *ie);
#endif
