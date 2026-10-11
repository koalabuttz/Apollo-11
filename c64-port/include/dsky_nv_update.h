/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_UPDATE_H
#define DSKY_NV_UPDATE_H
#include "dsky_nv_admission.h"
enum dsky_nv_update_entry {DSKY_NV_VERB_STORE,DSKY_NV_VERB_UPDATE,DSKY_NV_NOUN_STORE,DSKY_NV_NOUN_UPDATE};
/* Actual bank41 UPDATVB/-1 and UPDATNN/-1 through original fixed caller.
 * Performs real immutable noun-table read, source SETNCADR and actual
 * mission DSPDECVN/GOVNUPDT/POSTJUMP return. A or existing VERBREG/NOUNREG
 * holds0..99; ordinary Q02000..07777, nonzero selected relay word, coherent
 * incoming banks/SUPER100 and ordinary RAM. Canonical image/IO authoritative;
 * caller excludes producers. Unsupported preserves image/IO; no poll or
 * formatter substitute, no opcode decoding. */
enum p66_bias_status dsky_nv_update(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_nv_update_entry);
#endif
