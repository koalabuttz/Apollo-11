/* SPDX-License-Identifier: MIT */
/* LATITUDE_LONGITUDE_SUBROUTINES.agc (Luminary099 pages 1133-1139,
 * Comanche055 pages 1236-1242). The faithful C is c64-port/src/
 * lm_lat_long.c and cm_lat_long.c.
 *
 * Between a position vector and latitude, longitude and altitude, over the
 * earth or the moon (LUNAFLAG):
 *
 *   LAT-LONG  "SUBROUTINE TO CONVERT RAD VECTOR AT GIVEN TIME TO
 *             LAT,LONG AND ALT": ALPHAV and the time in MPAC to LAT, LONG
 *             and ALT.
 *   LALOTORV  the other way: LAT, LONG, ALT and the time to ALPHAV.
 *
 * and their subroutines: GETERAD, the earth's radius at a latitude on an
 * ellipsoid; ARCTAN, an angle from its sine and cosine; SETGAMMA, the
 * ratio that turns latitude on the ellipsoid into the direction from the
 * centre; SETRE, the radius to measure altitude from.
 *
 * The two missions' files are the same code. The CM's keeps two constants
 * of its own among it, ERAD and 504RM; the LM's are elsewhere. All of it
 * is interpretive code: the table below, in the listing's notation, which
 * the readable interpreter runs. */
#ifndef READABLE_LAT_LONG_H
#define READABLE_LAT_LONG_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

/* The interpretive code, in bank 13, from LAT-LONG to the file's end:
 * LAT-LONG, LALOTORV, GETERAD and its constants, ARCTAN, SETGAMMA and
 * SETRE. */
extern const struct interpretive_code LAT_LONG_CODE;

#endif
