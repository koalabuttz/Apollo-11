/* SPDX-License-Identifier: MIT */
/* LEM_GEOMETRY.agc (Luminary099 pages 320-325). The faithful C is
 * c64-port/src/lm_geometry.c.
 *
 * Small pieces of geometry other routines call. SVDWN2 and SVDWN1 put a
 * state vector into the downlink's words; RRANGLES turns a line of sight
 * into the rendezvous radar's two pairs of gimbal angles; RRNB and
 * RRNBMPAC turn the radar's angles back into a line of sight.
 *
 * Nearly all of it is interpretive code: the tables below, in the
 * listing's notation, which the readable interpreter runs. The one piece
 * of basic code is RRANGLES's, after its EXIT. */
#ifndef READABLE_GEOMETRY_H
#define READABLE_GEOMETRY_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
void rrangles_after_exit(void); /* RRANGLES +042: swap MODEA and MODEB in mode 2 */

/* The interpretive code: SVDWN2 and SVDWN1, RRANGLES up to its EXIT and
 * its GOTO after the basic code, in bank 13; RRNB, RRNB1 and RRNBMPAC in
 * bank 23. */
extern const struct interpretive_code SVDWN_CODE, RRANGLES_CODE, RRANGLES_GOTO_CODE, RRNB_CODE;
#endif

#endif
