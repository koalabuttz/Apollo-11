/* SPDX-License-Identifier: MIT */
#ifndef LM_IDLE_DISPATCH_H
#define LM_IDLE_DISPATCH_H
#include "lm_basic_change.h"
enum lm_idle_dispatch_entry {LM_IDLE_ADVAN,LM_IDLE_NUCHANG2,LM_IDLE_NUDIRECT};
/* Literal source ADVAN03214, NUCHANG203011 or NUDIRECT03225. Legal pending
 * core offsets and +/-0, ordinary bank/CADR and explicit channel11 domain.
 * Returns actual basic/INTRSM transfer; SELFRET external self-check returns
 * EXEC_IDLE with its real nonzero pair. No callback or additional IRQ poll.
 * The caller owns exclusion; unsupported objects remain unchanged. */
enum exec_status lm_idle_dispatch(apollo_overlay_state *,p66_bias_io *,agc_word *,
 enum lm_idle_dispatch_entry,exec_transfer *);
#endif
