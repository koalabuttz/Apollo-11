/* SPDX-License-Identifier: MIT */
#ifndef CM_SINGLE_RETURN_H
#define CM_SINGLE_RETURN_H
#include "cm_quictrig.h"
/* Complete the actual basic TC Q and its saved return instruction in Q.
 * Entry is source-produced SPSIN/SPCOS PC05130/05133 with a fixed-code return
 * address02000..07777 in Q. Source A/L and bank context are retained; PC becomes
 * savedQ and Q becomes3. This explicit native continuation is not an opcode
 * decoder, and admits no Executive check. Caller excludes all producers and
 * retains the same canonical owner through the basic caller return. */
enum cm_trig_status cm_single_return(apollo_overlay_state *);
#endif
