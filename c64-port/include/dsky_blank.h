/* SPDX-License-Identifier: MIT */
#ifndef DSKY_BLANK_H
#define DSKY_BLANK_H
#include "dsky_pinball.h"
#include "cm_native_image.h"
enum dsky_blank_entry {DSKY_BLNKSUB1,DSKY_BLANK_RETURN};
/* Actual LM BLNKSUB1 40,3505 / CM40,3412, all retained EB, SUPER100.
 * NVTEMP0..7 selects source component blanking; selected isolated relay
 * words must be nonzero. Paired words accept either zero. Stops at actual
 * SUPDXCHZ+1 (LM5166/CM5230), preserving opaque saved BUF return words.
 * Return entry requires ordinary sign-extended bank A (bits076167 only),
 * ordinary L0100..07777, source aligned BB/EB. Executes WRITE SUPER,
 * TS BBANK, TC L through its actual selected address, no callback/poll.
 * Unsupported inputs preserve image and IO. Caller excludes producers. */
enum p66_bias_status dsky_blank(apollo_overlay_state *,p66_bias_io *,enum dsky_blank_entry);
enum exec_status cm_blank(apollo_overlay_state *,cm_native_io *,enum dsky_blank_entry);
#endif
