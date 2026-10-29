/* SPDX-License-Identifier: MIT */
#ifndef P66_REGODSPR_H
#define P66_REGODSPR_H
#include "p66_bias.h"
#include "executive.h"
/* Actual nonflashing V06N60 REGODSPR admission and saved-user return.
 * Entry source PC02470 bank10/EB7, A01474, Q04631, BUF2={03476,062000},
 * MODE0 and active core0 with nonzero source priority. Native NOVAC preserves
 * all physical aliases. Success stops actual ENDOFJOB entry PC05155;
 * resource exhaustion stops actual BAILOUT1 entry, with source alarm01202.
 * Caller owns the canonical image and IRQ exclusion across native calls.
 * No display consumer is executed here; MAKEPLAY is admitted as a real job. */
enum exec_status p66_regodspr(apollo_overlay_state *,p66_bias_io *);
#endif
