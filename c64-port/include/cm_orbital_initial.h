/* SPDX-License-Identifier: MIT */
#ifndef CM_ORBITAL_INITIAL_H
#define CM_ORBITAL_INITIAL_H
#include "overlay_runtime.h"
#include "agc_word.h"
/* DIFEQ0: native source initialization and JSWITCH route to ACCOMP/DOW.
 * Four phases, canonical image and separately owned interrupt latch (0/1).
 * Phases0..2 return at genuine DANZIG; phase3 returns directly NEWOPS and
 * enables interrupts as original SWITCHES RELINT. No job check there.
 * Caller excludes producers; invalid entries leave both objects unchanged. */
enum cm_orbital_initial_status {CM_ORBITAL_INITIAL_OK=0,CM_ORBITAL_INITIAL_OUT_OF_SCOPE=1};
enum cm_orbital_initial_status cm_orbital_initial_phase(apollo_overlay_state *,agc_word *interrupt_enabled,uint8_t phase);
#endif
