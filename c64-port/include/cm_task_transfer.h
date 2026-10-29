/* SPDX-License-Identifier: MIT */
#ifndef CM_TASK_TRANSFER_H
#define CM_TASK_TRANSFER_H
#include "cm_native_image.h"
enum cm_task_transfer_entry { CM_TASK_DISPATCH=0,CM_TASK_FINISH=1 };
enum cm_task_transfer_exit { CM_TASK_SELECTED=0,CM_TASK_RESUME=1,CM_TASK_INVALID=2 };
/* Original T3RUPT2 through actual selected task DTCB transfer, or TASKOVER
 * through that transfer / the actual hardware RESUME entry at05341. Producer
 * exclusion and source-valid queue/interrupt-save words are obligations. No
 * selected body is skipped; caller resolves source Z/BB/SUPERBNK to native
 * compiled mappings after this C frame returns. */
enum cm_task_transfer_exit cm_task_transfer(apollo_overlay_state *,cm_native_io *,enum cm_task_transfer_entry);
#endif
