/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_STORE_H
#define GUIDANCE_STORE_H
#include "guidance_push.h"
enum p66_store_status { P66_STORE_OK=0, P66_STORE_OUT_OF_SCOPE=1 };
/* STORE 30D in DP MODE=+0 only. Write MPAC 0/+1 to VAC-relative 30/31,
 * retain MPAC and PUSHLOC, clear A, retain the former word 31 in raw L.
 * Caller owns distinct objects and excludes producers. Stops at DANZIG
 * before DDV/PDVL. Unsupported MODE/FIXLOC returns without mutation;
 * this is a native scope check, not an original alarm. PUSHLOC is unused. */
enum p66_store_status p66_store_elapsed(p66_preparation *state,
                                        p66_pushdown *push);
#endif
