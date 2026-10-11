/* SPDX-License-Identifier: MIT */
#ifndef CM_DECIMAL_PAIR_H
#define CM_DECIMAL_PAIR_H
#include "cm_native_image.h"
/* Actual signed nonrounding DSPDC2NR40,3173 through ordinary callerQ.
 * Sign-agreed DP MPAC0/1, ordinary third limb; DSPCOUNT14/11/9/6/4/1,
 * nonzero relay words, bank40/SUPER100/any EB. Caller owns exclusion. */
enum exec_status cm_decimal_pair(apollo_overlay_state *,cm_native_io *);
#endif
