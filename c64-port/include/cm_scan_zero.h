/* SPDX-License-Identifier: MIT */
#ifndef CM_SCAN_ZERO_H
#define CM_SCAN_ZERO_H
#include "cm_basic_finish.h"
#include "cm_basic_sleep.h"
#include "cm_priority_complete.h"
/* Additive complete +0 candidate handling at the real END, JOBSLEEP and
 * PRIOCHNG entries. Existing ordinary wrappers stay byte-identical. At an
 * actual CCSHOLE transfer, next reports that source tuple and its inline Q;
 * the separately compiled alarm body remains a dispatcher continuation.
 * Ordinary canonical flight words, coherent banks, positive active priority,
 * ordinary compiled caller Q0100..07777 and legal selected contexts required.
 * Caller excludes producers; no source instruction decoder or callback. */
enum exec_status cm_scan_zero(apollo_overlay_state *,cm_native_io *,exec_transfer *);
#endif
