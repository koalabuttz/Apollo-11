/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_ROLL_CASES_H
#define CM_TVC_ROLL_CASES_H
#include "cm_tvc_roll.h"
#define CM_TVC_ROLL_CASES UINT32_C(65536)
/* Row: case number (two words), status (always 0: every admitted case
 * reaches TASKOVER or NOQRSM), channels 6 and 13, then the 2048-word image. */
#define CM_TVC_ROLL_ROW_WORDS 2053u
void cm_tvc_roll_input(agc_image *, cm_tvc_roll_io *, uint32_t n);
#endif
