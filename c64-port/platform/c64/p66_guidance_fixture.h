/* SPDX-License-Identifier: MIT */
#ifndef P66_GUIDANCE_FIXTURE_H
#define P66_GUIDANCE_FIXTURE_H
#include "p66_native_guidance.h"
typedef struct {uint8_t number,mixed,stage,failures;} p66_guidance_fixture;
#define P66_GUIDANCE_FIXTURE ((p66_guidance_fixture *)(uintptr_t)0xc000)
#endif
