/* SPDX-License-Identifier: MIT */
#ifndef LM_INTERPRETER_CHANGE_CASES_H
#define LM_INTERPRETER_CHANGE_CASES_H
#include "lm_interpreter_change.h"
#define LM_INTERPRETER_CHANGE_CASES 16384u
uint16_t lm_interpreter_change_channel11(uint16_t);
uint16_t lm_interpreter_change_channel14(uint16_t);
void lm_interpreter_change_input(apollo_overlay_state *,p66_bias_io *,uint16_t);
#endif
