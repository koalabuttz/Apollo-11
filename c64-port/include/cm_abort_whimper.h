/* SPDX-License-Identifier: MIT */
#ifndef CM_ABORT_WHIMPER_H
#define CM_ABORT_WHIMPER_H
#include "cm_interbank_extended.h"
/* Actual INHINT/WHIMPER7766 or7767, explicit known source TC7773 after RESUME,
 * and actual POSTJUMP to ENEMA12651. BRUPT is retained as canonical data; its
 * known source-selected continuation is compiled, not decoded. ISR-active is
 * external control metadata and clears on genuine source RESUME. */
enum exec_status cm_abort_whimper(apollo_overlay_state *,cm_native_io *,uint8_t *in_isr);
#endif
