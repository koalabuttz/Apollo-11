/* SPDX-License-Identifier: MIT */
#ifndef CM_JET_SELECT_CASES_H
#define CM_JET_SELECT_CASES_H
#include "cm_jet_select.h"
#define CM_JET_SELECT_CASES UINT32_C(65536)
/* Row: case number (two words), status (always 0: every admitted case
 * reaches RESUME), channels 5, 6 and 13, then the 2048-word image. */
#define CM_JET_SELECT_ROW_WORDS 2054u
void cm_jet_select_input(agc_image *, cm_jet_select_io *, uint32_t n);
#endif
