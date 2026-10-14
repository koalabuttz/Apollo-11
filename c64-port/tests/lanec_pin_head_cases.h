/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PIN_HEAD_CASES_H
#define LANEC_PIN_HEAD_CASES_H
/* Case states for the pin-head-* suites (docs/LANEC-PIN-HEADS.md). */
#include "lanec_suites.h"
#include "agc_transfer.h"

/* pin-transfer's program (tests/lanec_pin_transfer_cases.c): every Pinball
 * routine and the tables it calls, run from *s until Z reaches stop. */
enum exec_status lanec_pin_chain(apollo_overlay_state *, lanec_io *, enum lanec_mission, uint16_t stop, uint16_t *steps);

/* A state at bank,point (mission m) where the original gets to: a case of
 * src (the composite the head replaces) that src accepts, entered there or
 * run through the Pinball routines until it arrives. Tries `tries` cases
 * from number n on; 0 when none arrives (*s and *io are then junk). */
int lanec_pin_head_reach(uint32_t n, uint32_t tries, const lanec_suite *src, uint16_t bank, uint16_t point,
                         enum lanec_mission m, apollo_overlay_state *s, lanec_io *io);
#endif
