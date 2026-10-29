/* SPDX-License-Identifier: MIT */
#ifndef CM_BASIC_PRIORITY_H
#define CM_BASIC_PRIORITY_H
#include "cm_native_image.h"
#include "cm_dispatch.h"
/* Original Comanche PRIOCHNG5210 through actual basic DTCB dispatch.
 * Canonical physical words plus external SUPER/IRQ are the sole authority.
 * Caller owns restoring producer exclusion. Core0 active, upper priority
 * 01000..037000; candidates positive/dormant/-0 (CCSHOLE +0 excluded).
 * Returns the actual selected source pair, including suspended caller state.
 * Interpreter selection remains an explicit unsupported native boundary. */
enum exec_status cm_basic_priority_change(apollo_overlay_state *,cm_native_io *,exec_address *);
#endif
