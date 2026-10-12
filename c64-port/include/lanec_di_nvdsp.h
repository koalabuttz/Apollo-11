/* SPDX-License-Identifier: MIT */
#ifndef LANEC_DI_NVDSP_H
#define LANEC_DI_NVDSP_H
#include "lanec.h"
/* DISPLAY_INTERFACE_ROUTINES display start-up (both missions, bank 10),
 * PINBRNCH through V50PASTE: PINBRNCH, NORMBNCH, NVDSP (GOPLAY), NV50DSP,
 * BLANKCHK, PERFCHEK, GOANIDLE, ISITN00, 1STOR2ND and V50PASTE. Every basic
 * block is an entry and runs to its first boundary (ERASER, MARKPLAY,
 * UPFLAG, KEEPPRIO, PLAYJUM1, COPYPACS, CLEANEND, NVMONOPT, REST, FLASHOFF,
 * COPYTOGO, DOWNFLAG, BLANKSUB, FLASHSUB, ENDIT, ENDOFJOB or MARKPERF).
 * Out-of-scope input returns LANEC_OUT_OF_SCOPE before any change. */
enum lanec_status lanec_di_nvdsp_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
