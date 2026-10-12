/* SPDX-License-Identifier: MIT */
#ifndef LANEC_EV_SHORT_H
#define LANEC_EV_SHORT_H
#include "lanec.h"
/* EXTENDED_VERBS short verb bodies (both missions, bank 43): the flag
 * verbs, job and task starters, check-and-hand-over verbs and SHOWSUM.
 * Every label and every return point (after TESTXACT, CHKPOOH, FINDVAC,
 * NOVAC, BANKCALL, UPFLAG/DOWNFLAG, CHECKMM, PHASCHNG and the like) is an
 * entry; each runs to the first transfer out of these bodies. Out-of-scope
 * input returns LANEC_OUT_OF_SCOPE before any change. */
/* The entry addresses (bank 43) and membership test. */
const uint16_t *lanec_ev_short_entries(enum lanec_mission, uint16_t *count);
uint8_t lanec_ev_short_entry(enum lanec_mission, uint16_t z);
enum lanec_status lanec_ev_short_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
