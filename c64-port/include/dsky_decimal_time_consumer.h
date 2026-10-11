/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DECIMAL_TIME_CONSUMER_H
#define DSKY_DECIMAL_TIME_CONSUMER_H
#include "dsky_decimal_consumer.h"
/* Explicit original DECDSP stages for source routines8(HMS),9(M/S),12(two
 * positive integers): load0/select1, HMS2..9, M/S10..15, integers16..20,
 * actual DSPDCEND+2 row continuation21. Canonical ordinary RAM/source
 * arithmetic domains/allEB/selectingSUPER required. Caller excludes
 * producers and owns distinct IO/route. No saved ENTRET execution. */
enum p66_bias_status dsky_decimal_time_consumer(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,uint8_t,enum dsky_decimal_consumer_route *);
#endif
