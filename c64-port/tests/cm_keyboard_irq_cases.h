/* SPDX-License-Identifier: MIT */
#ifndef CM_KEYBOARD_IRQ_CASES_H
#define CM_KEYBOARD_IRQ_CASES_H
#include "cm_keyboard_irq.h"
#define CM_KEYBOARD_IRQ_CASES 12288u
#define CM_KEYBOARD_IRQ_TARGET_COUNT 64u
void cm_keyboard_irq_input(apollo_overlay_state *,cm_native_io *,uint16_t *,uint16_t);
uint16_t cm_keyboard_irq_channel15(uint16_t);
enum exec_status cm_keyboard_irq_run(apollo_overlay_state *,cm_native_io *,uint16_t *,uint16_t);
#endif
