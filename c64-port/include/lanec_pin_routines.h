/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PIN_ROUTINES_H
#define LANEC_PIN_ROUTINES_H
/* PINBALL_GAME_BUTTONS_AND_LIGHTS (Luminary099 and Comanche055) as native
 * routines of the control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). Every
 * point at which a pw-* suite enters its earlier Pinball module (through the
 * module's lanec_run, src/lanec_pw*_runs.c), or pin-gaps its own, is
 * registered at its physical address (src/lanec_pin_routines.inc,
 * tools/lanec/routines.py pin). The earlier modules overlap, so a routine
 * tries the modules that claim its point in turn, each on the state as it
 * came in, and keeps the first that accepts; LANEC_OUT_OF_SCOPE from all of
 * them is EXEC_OUT_OF_SCOPE.
 *
 * io is the mission's downtelemetry device record, as for EXTENDED_VERBS: an
 * lm_dntm_io for Luminary (channels 12-14; output channel 11 comes from
 * program->hardware, and the routines refuse when that is NULL) and a
 * cm_dntm_io for Comanche (its cm_native_io carries channels 11-14 and the
 * keyboard channels 15 and 16).
 *
 * Pinball returns through erasable words: TC Q, TC L, and the cells listed
 * in lanec_pin_cells (TC DSEXIT, TC WDRET, TC ENTRET, ...). When a module
 * ends at one of them and it holds a TC (not to itself, and not RELINT,
 * INHINT or EXTEND) or a TCF into fixed memory, the routine also executes
 * that one instruction (Z = its operand; for a TC, Q = the word's address
 * + 1), so the next routine starts where the original program goes. */
#include "agc_transfer.h"
#define LANEC_PIN_CELLS 13u
/* RUPTREG3, DSEXIT/EXITEM, WDRET/DECRET, 0116, UPDATRET, SFTEMP2, MIXTEMP,
 * ENTEXIT/ENTRET, DSPWDRET/RELRET/..., MPAC, MPAC+2, MPAC+6, REQRET: the
 * same addresses in both missions */
extern const uint16_t lanec_pin_cells[LANEC_PIN_CELLS];
extern const agc_routine_table lanec_pin_lm_routines, lanec_pin_cm_routines;
/* The C64 round-trip program's table (it cannot hold every module; built
 * with LANEC_PIN_C64 it has only this one): the Luminary099 points of the
 * decimal display module (pw-decimal, DSPDECWD), running that module alone. */
extern const agc_routine_table lanec_pin_lm_c64;
#endif
