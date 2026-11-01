/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, R23 and R24 (Luminary099 pp. 513-516). Parts 1 and 2 need
 * only R23LEM's and R24LEM's first steps, which P20 and R21 run; the rest
 * of R23, manual acquisition, and R24, the search, is part 3. */
#include "p20_p25.h"
#include "../agc/names.h"

#if LUMINARY099

/* R23LEM -- p. 513. "SET NO ANGLE MONITOR FLAG": UPFLAG, NORRMON. */
void r23lem(void)
{
    tc_at(R23LEM);
    continue_at(UPFLAG); /* ADRES NORRMON */
}

/* R24LEM -- p. 515. "SET SRCHOPT FLAG": UPFLAG, SRCHOPTN. */
void r24lem(void)
{
    tc_at(R24LEM);
    continue_at(UPFLAG); /* ADRES SRCHOPTN */
}

#endif
