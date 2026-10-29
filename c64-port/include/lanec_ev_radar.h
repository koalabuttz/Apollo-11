/* SPDX-License-Identifier: MIT */
#ifndef LANEC_EV_RADAR_H
#define LANEC_EV_RADAR_H
#include "lanec.h"
/* EXTENDED_VERBS radar test verbs (Luminary099 only, bank 43): R77 through
 * R77END (V63 R04, V78 R77, V79 R77END). Every label and return point is an
 * entry and runs to the first transfer out of these bodies. Out-of-scope
 * input (including any Comanche055 state) returns LANEC_OUT_OF_SCOPE before
 * any change. */
const uint16_t *lanec_ev_radar_entries(uint16_t *count);
enum lanec_status lanec_ev_radar_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
