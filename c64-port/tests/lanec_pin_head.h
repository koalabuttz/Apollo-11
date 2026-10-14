/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PIN_HEAD_H
#define LANEC_PIN_HEAD_H
/* Shared case maker of the pin-head-* suites (tests/lanec_pin_head_*_cases.c).
 * A head runs the original program from one of its points to the next
 * registered point (docs/LANEC-PIN-HEADS.md). Its cases are the accepted
 * cases of the display consumers' own suites (and the suites of the module
 * the head stands in for) that enter at one of the head's points: the states
 * the earlier composite modules took there. The oracle stops where the head
 * stopped, with the same Q (lanec_pw_exit_stop). Test-only. */
#include "lanec_pw.h"

typedef struct { uint8_t mission, bank; uint16_t z; } lanec_pin_head_point;
typedef struct {
    const lanec_suite *const *sources;   /* 0-terminated */
    const lanec_pin_head_point *points;
    uint16_t point_count;
    lanec_run run;                       /* the head */
} lanec_pin_head_family;

/* Case n of the family: source case n mod (the number found), so a suite of
 * `cases` cases cycles through them; *round is n div that number, for a
 * suite that varies a found case (the superbank, ...). */
void lanec_pin_head_make(const lanec_pin_head_family *, uint32_t n, apollo_overlay_state *, lanec_io *,
                         lanec_meta *, uint32_t *round);
#endif
