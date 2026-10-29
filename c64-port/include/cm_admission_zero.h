/* Additive source +0-core unavailable scan; old helpers remain unchanged.
 * SPDX-License-Identifier: MIT */
#ifndef CM_ADMISSION_ZERO_H
#define CM_ADMISSION_ZERO_H
#include "cm_native_image.h"
#include "cm_admission_complete.h"
/* Genuine canonical admission through Q+2/SPVAC return or BAILOUT07755.
 * The alarm body is a separate compiled continuation. Resource exhaustion
 * preserves reservations and source scratch/register effects. SPVAC entries
 * require caller IRQ exclusion and source NEWPRIO setup. job is the actual
 * compiled inline pair for NOVAC/FINDVAC; SPVAC uses source raw A/L instead. */
enum exec_status cm_admission_zero(apollo_overlay_state *,cm_native_io *,enum cm_admission_entry,exec_address);
#endif
