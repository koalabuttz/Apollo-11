/* SPDX-License-Identifier: MIT */
#ifndef DSKY_INPUT_STORE_H
#define DSKY_INPUT_STORE_H
#include "dsky_input_select.h"
enum dsky_input_store_route {DSKY_INPUT_STORE_CALLER,DSKY_INPUT_STORE_DECIMAL,DSKY_INPUT_STORE_RECYCLE,DSKY_INPUT_STORE_CHANNEL};
/* Original PUTCOM/PUTNORM/PUTDPCOM/PUTCOM2. Source selected ordinary
 * canonical data destination; decimal and channel branches stop at literal
 * PUTDECSF/CHANLOAD, no fabricated scaling/channel write. */
enum p66_bias_status dsky_input_store(apollo_overlay_state *,dsky_lamp_io *,enum dsky_input_store_route *);
enum exec_status cm_input_store(apollo_overlay_state *,cm_native_io *,uint16_t *,uint16_t *,enum dsky_input_store_route *);
#endif
