/* SPDX-License-Identifier: MIT */
#ifndef CM_WAKE_COMPLETE_H
#define CM_WAKE_COMPLETE_H
#include "cm_native_image.h"
#include "cm_dispatch.h"
/* Original JOBWAKE05201, including genuine dormant core0 while idle.
 * Returns ordinary caller or the real source exceptional transfer: +0
 * priority executes COREINC as TC014; core0 matched with NEWJOB+0 reaches
 * CCSHOLE05747. The register-code body at014 is outside this source file and
 * is never decoded/executed here. next is the exact actual transfer.
 * Source A normal CADR, Q0100..07777, coherent banks/ZERO, typed NEWJOB
 * -0/0/12..72; matched sleeping bank uses bits0167. Caller excludes producers.
 * Unsupported input preserves canonical image, all13 latches and transfer. */
enum exec_status cm_wake_complete(apollo_overlay_state *,cm_native_io *,exec_transfer *);
#endif
