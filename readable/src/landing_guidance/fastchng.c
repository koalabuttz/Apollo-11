/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, p. 826 (Luminary099 only): the
 * "SPECIALIZED 'PHASCHNG' ROUTINE". */
#include "landing_guidance.h"
#include "../agc/names.h"

#if LUMINARY099

/* FASTCHNG: PHSNAME3, restart group 3's phase, gets the caller's return.
 * XCH EBANK switches in E-bank 3 and leaves the caller's EBANK in A; DXCH L
 * moves the return from Q to A and that EBANK to L; LXCH EBANK puts it
 * back, and TC A returns. The return must be a TC to fixed memory (02000
 * to 07777), and then TS PHSNAME3 never skips; any other is not
 * modelled. */
void fastchng(void)
{
    if (Q < 02000u || Q > 07777u) not_modelled();
    set_A(EBANK3);
    xch(EBANK_ADDRESS);
    dxch(L_ADDRESS);
    if (!ts(ADDRESS_OF(PHSNAME3))) lxch(EBANK_ADDRESS);
    tc_through(A_ADDRESS);
}

#endif
