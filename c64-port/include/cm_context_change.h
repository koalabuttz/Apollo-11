/* SPDX-License-Identifier: MIT */
#ifndef CM_CONTEXT_CHANGE_H
#define CM_CONTEXT_CHANGE_H
#include "cm_native_image.h"
#include "cm_dispatch.h"
/* Actual CHANG105164/CHANG205170 through selected basic DTCB or INTRSM
 * NEWOPS06037. Outgoing basic Q or completed interpreter LOC is an ordinary
 * compiled address; selected saved contexts are genuine source representations.
 * Producer exclusion is caller-owned; no callback/body is executed here. */
enum exec_status cm_context_change(apollo_overlay_state *,cm_native_io *,uint8_t interpretive,exec_transfer *);
#endif
