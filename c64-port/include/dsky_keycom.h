/* SPDX-License-Identifier: MIT */
#ifndef DSKY_KEYCOM_H
#define DSKY_KEYCOM_H
#include "p66_bias.h"
#include "executive.h"
/* Original KEYCOM3303 through actual RESUME5270 entry, or BAILOUT5716.
 * Bank04, any retained EB0..7, SUPER0..0160 (multiples020), A=channel15
 * five-bit keycode. Caller excludes producers. Queues actual CHARIN2077/
 * 060101 at PRIO30 using native NOVAC; CHARIN execution/RESUME are separate.
 * Complete canonical image; no hardware keyboard polling or opcode decoder. */
enum exec_status dsky_keycom(apollo_overlay_state *,p66_bias_io *);
#endif
