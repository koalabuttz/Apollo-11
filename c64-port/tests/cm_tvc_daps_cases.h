/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_DAPS_CASES_H
#define CM_TVC_DAPS_CASES_H
#include "cm_tvc_daps.h"
#define CM_TVC_DAPS_CASES UINT32_C(65536)
/* Row: case number (two words), status (always 0: every admitted case
 * reaches RESUME, NOQRSM, HACK or the caller's return), channel 14, a zero
 * word, then the 2048-word image. */
#define CM_TVC_DAPS_ROW_WORDS 2053u
void cm_tvc_daps_input(agc_image *, cm_tvc_daps_io *, uint32_t n);
#endif
