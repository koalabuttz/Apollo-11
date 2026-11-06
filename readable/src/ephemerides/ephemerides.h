/* SPDX-License-Identifier: MIT */
/* LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc (Luminary099 pages 984-987,
 * Comanche055 pages 785-788). The faithful C is c64-port/src/
 * lm_ephemeris.c and cm_ephemeris.c.
 *
 * Where the sun and the moon are, seen from the earth, at a time the
 * caller gives. The two missions' files share their name and little else.
 *
 * The LM's LSPOS (alias LUNPOS) gives only their directions, unit vectors
 * in VSUN and VMOON, from mean angles that grow through the year and a few
 * periodic corrections, all constants of the rope. Its LUNVEL does
 * nothing.
 *
 * The CM's subroutines read a short ephemeris the ground loads before
 * launch: the moon's position as a polynomial in time, "VALID OVER A 15
 * DAY INTERVAL BEGINNING SHORTLY BEFORE LAUNCH", and the sun's position
 * and velocity at one time, which it turns about the earth at a fixed
 * rate. LSPOS gives both positions, LUNPOS the moon's, LUNVEL the
 * moon's velocity and SOLPOS the sun's.
 *
 * Both are all interpretive code: the table below, in the listing's
 * notation, which the readable interpreter runs. */
#ifndef READABLE_EPHEMERIDES_H
#define READABLE_EPHEMERIDES_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

/* The interpretive code. The LM's, in bank 15: LSPOS, POSITA, POSITB,
 * POSITD, POSITF, POSITE and LUNVEL. The CM's, in bank 26: LSPOS, LUNPOS,
 * LUNVEL and SOLPOS; LSTIME; RES and RESA; REM and REMA; VEM and VEMA;
 * and the two constants after them, NINEB4 and ONEB4. */
extern const struct interpretive_code EPHEMERIDES_CODE;

#endif
