/* SPDX-License-Identifier: MIT */
#ifndef DSKY_KEY_IRQ_CASES_H
#define DSKY_KEY_IRQ_CASES_H
#include "dsky_key_irq.h"
#define DSKY_KEY_IRQ_CASE_COUNT 4096
#define DSKY_KEY_IRQ_TARGET_COUNT 64
uint16_t dsky_key_irq_channel15(uint16_t);
void dsky_key_irq_prepare(apollo_overlay_state *,uint16_t);
#endif
