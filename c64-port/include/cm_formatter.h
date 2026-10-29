/* SPDX-License-Identifier: MIT */
#ifndef CM_FORMATTER_H
#define CM_FORMATTER_H
#include "cm_native_image.h"
enum cm_formatter_status {CM_FORMATTER_OK,CM_FORMATTER_OUT_OF_SCOPE};
/* Actual Comanche DSPDECVN40,3213; A0..99, odd DSPCOUNT1..21. */
enum cm_formatter_status cm_two_digit(apollo_overlay_state *,cm_native_io *);
/* Actual DSPDECWD40,3133 (rounded), DSPDECNR40,3167 (unrounded).
 * Sign-agreed DP MPAC0/1, original TP MPAC+2, DSPCOUNT14/9/4. */
enum cm_formatter_status cm_decimal_display(apollo_overlay_state *,cm_native_io *,uint8_t rounded);
/* Both preserve exact original CM return/scratch word stores and physical
 * caller. Source-qualified arithmetic shared with native decimal kernels;
 * CM addresses are literal source constants, never synthesized opcodes.
 * Bank40/SUPER100/EB0..7; nonzero relay words; ordinary caller Q.
 * One canonical image and restoring caller exclusion are required. */
#endif
