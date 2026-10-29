/* SPDX-License-Identifier: MIT */
#ifndef CM_IRQ_SEQUENCE_H
#define CM_IRQ_SEQUENCE_H
#include "cm_irq.h"
#include "cm_dispatch_cases.h"
typedef enum exec_status (*cm_irq_publish)(cm_irq_queue *,const cm_irq_request *,void *);
unsigned cm_irq_sequence_run(cm_executive *,uint8_t,const exec_exclusion *,cm_irq_publish,void *,cm_dispatch_observer,void *,uint8_t,uint8_t);
#endif
