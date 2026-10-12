/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_EXEC_CASES_H
#define CM_TVC_EXEC_CASES_H
#include "cm_tvc_exec.h"
#define CM_TVC_EXEC_CASES UINT32_C(65536)
/* Row: case number (two words), status (0 stopped at a frontier, 2 no exit),
 * IRQ enable, then the 2048-word image at the frontier (zero for no exit). */
#define CM_TVC_EXEC_ROW_WORDS 2052u
void cm_tvc_exec_input(agc_image *, agc_word *irq, agc_word *channel11, uint32_t n);
#endif
