/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_RETURN_H
#define DSKY_NV_RETURN_H
#include "dsky_nv_admission.h"
/* Actual mission NVSUBEND and SUPDXCHZ through saved fixed caller entry.
 * NVQTEM/NVBNKTEM hold ordinary fixed PC02000..07777 and coherent BBCON
 * (FB/EB/SUPER bits only). Raw A/L are exchanged into those words with
 * original overflow correction. No NEWJOB/IRQ poll. Unsupported preserves
 * complete image/IO. Caller excludes producers; no instruction decoding. */
enum p66_bias_status dsky_nv_return(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission);
#endif
