/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, part 9 (Luminary099 pp. 975-983, bank 15): P57, the
 * lunar surface IMU alignment.
 *
 * So far only the two places the alignment's earlier pages go on to:
 * ATTCHK, where GVDETER's PROGRAV ends, and P57JUMP, where the surface
 * alignment (P57POST, B2F8) goes to align again. The rest of P57 is part
 * 9's. */
#include "p51_p53.h"
#include "../agc/names.h"

#if LUMINARY099

/* ---- ATTCHK and P57JUMP -- p. 980 ---- */

/* ATTCHK: a restart point (PHASCHNG, 04024), then "CHECK ATTFLAG". */
void attchk(void)
{
    tc_to(ATTCHK, PHASCHNG); /* OCT 04024 */
}

/* P57JUMP: a restart point (PHASCHNG, 04024), then "BRANCH ON OPTION
 * CODE". */
void p57jump(void)
{
    tc_to(P57JUMP, PHASCHNG); /* OCT 04024 */
}

#endif
