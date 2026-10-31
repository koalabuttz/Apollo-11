/* SPDX-License-Identifier: MIT */
/* Down-telemetry (DOWN-TELEMETRY_PROGRAM.agc): every 20 ms the DOWNRUPT
 * interrupt sends two words down to the ground, in channels 34 and 35
 * (DNTM1, DNTM2).
 *   - Normally the words come from the current downlist: a control list of
 *     DNADRs (addresses of 1 to 6 double words), DNCHANs (a pair of
 *     channels) and DNPTRs (pointers to sublists), 200 words every 2
 *     seconds. A snapshot sublist is copied all at once into DNTMBUFF, so
 *     its words belong together.
 *   - On V74 the erasable dump sends all of erasable memory instead, bank
 *     by bank, one to four times.
 *
 * DODOWNTM starts each DOWNRUPT and ends at TC DNTMGOTO: DNTMGOTO holds
 * where this DOWNRUPT goes on (DNPHASE1, DNPHASE2, DNDUMPI, DNDUMPI +3,
 * DNDUMP, DNDUMP1), run as the instruction TC <address>. Each of those
 * ends at DNTMEXIT, which writes the channels and returns through RESUME. */
#ifndef READABLE_DOWNTELEMETRY_H
#define READABLE_DOWNTELEMETRY_H

#include "../agc/agc.h"

void dodowntm(void);
void dntmgoto(void);

void dnphase1(void);
void dnphase2(void);

void dndumpi(void);
void dndumpi_plus3(void);
void dndump(void);
void dndump1(void);

#endif
