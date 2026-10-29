/* SPDX-License-Identifier: MIT */
#ifndef CM_ADMISSION_FLOOR_H
#define CM_ADMISSION_FLOOR_H
#include "cm_admission_complete.h"
/* Bare source priority+0 becomes a positive packed resource priority after
 * NOVAC FAKEPRET110 / FINDVAC VACbase+1 allocation. All seven core+0 positions
 * retain original unavailable scan. Ordinary Q0100..07775, coherent banks,
 * typed NEWJOB, authentic free/allocated VAC markers; failure reservations
 * and actual BAILOUT handoff retained. SPVAC requires IRQ exclusion.
 * No application job body or alarm/recovery is executed by this service. */
enum exec_status cm_admission_floor(apollo_overlay_state *,cm_native_io *,enum cm_admission_entry,exec_address);
#endif
