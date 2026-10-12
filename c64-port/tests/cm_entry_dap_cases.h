/* SPDX-License-Identifier: MIT */
#ifndef CM_ENTRY_DAP_CASES_H
#define CM_ENTRY_DAP_CASES_H
#include "cm_entry_dap.h"
#define CM_ENTRY_DAP_CASES UINT32_C(65536)
/* Row: case number (two words), status (always 0: every admitted case
 * reaches an exit), channels 5 and 6, then the 2048-word image. */
#define CM_ENTRY_DAP_ROW_WORDS 2053u
void cm_entry_dap_input(agc_image *, cm_entry_dap_io *, uint32_t n);
#endif
