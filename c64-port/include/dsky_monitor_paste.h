/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MONITOR_PASTE_H
#define DSKY_MONITOR_PASTE_H
#include "dsky_nv_admission.h"
enum dsky_monitor_paste_stage {DSKY_MONITOR_PASTE_BEGIN,DSKY_MONITOR_PASTE_UPDATED,DSKY_MONITOR_PASTE_BLANKED};
/* Original PASTEVB/PASTEOPT control stages: known ENTRET savedTC or direct
 * paste entry ->UPDATVB-1, SWRETURN ->BLANKSUB, returned blank ->ENDOFJOB.
 * Selected verb0..99, updated helperQ0100..7775, ordinary image/rawA/L, coherent bank41/allEB/selecting
 * SUPER/IRQ; saved BANKCALL frame actual source pair. Caller owns actual
 * update/blank/END bodies; no saved arbitrary instruction is decoded.
 * Producer exclusion/distinct image/IO; unsupported complete no-op. */
enum p66_bias_status dsky_monitor_paste(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_monitor_paste_stage);
#endif
