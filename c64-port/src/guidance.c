/* SPDX-License-Identifier: MIT */
#include "guidance.h"
void p66_rate_update(p66_rate_state *state) {
    agc_word count = state->count;
    state->count = AGC_POS_ZERO;
    state->registers = agc_multiply(agc_extend(count), state->scale);
    agc_double_add(&state->registers, &state->desired);
}
