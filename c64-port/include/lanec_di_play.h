/* SPDX-License-Identifier: MIT */
#ifndef LANEC_DI_PLAY_H
#define LANEC_DI_PLAY_H
#include "lanec.h"
/* DISPLAY_INTERFACE_ROUTINES display job selection (both missions, bank 10):
 *   MAKEPLAY -> PRIOCHNG; MAKEPLAY +5 (PRIOCHNG returns) through MAKEPRIO,
 *   IFLEGAL or MAKEMARK;
 *   MAKEPRIO, MAKEPRIO +4 (Q+1 returns), HIPRIO, SETMARK, ASKIFNRM, SETNORM,
 *   OKTOCOPY -> Q+1, POODOO, JOBXCHS, JOBWAKE or ENDOFJOB;
 *   OKTOCOPY +3 (JOBWAKE returns), REDOPRIO, KEEPPRIO -> GOPLAY;
 *   IFLEGAL, IFLEGAL +4 (Q+1 returns), PRIOBORT, OKTOPLAY -> Q+1, POODOO,
 *   GOSLEEPS, JOBWAKE or ENDOFJOB; OKTOPLAY +13 (JOBWAKE returns),
 *   PLAYJUM1, PRIOPLAY -> GOPLAY;
 *   LINUSCHR, WITCHONE, XCHTOEND, XCHNYLOC -> the caller's return
 *   (LINUSCHR also Q+1 or ENDOFJOB).
 * Out-of-scope input returns LANEC_OUT_OF_SCOPE before any change. */
enum lanec_status lanec_di_play_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
