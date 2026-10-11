/* SPDX-License-Identifier: MIT */
#ifndef DSKY_LOAD_STORE_H
#define DSKY_LOAD_STORE_H
#include "dsky_input_mode.h"
enum dsky_load_store_route {DSKY_LOAD_STORE_COMPONENT,DSKY_LOAD_STORE_COMPLETE,DSKY_LOAD_STORE_RECYCLE};
/* Original load callers/table setup, literal PUTCOM returns/stores and noun7
 * bit controls. Ends actual PUTCOM, LOADLV or ALMCYCLE; no extra poll. */
enum p66_bias_status dsky_load_store(apollo_overlay_state *,dsky_lamp_io *,enum dsky_load_store_route *);
enum exec_status cm_load_store(apollo_overlay_state *,cm_native_io *,uint16_t *,uint16_t *,enum dsky_load_store_route *);
#endif
