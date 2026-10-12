/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_MASSPROP_CASES_H
#define CM_TVC_MASSPROP_CASES_H
#include "cm_tvc_massprop.h"
#define CM_TVC_MASSPROP_CASES UINT32_C(65536)
/* Row: case number (two words), status (0 return, 2 no exit), IRQ enable,
 * then the 2048-word image after the source returns (zero when it never does). */
#define CM_TVC_MASSPROP_ROW_WORDS 2052u
void cm_tvc_massprop_input(agc_image *, agc_word *irq, uint32_t n);
#endif
