/* SPDX-License-Identifier: MIT */
#ifndef LANEC_DI_MARK_H
#define LANEC_DI_MARK_H
#include "lanec.h"
/* DISPLAY_INTERFACE_ROUTINES mark displays (both missions):
 *   BLANKET, CLEARMRK (+2), ENDMARK (fixed-fixed) -> the caller's return or
 *   POSTJUMP; MARKEND -> MARKOVER;
 *   GOMARK, GOMARS, KLEENEX, GOMARKF, GOMARK2, MARKFORM, GOMARK3, GOMARK4,
 *   GOMARKR, GOMARKFR, GOMARK2R, GOMARK3R (bank 10) -> the display-start
 *   paths of lanec_di_dsp (MAKECADR, NOVAC or SPVAC);
 *   MAKEMARK -> CHKPRIO's exits, MARKPLAY's exit or ENDOFJOB;
 *   MARKPLAY, GOGOMARK, MARKCOP -> PRIOPLAY;
 *   COPYTOGO, COPYPACS -> the caller's return.
 * Out-of-scope input returns LANEC_OUT_OF_SCOPE before any change. */
enum lanec_status lanec_di_mark_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
