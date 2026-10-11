/* SPDX-License-Identifier: MIT */
#ifndef P66_ELAPSED_RETURN_CONTROL_H
#define P66_ELAPSED_RETURN_CONTROL_H
#include "overlay_runtime.h"
#include "agc_multiword.h"
/* Source DSU -> ENDVXV's initial DAS preserves Q unless it calls OVERFLOW.
 * Observe only that initial double addition, before the numerical operation;
 * later overflow realignment does not change this source call distinction. */
agc_word p66_elapsed_return_q(agc_word incoming_q,agc_dp mpac,agc_dp piptime);
/* Commit the return debris at the compiled P66 elapsed DSU's literal DANZIG
 * boundary. Caller retains the source input operands until this normal C
 * return. No Executive poll, arithmetic output rewriting or opcode decoding. */
uint16_t p66_elapsed_return_control(apollo_overlay_state *,agc_word incoming_q,
 agc_dp before,agc_dp piptime);
#endif
