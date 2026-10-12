/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_STROKE_CASES_H
#define CM_TVC_STROKE_CASES_H
#include "cm_tvc_stroke.h"
#define CM_TVC_STROKE_CASES UINT32_C(65536)
/* Row: case number (two words), status (always 0: every admitted case
 * reaches a frontier), channel 14, interrupt enable, then the 2048-word
 * image. */
#define CM_TVC_STROKE_ROW_WORDS 2053u
void cm_tvc_stroke_input(agc_image *, cm_tvc_stroke_io *, uint32_t n);
#endif
