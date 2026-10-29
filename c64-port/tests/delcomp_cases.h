/* SPDX-License-Identifier: MIT */
#ifndef DELCOMP_CASES_H
#define DELCOMP_CASES_H
#include "delcomp.h"
#define DELCOMP_CASES_PER_MISSION 5120u
#define DELCOMP_CASE_COUNT 10240u
void delcomp_prepare(apollo_overlay_state *,gimbal_io *,uint16_t);
enum apollo_fixed_mission delcomp_mission(uint16_t);
#endif
