/* SPDX-License-Identifier: MIT */
#ifndef LANEC_DI_IDLE_H
#define LANEC_DI_IDLE_H
#include "lanec.h"
/* DISPLAY_INTERFACE_ROUTINES ENDIDLE handling and display returns (both
 * missions, bank 10), CLEANEND through TERMATE and PROCEED through ERASER:
 * every basic block is an entry and runs to its first boundary (FINDVAC,
 * UPENT2, DOWNENT2, NVSUBUSY, FLASHON, ENDIDLE, NVSUB, PRIOCHNG, SUPERSW,
 * NOVAC, BANKJUMP, ENDOFJOB, PRIOBORT, KEEPPRIO, MARKWAKE, WAKEPLAY,
 * GOSLEEPS, NORMBNCH or PINBRNCH). Out-of-scope input returns
 * LANEC_OUT_OF_SCOPE before any change. */
enum lanec_status lanec_di_idle_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
