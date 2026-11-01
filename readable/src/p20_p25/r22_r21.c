/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, R22 and R21 (Luminary099 pp. 504-512). Part 1 needs only
 * LUNSFCHK, which P20 calls; the rest of R22, the radar data read, and
 * R21, the designate, is part 2. */
#include "p20_p25.h"
#include "../agc/names.h"

#if LUMINARY099

/* TC Q: the routine returns to the address in Q (bits 1-12), and Q
 * becomes 3. Not modelled: a Q that is not such an address (bits 13-15
 * set, or below 010). */
static void tc_q(void)
{
    word at = (word)(Q & 07777u);

    if ((Q & 070000u) || Q < 010u) not_modelled();
    Q = 3u;
    Z = at;
}

/* LUNSFCHK -- p. 507. "LUNSFCHECK -- CLOSED SUBROUTINE TO CHECK IF ON
 * LUNAR SURFACE (P22FLAG) RETURNS TO CALLER +1 IF P22FLAG SET TO CALLER +2
 * IF P22FLAG NOT SET": the flag is FLAGWRD8's SURFFLAG. (The masked
 * complement is never negative: not modelled.) */
void lunsfchk(void)
{
    cs(ADDRESS_OF(FLAGWRD8)); /* "CHECK IF ON LUNAR SURFACE" */
    mask(SURFFBIT);           /* "IS SURFFLAG SET?" */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        incr(Q_ADDRESS);
        tc_q();
        return;                        /* "NOT SET" */
    case IS_PLUS_ZERO: tc_q(); return; /* "P22FLAG SET" */
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

#endif
