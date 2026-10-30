/* SPDX-License-Identifier: MIT */
/* Restarts (RESTARTS_ROUTINE.agc and RESTART_TABLES.agc): what the program
 * starts again after a restart.
 *
 * A restart clears the Executive and WAITLIST. GOPROG then runs RESTARTS
 * once for each restart group whose phase (PHASE_TABLE_MAINTENANCE) is
 * set, with the group in MPAC +5 and its phase in MPAC (TEMPPHS). The
 * phase names what to start: a job (FINDVAC or NOVAC), a task (WAITLIST)
 * or a LONGCALL, either from the group's own words (a "variable" phase,
 * stored by the call that set it) or from the restart tables (a "table"
 * phase, from RESTART_TABLES). An even table phase starts two things.
 *
 * RESTARTS ends at TC GOLOC -1, with the call it chose in GOLOC -1 and the
 * 2CADR to start in GOLOC; the call returns to GOLOC +2, which goes on to
 * the next group (SWRETURN) or to the second part (PHSPART2, GETPART2).
 *
 * Each entry is called with the machine as the original's TC leaves it and
 * returns with Z at the address the original continues at. */
#ifndef READABLE_RESTART_H
#define READABLE_RESTART_H

#include "../agc/agc.h"

/* RESTARTS_ROUTINE. */
void restarts(void);
void getpart2(void);
void getpart2_after_findvac(void);
void phspart2(void);
void goloc_minus1(void);
void goloc_plus2(void);

/* RESTART_TABLES: the tables, from SIZETAB on (bank 01), and the CM's
 * NEWPHASE. */
extern const word restart_tables[];
extern const unsigned restart_table_words;
#if COMANCHE055
void newphase(void);
#endif

#endif
