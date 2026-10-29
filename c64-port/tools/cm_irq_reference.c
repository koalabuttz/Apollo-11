/* SPDX-License-Identifier: GPL-2.0-or-later
 * Queue payloads applied to original CM admission code; no native code linked. */
#include "cm_irq_cases.h"
#undef CM_EXEC_CASE_COUNT
#define CM_EXEC_CASE_COUNT CM_IRQ_CASE_COUNT
#define cm_case_prepare cm_irq_case_prepare
#include "cm_executive_reference.c"
