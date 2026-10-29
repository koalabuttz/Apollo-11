/* SPDX-License-Identifier: MIT */
#ifndef LM_RADAR_LEADIN_FIXTURE_H
#define LM_RADAR_LEADIN_FIXTURE_H
#include "lm_radar_leadin.h"
typedef struct {p66_bias_io io;uint16_t status,failures;} lm_radar_leadin_fixture;
typedef char fixture_fits[(sizeof(lm_radar_leadin_fixture)<=0x40u)?1:-1];
#define LM_RADAR_LEADIN_FIXTURE ((lm_radar_leadin_fixture *)(uintptr_t)0xc000)
#endif
