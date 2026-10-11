/* SPDX-License-Identifier: MIT */
#ifndef DSKY_KEYRUPT_H
#define DSKY_KEYRUPT_H
#include "dsky_keycom.h"
/* Original KEYRUPT1 bank04/3274 through KEYCOM to RESUME5270/BAILOUT5716.
 * Saves overflow-corrected interrupted BBANK/Q, samples physical TIME2/TIME1
 * into SAMPTIME, reads channel015 low five bits, queues real CHARIN via NOVAC.
 * Input channel word is explicit device metadata, not duplicate flight state.
 * Caller excludes producers; interrupt lead-in and RESUME are separate entries. */
enum exec_status dsky_keyrupt(apollo_overlay_state *,p66_bias_io *,uint16_t channel15);
#endif
