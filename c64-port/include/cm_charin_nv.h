/* SPDX-License-Identifier: MIT */
#ifndef CM_CHARIN_NV_H
#define CM_CHARIN_NV_H
#include "cm_native_image.h"
/* Actual CM CHARIN40,2000 for VERB021/NOUN037 through NVCOM/2BLANK to
 * ENDOFJOB5217. Canonical bank40/SUPER100 retainedEB0..7, all ordinary
 * relay/NOUT words and rawL. Exact CM ENTRET/caller source stores retained.
 * Caller excludes producers; unsupported leaves image/I/O/event unchanged. */
enum exec_status cm_charin_nv(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11);
#endif
