/* SPDX-License-Identifier: MIT */
#ifndef ELAPSED_RETURN_CONTROL_CASES_H
#define ELAPSED_RETURN_CONTROL_CASES_H
#include "p66_elapsed_return_control.h"
#include "elapsed_cases.h"
#define EQ_CASES 8192u
extern const unsigned eq_addresses[41];
void eq_prepare(apollo_overlay_state *,uint16_t);
void eq_gather(elapsed_case *,const apollo_overlay_state *);
void eq_commit(apollo_overlay_state *,const elapsed_case *);
#endif
