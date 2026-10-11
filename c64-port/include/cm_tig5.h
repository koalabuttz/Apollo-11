/* SPDX-License-Identifier: MIT */
#ifndef CM_TIG5_H
#define CM_TIG5_H
#include "cm_native_image.h"
enum cm_tig5_exit { CM_TIG5_WAITLIST=0,CM_TIG5_FINDVAC=1,CM_TIG5_TASKOVER=2,CM_TIG5_INVALID=3 };
/* Actual TIG-5 source groups at24,2574 /24,2600 /24,2611. Returns before
 * selected resident WAITLIST/FINDVAC/TASKOVER services, no live overlay frames.
 * Source bank24/E7 and separate valid IRQ/SUPERBNK latches required. */
enum cm_tig5_exit cm_tig5_phase(apollo_overlay_state *,cm_native_io *,uint8_t);
#endif
