/* SPDX-License-Identifier: MIT */
/* PLANETARY_INERTIAL_ORIENTATION.agc (Luminary099 pages 1140-1148,
 * Comanche055 pages 1243-1251). The faithful C is c64-port/src/
 * lm_planetary.c and cm_planetary.c.
 *
 * Between the reference axes (the basic inertial system) and a planet's
 * own, which turn with it, the earth's or the moon's (MPAC zero for the
 * earth, not zero for the moon):
 *
 *   RP-TO-R   a vector in the planet's axes to the reference axes,
 *             "R=MT(T)*(RP+LPXRP)".
 *   R-TO-RP   the other way, "RP=M(T)*(R-LXR)".
 *
 * and their subroutines: MOONMX and EARTHMX, the matrix M that turns the
 * reference axes into the planet's at a time; NEWANGLE, an angle that
 * grows at a steady rate, "X=X0+(XDOT)(T+T0)"; EARTHL, the earth's small
 * vector L.
 *
 * The two missions' files are the same code. The LM's is spread over four
 * banks (24, 25, 26 and 6), so MOONMX ends in a GOTO to the rest of it,
 * MOONMXA, in bank 25; the CM's is all in bank 26 and runs on. The CM's
 * keeps the moon's and the earth's constants (COSI to WEARTH) at its end;
 * the LM's are in CONTROLLED_CONSTANTS. All of it is interpretive code:
 * the tables below, in the listing's notation, which the readable
 * interpreter runs. */
#ifndef READABLE_PLANETARY_H
#define READABLE_PLANETARY_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

/* The interpretive code, part by part as the listing places it: RP-TO-R;
 * R-TO-RP and MOONMX; the rest of MOONMX (the LM's MOONMXA) and NEWANGLE;
 * EARTHMX; EARTHL; and in the CM the constants. */
#if LUMINARY099
#define PLANETARY_PARTS 5u
#else
#define PLANETARY_PARTS 6u
#endif
extern const struct interpretive_code PLANETARY_CODE[PLANETARY_PARTS];

#endif
