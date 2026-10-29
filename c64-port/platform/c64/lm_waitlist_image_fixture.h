/* SPDX-License-Identifier: MIT */
#ifndef LM_WAITLIST_IMAGE_FIXTURE_H
#define LM_WAITLIST_IMAGE_FIXTURE_H
#include "lm_waitlist_image.h"
typedef struct {p66_bias_io io;uint16_t entry,status,failures;exec_address job;} lm_waitlist_image_fixture;
typedef char fixture_fits[(sizeof(lm_waitlist_image_fixture)<=0x40u)?1:-1];
#define LM_WAITLIST_IMAGE_FIXTURE ((lm_waitlist_image_fixture *)(uintptr_t)0xc000)
#endif
