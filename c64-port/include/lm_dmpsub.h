/* SPDX-License-Identifier: MIT */
#ifndef LM_DMPSUB_H
#define LM_DMPSUB_H
#include "overlay_runtime.h"
/* Luminary INTERPRETER's fixed-fixed basic subroutines DMP (07103) and
 * DMPSUB (07107), run in place by a compiled caller's TC (spec
 * 'native_calls'), as the original runs them: MPAC (TP) = MPAC times the
 * DP word at C(ADDRWD). DMP first sets ADDRWD from the ADRES word after its
 * TC. The caller's TC has set Q, and its run goes on past the call; Q is
 * left as DMPSUB's TC Q leaves it (3). RTB_OUT_OF_SCOPE (rtb_angles.h) when
 * the operand is not an erasable word; the image is then unchanged. */
uint8_t lm_dmpsub(apollo_overlay_state *);
uint8_t lm_dmp(apollo_overlay_state *, uint16_t adres);
#endif
