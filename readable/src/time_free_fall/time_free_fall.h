/* SPDX-License-Identifier: MIT */
/* TIME_OF_FREE_FALL.agc, both missions' (Luminary099 pp. 1268-1283,
 * Comanche055 pp. 1373-1388): TFFCONIC, TFFRP/RA, CALCTPER and CALCTFF,
 * in time_free_fall.c. The binding is
 * src/binding/time_free_fall_routines.c. */
#ifndef READABLE_TIME_FREE_FALL_H
#define READABLE_TIME_FREE_FALL_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#ifndef COUNT
#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#endif

/* A fixed label's bank, and its address in the bank, from its I_ value
 * (bank * 02000 + its place in the bank). */
#ifndef CODE_BANK
#define CODE_BANK(i) ((unsigned)((i) >> 10))
#define CODE_Z(i)    ((word)((i) % 02000u + 02000u))
#endif

/* TFFCONIC to TFFEL1's GOTO ENDTFF; the series after T(X)'s TC POLY; the
 * constants. */
extern const struct interpretive_code TFF_CODE, TFF_SERIES, TFF_CONSTANTS;

/* The basic code. */
void t_x(void);    /* T(X): TC POLY */
void endt_x(void); /* ENDT(X): TC DANZIG */

#endif
