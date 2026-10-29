/* SPDX-License-Identifier: MIT */
#ifndef EPHEMERIS_ADD_H
#define EPHEMERIS_ADD_H
#include "overlay_runtime.h"
#include "fixed_data.h"
#include "agc_word.h"
/* Native original interpreter TAD basic arithmetic, needed by both mission
 * ephemerides. Ordinary erasable triple at ADDRWD, with raw16 A/L, coherent
 * bank registers, REGZERO+0 and exclusive canonical image ownership. Reads
 * the major operand after the minor DAS, retaining real scratch aliases.
 * Stops at original DANZIG; the following interpreter pair is not executed.
 * IRQ/SUPER/device state is external and untouched. Invalid input returns0
 * with the complete image unchanged. This does not implement ephemerides. */
uint8_t ephemeris_triple_add(apollo_overlay_state *,enum apollo_fixed_mission);
#endif
