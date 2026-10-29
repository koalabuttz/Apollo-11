/* SPDX-License-Identifier: MIT */
#ifndef CM_MYSUBS_H
#define CM_MYSUBS_H
#include "overlay_runtime.h"
#include "agc_multiword.h"
enum cm_mysubs_status { CM_MYSUBS_OK=0,CM_MYSUBS_OUT_OF_SCOPE=1 };
/* Original CM basic SMALLMP and DPADD, bank21/E6 (FB042000/EB03000/BB042006).
 * SMALLMP entry02042, DPADD02054. Raw16-bit A/L and ordinary Q<=07777;
 * KMPAC03110/03111 and KMPTEMP03112. Stop before TCQ02053/02073.
 * All raw signs/overflow paths retained, every other image word preserved.
 * No Executive handoff; caller retains canonical ownership/excludes producers. */
enum cm_mysubs_status cm_smallmp(apollo_overlay_state *);
enum cm_mysubs_status cm_dpadd(apollo_overlay_state *);
/* Source basic TCQ and fixed-code saved TC return; Q02000..07777. */
enum cm_mysubs_status cm_mysubs_return(apollo_overlay_state *);
#endif
