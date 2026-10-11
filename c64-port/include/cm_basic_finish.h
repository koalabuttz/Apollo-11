/* SPDX-License-Identifier: MIT */
#ifndef CM_BASIC_FINISH_H
#define CM_BASIC_FINISH_H
#include "cm_native_image.h"
#include "cm_dispatch.h"
/* Actual ENDOFJOB05217 through selected basic DTCB, interpreted INTRSM before
 * NEWOPS06037, or idle ADVAN03231 after channel11 activity output. Canonical
 * image + explicit I/O are authoritative; no callback/decoder/persistent view.
 * Active positive core0 with an admitted VAC base or lowbits0..FAKEPRET;
 * candidates ordinary positive/dormant/-0 (CCSHOLE +0 not yet supported).
 * Raw Q is retained exactly when the scan never calls EJ1. */
enum exec_status cm_basic_finish(apollo_overlay_state *,cm_native_io *,exec_transfer *);
#endif
