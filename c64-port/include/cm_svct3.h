/* SPDX-License-Identifier: MIT */
#ifndef CM_SVCT3_H
#define CM_SVCT3_H
#include "cm_native_image.h"
enum cm_svct3_exit { CM_SVCT3_TASKOVER=0,CM_SVCT3_NOVAC=1,CM_SVCT3_FIXDELAY=2,CM_SVCT3_INVALID=3 };
/* Original housekeeping task: 0=SVCT3, 1=NOVAC return, 2=FIXDELAY return.
 * The compiled caller supplies FIXDELAY literal 500 and NBDONLY pair
 * (03542,014063). Each phase returns before the resident invokes a service. */
enum cm_svct3_exit cm_svct3_phase(apollo_overlay_state *,cm_native_io *,uint8_t);
#endif
