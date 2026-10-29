/* SPDX-License-Identifier: MIT */
#ifndef CM_NV85_H
#define CM_NV85_H
#include "cm_native_image.h"
/* Bounded normal nonflashing V06N85 compiled display caller stages.
 * Canonical image/producer exclusion are caller-owned. */
uint8_t cm_nv85_goplay(apollo_overlay_state *,cm_native_io *);
uint8_t cm_nv85_enter(apollo_overlay_state *,cm_native_io *);
uint8_t cm_nv85_verb(apollo_overlay_state *,cm_native_io *);
uint8_t cm_nv85_noun(apollo_overlay_state *,cm_native_io *);
uint8_t cm_nv85_rows(apollo_overlay_state *,cm_native_io *);
uint8_t cm_nv85_select(apollo_overlay_state *,cm_native_io *);
uint8_t cm_nv85_decimal(apollo_overlay_state *,cm_native_io *);
uint8_t cm_nv85_next(apollo_overlay_state *,cm_native_io *);
uint8_t cm_nv85_finish(apollo_overlay_state *,cm_native_io *);
/* Linked normal-source route0..25, ending actual idle ADVAN after END.
 * No private frame may remain across an overlay replacement; source-selected
 * unexpected wake/priority/END continuations are native fault boundaries. */
uint8_t cm_nv85_step(apollo_overlay_state *,cm_native_io *,uint8_t route);
#endif
