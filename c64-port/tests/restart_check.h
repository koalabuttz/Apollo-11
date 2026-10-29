/* SPDX-License-Identifier: MIT */
#ifndef RESTART_CHECK_H
#define RESTART_CHECK_H
#include "restart_cases.h"
#include "irq_delivery.h"
unsigned rs_check(restart_case *, const exec_exclusion *);
unsigned rs_demo(restart_case *, const exec_exclusion *);
#endif
