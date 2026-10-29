/* SPDX-License-Identifier: MIT */
/* The resident's null module (RK_MODULE_BOUNDARY): its main only returns.
 * The boundary-word table it once carried ends the REU store image instead
 * (tools/overlay_v2_pack.py --tail, tools/resident_boundary_words.py), so it
 * grows with the store's resolver records and no slot bounds it; the load
 * check (platform/c64/flight_load_check.c) times a cached call through this
 * module. */
#include "resident_flight.h"
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){(void)selector;
 f->value=0;return APOLLO_OV2_DONE;}
