/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc (Luminary099 pages 1205-1226, Comanche055
 * pages 1309-1333), in part: so far its state vector moves, which
 * MEASUREMENT_INCORPORATION reaches by RTB. The faithful C is c64-port/src/
 * lm_integration_moves.c and cm_integration_moves.c.
 *
 * The orbital integration works on its own state vector, RRECT and the 41
 * words after it (position, velocity, the deviations from the conic, the
 * times and the rest). Each vehicle keeps a permanent copy: RRECTCSM for
 * the CSM, RRECTLEM for the LM. Each move copies one into the other,
 * through SETBANK (the integration's banks), last word first, and ends at
 * DANZIG, as a routine an RTB runs does. */
#ifndef READABLE_INTEGRATION_INIT_H
#define READABLE_INTEGRATION_INIT_H

#include "../agc/agc.h"

void moveacsm(void); /* MOVEACSM: RRECT to RRECTCSM */
void movepcsm(void); /* MOVEPCSM: RRECTCSM to RRECT */
void movealem(void); /* MOVEALEM: RRECT to RRECTLEM */
void moveplem(void); /* MOVEPLEM: RRECTLEM to RRECT */

#endif
