/* SPDX-License-Identifier: MIT */
#ifndef HORIZONTAL_PHASED_CASES_H
#define HORIZONTAL_PHASED_CASES_H
#include "horizontal_cases.h"
#include "guidance_horizontal_phased.h"
#define HHP_WORDS (HH_WORDS+12u)
typedef struct {horizontal_case source;agc_word control[12];uint8_t next;unsigned unit_status;} horizontal_phased_case;
void hhp_input(horizontal_phased_case *,uint16_t);
void hhp_words(const horizontal_phased_case *,uint16_t [HHP_WORDS]);
void hhp_from_words(horizontal_phased_case *,const uint16_t [HHP_WORDS]);
unsigned hhp_step(horizontal_phased_case *,uint8_t *returned_stage);
#endif
