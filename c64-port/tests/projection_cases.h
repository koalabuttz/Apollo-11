/* SPDX-License-Identifier: MIT */
#ifndef PROJECTION_CASES_H
#define PROJECTION_CASES_H
#include "trig_cases.h"
#include "guidance_projection.h"
#define PJ_WORDS 134u
#define PJ_CASES 8192u
typedef struct {trig_case previous;p66_rotation_scratch rotation;agc_dp afc;} projection_case;
void pj_input(projection_case *,uint16_t);
void pj_from_words(projection_case *,const uint16_t [PJ_WORDS]);
void pj_words(const projection_case *,uint16_t [PJ_WORDS]);
#ifndef PROJECTION_INPUT_ONLY
unsigned pj_apply(projection_case *,uint8_t);
#endif
#endif
