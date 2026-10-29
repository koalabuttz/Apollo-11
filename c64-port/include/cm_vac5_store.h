/* SPDX-License-Identifier: MIT */
#ifndef CM_VAC5_STORE_H
#define CM_VAC5_STORE_H
#include "cm_native_image.h"
/* Actual VAC5STOR20,2000 including source GENTRAN and scratch aliases, stopping
 * at genuine SWRETURN4675 (Q2064). The canonical VAC5 snapshot is diagnostic
 * source flight data, not an additional authoritative image. Producer excluded. */
enum exec_status cm_vac5_store(apollo_overlay_state *,cm_native_io *);
#endif
