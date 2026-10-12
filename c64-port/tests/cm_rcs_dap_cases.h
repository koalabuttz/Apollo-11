/* SPDX-License-Identifier: MIT */
#ifndef CM_RCS_DAP_CASES_H
#define CM_RCS_DAP_CASES_H
#include "cm_rcs_dap.h"
#define CM_RCS_DAP_CASES UINT32_C(65536)
/* Row: case number (two words), status (always 0: every admitted case
 * reaches an exit), channels 12, 13 and 14, then the 2048-word image. */
#define CM_RCS_DAP_ROW_WORDS 2054u
void cm_rcs_dap_input(agc_image *, cm_rcs_dap_io *, uint32_t n);
#endif
