/* SPDX-License-Identifier: MIT */
#ifndef CM_PHASE_CLEANUP_H
#define CM_PHASE_CLEANUP_H
#include "cm_native_image.h"
/* Actual MR.KLEAN05,2501, P00KLEAN05,2505, V37KLEAN05,2513 through genuine
 * ordinary TCQ caller return. Physical phase pairs, raw registers and IRQ
 * effects commit directly to sole canonical image. Producers excluded. */
enum exec_status cm_phase_cleanup(apollo_overlay_state *,cm_native_io *);
#endif
