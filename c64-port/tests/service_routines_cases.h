/* SPDX-License-Identifier: MIT */
#ifndef SERVICE_ROUTINES_CASES_H
#define SERVICE_ROUTINES_CASES_H
#include "service_routines.h"
#define SERVICE_ROUTINES_CASES UINT32_C(106496)
unsigned service_routines_kind(uint32_t n);
void service_routines_input(apollo_overlay_state *,p66_bias_io *,uint32_t n,enum service_routines_mission *,enum service_routines_entry *,agc_word fixed[3]);
#endif
