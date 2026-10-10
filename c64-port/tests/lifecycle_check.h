/* SPDX-License-Identifier: MIT */
#ifndef LIFECYCLE_CHECK_H
#define LIFECYCLE_CHECK_H
#include "irq_delivery.h"
unsigned lifecycle_check(executive *state,const exec_exclusion *exclusion);
#endif
