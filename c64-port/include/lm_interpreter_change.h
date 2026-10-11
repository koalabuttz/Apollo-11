/* SPDX-License-Identifier: MIT */
#ifndef LM_INTERPRETER_CHANGE_H
#define LM_INTERPRETER_CHANGE_H
#include "p66_bias.h"
#include "executive.h"
/* Literal CHANG2 at 05126 through source DTCB or INTRSM/NEWOPS. Only a
 * completed interpretive pair (EDOP+0), positive active/selected priorities,
 * valid NEWJOB core offset and source addresses are accepted. All guards run
 * before mutation. One canonical image, explicit SUPER/IRQ, restoring caller
 * exclusion. No instruction fetch, decoder or native callback occurs here. */
enum exec_status lm_interpreter_change(apollo_overlay_state *,p66_bias_io *,exec_transfer *);
#endif
