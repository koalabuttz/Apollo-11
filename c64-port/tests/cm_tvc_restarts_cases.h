/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_RESTARTS_CASES_H
#define CM_TVC_RESTARTS_CASES_H
#include "cm_tvc_restarts.h"
#define CM_TVC_RESTARTS_CASES UINT32_C(65536)
/* Row: case number (two words), status (always 0: every admitted case
 * reaches a frontier), channels 12 and 14, then the 2048-word image. */
#define CM_TVC_RESTARTS_ROW_WORDS 2053u
void cm_tvc_restarts_input(agc_image *, cm_tvc_restarts_io *, uint32_t n);
#endif
