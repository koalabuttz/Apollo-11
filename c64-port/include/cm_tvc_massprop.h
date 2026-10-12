/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_MASSPROP_H
#define CM_TVC_MASSPROP_H
#include "agc_image_ops.h"
/* Comanche055 TVCMASSPROP: MASSPROP (5,3366) and its FIXCW entry (5,3422),
 * compiled one original word at a time over the canonical 2048-word image.
 * Entry: Z names one of the two entries, FB bank 05, EB E6 (BB coherent),
 * any raw A/L/Q and erasable contents. CM_TVC_MASSPROP_OK leaves the image
 * as the source's final TC Q leaves it: TC Q executes the TC word held in
 * Q, so Z = the caller's return and Q = 3.
 * CM_TVC_MASSPROP_NO_EXIT: overflow detours re-entered a loop head with an
 * unchanged loop state, so the source never returns; the image then holds
 * the state at the repeated loop head. Out-of-scope entry leaves the image
 * unchanged. CM_TVC_MASSPROP_FAULT (an INDEX value outside +-9 or a fixed
 * operand outside 3435/3462-3520) cannot occur from any admitted entry: the
 * loop counters are set only by the source's own CAF/CCS/DIM/TS words. */
enum cm_tvc_massprop_status {
    CM_TVC_MASSPROP_OK = 0,
    CM_TVC_MASSPROP_OUT_OF_SCOPE = 1,
    CM_TVC_MASSPROP_NO_EXIT = 2,
    CM_TVC_MASSPROP_FAULT = 3
};
enum cm_tvc_massprop_status cm_tvc_massprop(agc_image *);
/* Original bank-05 words the source reads as operands: 3435 (reached as an
 * AD operand when TS TEMP333 overflows) and 3462-3520. */
uint8_t cm_tvc_massprop_fixed(uint16_t address, agc_word *out);
#endif
