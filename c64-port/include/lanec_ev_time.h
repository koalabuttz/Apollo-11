/* SPDX-License-Identifier: MIT */
#ifndef LANEC_EV_TIME_H
#define LANEC_EV_TIME_H
#include "lanec.h"
/* EXTENDED_VERBS V55 clock update (Luminary099 R33, bank 42; Comanche055
 * ALINTIME, bank 43) and the Luminary099 V43 ATTCK2 task (bank 42). Every
 * label and return point is an entry and runs to the first transfer out of
 * these bodies. Out-of-scope input returns LANEC_OUT_OF_SCOPE before any
 * change. */
const uint16_t *lanec_ev_time_entries(enum lanec_mission, uint16_t *count);
enum lanec_status lanec_ev_time_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
