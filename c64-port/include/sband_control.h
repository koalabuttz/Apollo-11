/* SPDX-License-Identifier: MIT */
#ifndef SBAND_CONTROL_H
#define SBAND_CONTROL_H
#include "fixed_data.h"
#include "p66_bias.h"
#include "agc_word.h"
#include <stdbool.h>
enum sband_control_entry {SBAND_CONTROL_START,SBAND_CONTROL_INTERPRETER,SBAND_CONTROL_DISPLAY,SBAND_CONTROL_AFTER_PRIORITY,SBAND_CONTROL_TERMINATE,SBAND_CONTROL_PROCEED,SBAND_CONTROL_RECYCLE,SBAND_CONTROL_BLANK,SBAND_CONTROL_RESTART,SBAND_CONTROL_REENTER};
/* Original explicit basic antenna callers and INTPRET front. Entry/return
 * metadata is compile-time source information, never an opcode decoder.
 * Correct original bank42 requiresFB064000 and SUPER bit100 under all four
 * bank-selecting forms. Coherent EB/BB, ZERO0, ordinary RAM, rawA/L/Q16 and
 * exclusive image ownership. SOURCE_B5OFF is composed through ENDOFJOB;
 * all other orbital/display/Executive bodies remain genuine external calls.
 * Unsupported context is unchanged. IRQ/SUPER are external latches. */
bool sband_control(apollo_overlay_state*,p66_bias_io*,enum apollo_fixed_mission,enum sband_control_entry);
#endif
