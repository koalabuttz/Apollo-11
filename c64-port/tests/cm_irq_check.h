/* SPDX-License-Identifier: MIT */
#ifndef CM_IRQ_CHECK_H
#define CM_IRQ_CHECK_H
#include "cm_irq_sequence.h"
unsigned cm_irq_check_sequence(cm_executive *,uint8_t,const exec_exclusion *,cm_irq_publish,void *,uint8_t,uint8_t);
#endif
