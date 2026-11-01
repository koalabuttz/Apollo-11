/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, R23 and R24 (Luminary099 pp. 513-516). Part 1 needs only
 * R23LEM's first step, which P20 runs; the rest of R23, manual
 * acquisition, and R24, the search, is part 3. */
#include "p20_p25.h"
#include "../agc/names.h"

#if LUMINARY099

/* R23LEM -- p. 513. "SET NO ANGLE MONITOR FLAG": UPFLAG, NORRMON. */
void r23lem(void)
{
    tc_at(R23LEM);
    continue_at(UPFLAG); /* ADRES NORRMON */
}

#endif
