/* SPDX-License-Identifier: MIT */
#ifndef CM_AUTO_MAN_CASES_H
#define CM_AUTO_MAN_CASES_H
#include "cm_auto_man.h"
#define CM_AUTO_MAN_CASES UINT32_C(65536)
/* Row: case number (two words), status (always 0: every admitted case
 * reaches an exit), two zero words (the file writes no channel), then the
 * 2048-word image. */
#define CM_AUTO_MAN_ROW_WORDS 2053u
void cm_auto_man_input(agc_image *, cm_auto_man_io *, uint32_t n);
#endif
