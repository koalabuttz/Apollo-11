/* SPDX-License-Identifier: MIT */
#ifndef CM_RCS_MATRIX_H
#define CM_RCS_MATRIX_H
#include "overlay_runtime.h"
#include "agc_word.h"
enum cm_rcs_matrix_status {CM_RCS_MATRIX_OK=0,CM_RCS_MATRIX_OUT_OF_SCOPE=1};
/* Original AMBGUPDT at22,3447, EB=E6, coherentBB, REGZERO0. Explicit
 * source SPSIN1/2/SPCOS1/2 aliases compute sourceAMGB andCAPSI. Stops at
 * actual ENDOFJOB entry05217; caller owns native Executive completion.
 * All ordinary flags/MODE/CDU and raw A/L supported; retain IRQ externally.
 * There is no Executive check within this basic job; exclude producers. */
enum cm_rcs_matrix_status cm_rcs_matrix(apollo_overlay_state *);
uint8_t cm_rcs_matrix_constant_read(uint16_t,uint16_t *);
#endif
