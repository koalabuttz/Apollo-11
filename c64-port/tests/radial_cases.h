/* SPDX-License-Identifier: MIT */
#ifndef RADIAL_CASES_H
#define RADIAL_CASES_H
#include "velocity_cases.h"
#include "guidance_radial.h"
#define RA_WORDS 134u
#define RA_CASES 2467u
#define RA_TARGET 163u
typedef struct {velocity_case previous;p66_norm_scratch norm;p66_radial_state output;agc_dp land;} radial_case;
void ra_prepare(radial_case *,uint16_t);
void ra_next(radial_case *,uint16_t,uint8_t);
uint8_t ra_integrated(uint16_t);
uint8_t ra_steps(uint16_t);
void ra_words(const radial_case *,uint16_t [RA_WORDS]);
void ra_from_words(radial_case *,const uint16_t [RA_WORDS]);
#ifndef RADIAL_INPUT_ONLY
void ra_prefix(radial_case *,uint16_t);
enum p66_radial_status ra_apply(radial_case *);
unsigned ra_check(radial_case *);
#endif
#endif
