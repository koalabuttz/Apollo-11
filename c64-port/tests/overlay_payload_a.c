/* SPDX-License-Identifier: MIT */
#include "overlay_runtime.h"
static uint16_t transient;
/* Test-only native body; this does not purport to translate an AGC routine. */
uint16_t apollo_overlay_main(apollo_overlay_state *state,uint16_t selector) {
    uint16_t old=transient;
    if((*(volatile uint8_t *)1 & 7u)!=6u) return 0xdead;
    ++transient;
    state->words[0300]=(uint16_t)(state->words[0300]+selector);
    state->words[0301]=old;
    return (uint16_t)(0xa100u+old);
}
