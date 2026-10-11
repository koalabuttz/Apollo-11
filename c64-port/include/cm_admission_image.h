/* SPDX-License-Identifier: MIT */
#ifndef CM_ADMISSION_IMAGE_H
#define CM_ADMISSION_IMAGE_H
#include "cm_native_image.h"
/* Successful original NOVAC/FINDVAC through actual Q+2 return. Image A is
 * ordinary source priority, Q points to source inline2CADR, job pair is compiled
 * metadata. Source-valid available core/VAC and producer exclusion required.
 * Resource alarms retain the separate bounded admission service's scope. */
enum exec_status cm_admission_image(apollo_overlay_state *,cm_native_io *,exec_address,uint8_t needs_vac);
#endif
