/* SPDX-License-Identifier: MIT */
#ifndef UNIT_PHASED_CASES_H
#define UNIT_PHASED_CASES_H
#include "unit_cases.h"
#include "guidance_unit_phased.h"
#define UPH_NORMAL_CASES (UNIT_CASES/3u)
#define UPH_CASES (UPH_NORMAL_CASES+256u)
void uph_input(unit_case *,uint32_t);
void uph_from_words(unit_case *,const uint16_t [UNIT_WORDS]);
unsigned uph_apply(unit_case *,uint8_t);
#endif
