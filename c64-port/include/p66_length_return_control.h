/* SPDX-License-Identifier: MIT */
#ifndef P66_LENGTH_RETURN_CONTROL_H
#define P66_LENGTH_RETURN_CONTROL_H
#include "p66_native_descent.h"
/* Raw source return from compiled feedback/horizontal ABVAL finish (212/236).
 * Observe the existing GENSCR unnormalization post-state; numerical MPAC,
 * scratch and VAC remain authoritative in image. Q is the original TC return
 * debris, retained by the following timestamp PDDL. No scheduler admission. */
uint16_t p66_length_return_control(apollo_overlay_state *,uint16_t selector);
/* STODL LASTTPIP invokes STORE/TC Q, replacing the preceding root's Q. */
uint16_t p66_feedback_store_return_control(apollo_overlay_state *);
#endif
