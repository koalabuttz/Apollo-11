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
#include "lanec.h"
/* The trampoline (src/lanec_pin_trampoline.c): run the claiming modules f
 * (0-terminated) in turn on the mission's device record v; the first that
 * accepts gives the routine's result. */
typedef enum lanec_status (*lanec_pin_dispatch)(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#define LANEC_PIN_CELLS 13u
/* RUPTREG3, DSEXIT/EXITEM, WDRET/DECRET, 0116, UPDATRET, SFTEMP2, MIXTEMP,
 * ENTEXIT/ENTRET, DSPWDRET/RELRET/..., MPAC, MPAC+2, MPAC+6, REQRET: the
 * same addresses in both missions */
extern const uint16_t lanec_pin_cells[LANEC_PIN_CELLS];
extern const agc_routine_table lanec_pin_lm_routines, lanec_pin_cm_routines;
/* The keypress groups' routines (CHARIN and its key labels, src/lanec_pin_keys.c),
 * in their own tables and files so the flight store links each group as one
 * small module: number entry, and the other keys. */
extern const agc_routine_table lanec_pin_keys_num_lm_routines, lanec_pin_keys_num_cm_routines;
extern const agc_routine_table lanec_pin_keys_cmd_lm_routines, lanec_pin_keys_cmd_cm_routines;
/* The C64 round trips. Every module does not fit one C64 program, so each
 * program registers one module's points (those it claims first, from
 * lanec_pin_c64_points) and runs that module alone. lanec_pin_c64_select
 * builds lanec_pin_c64 for a module index (the pw-* suites in name order, then
 * pin-gaps) and mission (cm 0: Luminary099, 1: Comanche055); a program built
 * with LANEC_PIN_C64 runs LANEC_PIN_C64_RUN, its module's lanec_run, for it.
 * lanec_pin_c64_points gives every point of the mission with the index of the
 * module that claims it first (LANEC_PIN_SWRETURN: a BANKCALL return point,
 * entered through SWRETURN). */
typedef struct { uint16_t physical; uint8_t module; } lanec_pin_point;
/* A C64 program runs one mission (LANEC_PIN_C64_CM: Comanche055) and leaves
 * the other's code out. */
#if !defined(LANEC_PIN_C64) || !defined(LANEC_PIN_C64_CM)
#define LANEC_PIN_HAS_LM 1
#endif
#if !defined(LANEC_PIN_C64) || defined(LANEC_PIN_C64_CM)
#define LANEC_PIN_HAS_CM 1
#endif
#define LANEC_PIN_SWRETURN 0377u
#ifdef LANEC_PIN_HAS_LM
enum exec_status lanec_pin_run_lm(const agc_native_program *, apollo_overlay_state *, void *, const lanec_pin_dispatch *);
#endif
#ifdef LANEC_PIN_HAS_CM
enum exec_status lanec_pin_run_cm(const agc_native_program *, apollo_overlay_state *, void *, const lanec_pin_dispatch *);
#endif
extern agc_routine_table lanec_pin_c64;
void lanec_pin_c64_select(uint8_t module, int cm);
const lanec_pin_point *lanec_pin_c64_points(int cm, uint16_t *count);
#endif
