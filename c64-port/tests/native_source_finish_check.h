/* SPDX-License-Identifier: MIT */
#ifndef NATIVE_SOURCE_FINISH_CHECK_H
#define NATIVE_SOURCE_FINISH_CHECK_H
#include "lm_finish_cases.h"
#include "irq_delivery.h"
uint16_t native_source_finish_check(uint8_t,uint8_t,apollo_overlay_state *,apollo_overlay_state *);
uint16_t native_source_finish_guards(void);
#endif
