/* SPDX-License-Identifier: MIT */
#ifndef RESIDENT_VECTOR_RUN_H
#define RESIDENT_VECTOR_RUN_H
/* The host model of a resident_vector_cases.h case through the flight
 * resident: the entry (rk_rupt_take), then the routine at each point from
 * the resident's tables (kernel, single precision) or the wrapped interrupt
 * bodies' tables (lead-ins, LM T6RUPT, T4RUPT core, KEYRUPT/UPRUPT), until
 * RESUME brings Z back to the interrupted point, a point no table holds, a
 * routine's refusal or RR_V_MAX routines. The C64 resident runs the same
 * routines: its own tables first, the wrappers' through the resolver. */
#include <stdbool.h>
#include "resident_vector_cases.h"
#define RR_V_MAX 4000u
typedef bool (*rr_fixed)(const agc_native_program *, uint16_t physical, agc_word *word);
/* *point: where the run stopped (the interrupted point when resumed);
 * *routines: how many routines ran. */
uint8_t rr_vector_run(unsigned mission, rr_vcase *c, rr_fixed fixed, uint16_t *point, uint16_t *routines);
/* Every table the model runs, and how many (resident ones first). */
const agc_routine_table *rr_vector_tables(unsigned mission, uint8_t *count, uint8_t *resident);
#endif
