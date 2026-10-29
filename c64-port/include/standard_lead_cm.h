/* SPDX-License-Identifier: MIT */
#ifndef STANDARD_LEAD_CM_H
#define STANDARD_LEAD_CM_H
#include "cm_native_image.h"
enum standard_lead_cm_entry {STANDARD_LEAD_COMPVER,STANDARD_LEAD_GTSCPSS1,STANDARD_LEAD_REDO};
/* Original TESTLEAD entry controls. COMPVER/GTSCPSS1 stop at their actual
 * external native routine entries. REDO executes NEWMODEX/SETUPDSP and admits
 * the real DSPMMJOB 2CADR; returns before IMUTEST, or at BAILOUT on no core.
 * The seven-core native Executive admission contract applies; priority +0
 * (CCSHOLE/fault context) is rejected before mutation. All source
 * scratch/alias stores live in this sole image; producers excluded. */
enum exec_status standard_lead_cm_test(apollo_overlay_state *,cm_native_io *,enum standard_lead_cm_entry);
#endif
