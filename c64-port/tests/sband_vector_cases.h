/* SPDX-License-Identifier: MIT */
#ifndef SBAND_VECTOR_CASES_H
#define SBAND_VECTOR_CASES_H
#include "sband_vector.h"
void sband_vector_input(apollo_overlay_state*,agc_word*,agc_word*,uint32_t,enum apollo_fixed_mission);
void sband_vector_fixed_input(agc_word factor[6],uint32_t n);
#endif
