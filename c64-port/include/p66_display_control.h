/* SPDX-License-Identifier: MIT */
#ifndef P66_DISPLAY_CONTROL_H
#define P66_DISPLAY_CONTROL_H
#include "p66_bias.h"
enum p66_display_boundary {P66_DISPLAY_REGODSPR=0,P66_DISPLAY_ENDOFJOB=1,P66_DISPLAY_OUT_OF_SCOPE=2};
/* Original final P66 BANKCALL/DISPEXIT+3 vertical display selection.
 * Entry PC03531/LOC03530, BB/BANKSET064007, MODE0, CYR/EDOP/NEWJOB0.
 * WCHPHOLD2 selects the P66 vertical display; when FLUNDISP is set its value
 * is unused. Stops at actual REGODSPR entry02470 (V06N60) or ENDOFJOB05155.
 * Canonical image and IO owned under producer exclusion; invalid boundary
 * rejects both unchanged. No display placeholder or opcode decoding. */
enum p66_display_boundary p66_display_exit(apollo_overlay_state *,p66_bias_io *);
#endif
