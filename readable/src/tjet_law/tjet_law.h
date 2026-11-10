/* SPDX-License-Identifier: MIT */
/* TJET_LAW.agc, the LM's (Luminary099 pages 1460-1469, bank 17): TJETLAW,
 * the subroutine the RCS autopilot calls for the jet firing time of one
 * axis, P, U or V (AXISCTR -1, +0, +1). From the attitude error E and its
 * rate EDOT it stores the signed time in TJP, TJU or TJV and the jets to
 * use in NUMBERT, then returns by the Q it saved in HOLDQ. The fine law
 * works in the phase plane, zone by zone; the rough laws (RUFLAW1, -2 and
 * -3) take over when the error or the rate is large. The faithful C is
 * c64-port/src/lm_tjet_law.c.
 *
 * The whole routine is one entry, called by TC (or IBNKCALL) with the
 * return in Q. It ends at TC HOLDQ, or, in two branches no admitted input
 * reaches, at TC CCSHOLE. */
#ifndef READABLE_TJET_LAW_H
#define READABLE_TJET_LAW_H

#include "../agc/names.h"

#if LUMINARY099
void tjetlaw(void); /* TJETLAW, a subroutine */
#endif

#endif
