/* SPDX-License-Identifier: MIT */
#ifndef LM_DELAY_IMAGE_FIXTURE_H
#define LM_DELAY_IMAGE_FIXTURE_H
#include "lm_delay_image.h"
typedef struct {p66_bias_io io;uint16_t entry,status,failures;exec_address job;} lm_delay_image_fixture;
typedef char fixture_fits[(sizeof(lm_delay_image_fixture)<=0x40u)?1:-1];
#define LM_DELAY_IMAGE_FIXTURE ((lm_delay_image_fixture *)(uintptr_t)0xc000)
#endif
