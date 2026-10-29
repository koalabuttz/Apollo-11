/* SPDX-License-Identifier: MIT */
#ifndef CM_DYNDISP_H
#define CM_DYNDISP_H
#include "cm_native_image.h"
/* Explicit compiled original DYNDISP/P40CNV85/S41.1 caller groups. Each
 * caller owns exclusion and uses the sole canonical image. Arithmetic
 * CDUTRIG and *SMNB* and MXV stages are separate source-qualified modules.
 * Initial qualification requires NEWJOB +0; phase values are native routes,
 * not AGC opcodes. Source instruction addresses are checked before mutation. */
uint8_t cm_dyndisp_stage(apollo_overlay_state *,cm_native_io *,uint8_t phase);
/* Linked compiled numerical route0..26, source chronology; completes each
 * callee before returning. A failed internal callee is a native fault, not a
 * rollback boundary. Scheduler may yield only at source EDOP0/DANZIG. No
 * linked route frame may remain across a target overlay replacement. */
uint8_t cm_dyndisp_step(apollo_overlay_state *,cm_native_io *,uint8_t route);
#endif
