/* SPDX-License-Identifier: MIT */
/* P51-P53.agc of the CM: P51 (Comanche055 pp. 767-770, bank 15). So far
 * only SET1/PDT, which GYCRS (part 1) runs by RTB; the rest of P51 is a
 * later part's. */
#include "p51_p53_cm.h"
#include "../agc/names.h"

#if COMANCHE055

/* SET1/PDT: the present time (TIME1) to 1/PIPADT, back to DANZIG. (A is a
 * counter's word: TS does not skip.) */
void set1_pdt(void)
{
    ca(ADDRESS_OF(TIME1));
    ts(ADDRESS_OF(ONE_PIPADT));
    continue_at(DANZIG); /* TCF DANZIG */
}

#endif
