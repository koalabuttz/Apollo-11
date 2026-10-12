/* SPDX-License-Identifier: MIT */
#ifndef LANEC_EV_TRACK_H
#define LANEC_EV_TRACK_H
#include "lanec.h"
/* EXTENDED_VERBS basic code of the tracking and state-vector verbs: V69,
 * LM TRMTRAK1 (bank 42), DSPRRLOS/RRLOSDSP (bank 40) and ATTACHIT (bank
 * 43); CM TRACKTRM, CLEANOUT, SETVHFLG, RESETVHF, ATTACHIT, TACHEXIT,
 * LMTOCMSV and LMTOCM (bank 43). Every label, return point and word after
 * an interpretive EXIT is an entry of its bank and runs to the first
 * transfer out of these bodies (TC INTPRET included). Out-of-scope input
 * returns LANEC_OUT_OF_SCOPE before any change. */
const uint16_t *lanec_ev_track_entries(enum lanec_mission, uint16_t bank, uint16_t *count);
enum lanec_status lanec_ev_track_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
