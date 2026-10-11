/* SPDX-License-Identifier: MIT */
#ifndef LANEC_SUITES_H
#define LANEC_SUITES_H
#include "lanec_case.h"
extern const lanec_suite lanec_suite_ev_front;
extern const lanec_suite lanec_suite_pin_gaps;
const lanec_suite *lanec_find_suite(const char *name);
#endif
