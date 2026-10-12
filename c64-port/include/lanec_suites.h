/* SPDX-License-Identifier: MIT */
#ifndef LANEC_SUITES_H
#define LANEC_SUITES_H
#include "lanec_case.h"
extern const lanec_suite lanec_suite_ev_front;
extern const lanec_suite lanec_suite_pin_gaps;
extern const lanec_suite lanec_suite_di_dsp;
extern const lanec_suite lanec_suite_di_mark;
extern const lanec_suite lanec_suite_di_play;
extern const lanec_suite lanec_suite_di_sleep;
extern const lanec_suite lanec_suite_di_idle;
extern const lanec_suite lanec_suite_di_nvdsp;
extern const lanec_suite lanec_suite_ev_short;
extern const lanec_suite lanec_suite_ev_align;
extern const lanec_suite lanec_suite_ev_radar;
extern const lanec_suite lanec_suite_ev_dap;
extern const lanec_suite lanec_suite_ev_time;
extern const lanec_suite lanec_suite_ev_track;
const lanec_suite *lanec_find_suite(const char *name);
#endif
