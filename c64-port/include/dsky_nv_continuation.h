/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_CONTINUATION_H
#define DSKY_NV_CONTINUATION_H
#include "dsky_nv_return.h"
enum dsky_nv_continuation_entry {DSKY_NV_COMMON,DSKY_NV_SWITCH,DSKY_NV_KILL,DSKY_NV_ENTSET,DSKY_NV_ENTSET_CLEAR,DSKY_NV_NOUN_RETURN,DSKY_NV_AFTER_VERB,DSKY_NV_AFTER_NOUN,DSKY_NV_PAS0};
/* Actual source internal continuations: NVSBCOM/NVSUBCOM through NVSUB1;
 * KILMONON ordinary fixed caller return; ENTSET/ENTSET-2/NVSUB2+4 through
 * NVSUBEND and restored caller; NVSUB2+011 through next UPDATNN entry;
 * NVSUB2+013/ENDNVSB1 through ENTPAS0 entry. No selected formatter/ENTPAS0
 * body execution. Caller-owned nonnull distinct image/IO, coherent banks,
 * ordinary RAM, legal IO; COMMON A0or1/Q02000..07776; saved return ordinary.
 * Unsupported preserves complete image/IO. Producers excluded, no poll. */
enum p66_bias_status dsky_nv_continue(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_nv_continuation_entry);
#endif
