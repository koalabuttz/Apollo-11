/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 6 (Comanche055 pp. 631-632): GOTOV56, in fixed-fixed
 * memory (BLOCK 02), written with RENDISP2, which ends P20 through it. */
#include "p20_p25_cm.h"
#include "../agc/names.h"

#if COMANCHE055

/* GOTOV56 -- p. 631: "P20 TERMINATES BY GOTOV56 INSTEAD OF", "GOTOPOOH":
 * TRACKTRM's 2CADR (VB56CADR), verb 56's, through SUPDXCHZ. */
void gotov56(void)
{
    set_L(VB56CADR[1]); /* EXTEND; DCA VB56CADR */
    set_A(VB56CADR[0]);
    continue_at(SUPDXCHZ); /* TCF SUPDXCHZ */
}

#endif
