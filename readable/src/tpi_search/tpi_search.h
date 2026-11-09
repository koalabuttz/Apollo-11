/* SPDX-License-Identifier: MIT */
/* TPI_SEARCH.agc, the CM's (Comanche055 pp. 551-561): P17 and P77, the
 * TPI search. The faithful C is c64-port/src/cm_tpi_search.c.
 *
 * "THE TPI SEARCH ROUTINE DETERMINES THE MINIMUM TOTAL VELOCITY TRANSFER
 * TRAJECTORY FROM A GIVEN TPI MANEUVER TIME WITHIN THE CONSTRAINT OF A
 * SAFE PERICENTER": the sum of the impulses at TPI (the terminal phase
 * initiation) and at TPF (its end). S17.1 brings both vehicles to the TPI
 * time, finds the phase angle between them and the altitude difference,
 * and from its sign picks the search sector (KFLAG). S17.2 bounds the
 * sector, turns its angles into the search's start and end times
 * (TIMETHET), and steps the transfer time TF through it, a Lambert
 * solution (INITVEL) at each step, until the solution's pericenter is
 * safe and the total velocity least, or "AS SOON AS IT IS EVIDENT THAT NO
 * SOLUTION EXISTS IN THE SECTOR SEARCHED" (ALARUMS).
 *
 * Written in three parts (README.md):
 *   part 1: s17.c, S17.1, S17.2, BIS, CONCAUL, ADVCSM and JUNCT3 (pp.
 *     551-556, bank 36): the search sector and the search loop up to its
 *     pericenter test.
 *   part 2: s17.c, JUNCT1 to NEXUS (pp. 556-558, bank 36): the step
 *     and its interpolation, the end of the search; transang.c, TRANSANG
 *     (pp. 558-559, bank 07): the passive's central angle of travel.
 *   part 3: P17, P77 and their displays (bank 35), the basic code.
 * The binding (src/binding/cm_tpi_search_routines.c) runs the faithful
 * port's keys on these routines, and on the faithful port's own for the
 * keys no part has written yet. */
#ifndef READABLE_TPI_SEARCH_H
#define READABLE_TPI_SEARCH_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if COMANCHE055

/* ---- Part 1 (s17.c, bank 36) ---- */
extern const struct interpretive_code S17_1_CODE, S17_2_CODE, BIS_CODE, CONCAUL_CODE, JUNCT3_CODE;

/* ---- Part 2 (s17.c, bank 36; transang.c, bank 07) ---- */
extern const struct interpretive_code JUNCT1_CODE, INTERP_CODE, HALFSAFE_CODE, LRGRDVO_CODE,
    RNGETEST_CODE, USEKAY_CODE, TRANSANG_CODE;

#endif

#endif
