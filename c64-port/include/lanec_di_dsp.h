/* SPDX-License-Identifier: MIT */
#ifndef LANEC_DI_DSP_H
#define LANEC_DI_DSP_H
#include "lanec.h"
/* DISPLAY_INTERFACE_ROUTINES display-start entries (both missions, bank 10):
 *   GODSP, GODSP2, GODSPRET, GODSPR, GODSPR1, GODSPR2, CLEANDSP, REFLASH,
 *   REFLASHR (LM), REGODSP, REGODSPR, CLOCPLAY, GOFLASH, GOFLASH2, PRIODSPR,
 *   PRIODSP, SETPRIO, EXDSPRET, GOPERF1/S/2/4, GOFLASHR, GODSPRS, GODSPRS1,
 *   GOPERF1R/RS/2R/4R (through PURRS4) -> MAKECADR (via SAVELOCS), NOVAC or
 *   SPVAC;
 *   the NOVAC/SPVAC returns and BOTHJOBS -> MAKECADR;
 *   SAVELOCS +7 (MAKECADR returns) -> MAKEPLAY, MAKECADR again or BANKJUMP;
 *   COPYNORM, COPIES, COPIES2 -> the caller's return;
 *   CHKPRIO -> MARSLEEP or SETNORM; MARKPERF -> NV50DSP.
 * Out-of-scope input returns LANEC_OUT_OF_SCOPE before any change. */
enum lanec_status lanec_di_dsp_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
