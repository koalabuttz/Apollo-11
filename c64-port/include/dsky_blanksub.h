/* SPDX-License-Identifier: MIT */
#ifndef DSKY_BLANKSUB_H
#define DSKY_BLANKSUB_H
#include "dsky_blank.h"
/* Actual ordinary BLANKSUB LM4255/CM4271, all FB/EB/SUPER banks;
 * A raw16 blankingcode masked7, Q ordinary0100..07775, DSPLOCK0/1.
 * Busy returns actual callerQ; available returns Q+1. Selected isolated
 * relay must be nonzero only on available nonzero mask paths. Captures
 * original caller bank/SUPER into BUF, then composes real blanking and
 * SUPDXCHZ+1 return with no scheduler poll. Caller excludes producers.
 * Unsupported inputs preserve entire image and IO. */
enum p66_bias_status dsky_blanksub(apollo_overlay_state *,p66_bias_io *);
enum exec_status cm_blanksub(apollo_overlay_state *,cm_native_io *);
#endif
