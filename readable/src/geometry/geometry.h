/* SPDX-License-Identifier: MIT */
/* LEM_GEOMETRY.agc (Luminary099 pages 320-325) and CSM_GEOMETRY.agc
 * (Comanche055 pages 285-296). The faithful C is c64-port/src/
 * lm_geometry.c and cm_geometry.c.
 *
 * Small pieces of geometry other routines call. The LM's: SVDWN2 and
 * SVDWN1 put a state vector into the downlink's words; RRANGLES turns a
 * line of sight into the rendezvous radar's two pairs of gimbal angles;
 * RRNB and RRNBMPAC turn the radar's angles back into a line of sight.
 *
 * The CM's: SXTNB turns the sextant's angles into a line of sight;
 * CALCSXA and SXTANG turn a star's direction into sextant angles; LOG is
 * a natural logarithm; EARROT1 and EARROT2 turn a target on the earth
 * with the earth; SVDWN1 and SVDWN2 are the LM's SVDWN2 and SVDWN1.
 *
 * Nearly all of it is interpretive code: the tables below, in the
 * listing's notation, which the readable interpreter runs. The basic code
 * is RRANGLES's after its EXIT, SXTLOGIC (an RTB routine) and LOG's after
 * its EXIT. */
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
#else
void sxtlogic(void);       /* RTB SXTLOGIC: the sextant's trunnion offset */
void log_after_exit(void); /* LOG +4: TC POLY */
void log_after_poly(void); /* LOG +016: the shift count's log added in */

/* The interpretive code, in bank 23: SXTNB; CALCSXA, SXTANG, SXTANG1,
 * SXTALARM and ZNB=S1; LOG to its EXIT and its RVQ after the basic code;
 * EARROT1, EARROT2 and OVERADAY. In bank 10: SVDWN1 and SVDWN2. */
extern const struct interpretive_code SXTNB_CODE, CALCSXA_CODE, LOG_CODE, LOG_RVQ_CODE, EARROT_CODE,
    SVDWN_CODE;
#endif

#endif
