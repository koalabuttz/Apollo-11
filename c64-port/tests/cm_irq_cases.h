/* SPDX-License-Identifier: MIT */
#ifndef CM_IRQ_CASES_H
#define CM_IRQ_CASES_H
#include "cm_executive_cases.h"
#include "cm_irq.h"
#define CM_IRQ_CASE_COUNT 512
void cm_irq_case_prepare(cm_scenario *,uint16_t);
#ifndef CM_IRQ_INPUT_ONLY
enum exec_status cm_irq_case_run(cm_scenario *,uint16_t,const exec_exclusion *);
unsigned cm_irq_check_cases(const exec_exclusion *);
#endif
#endif
