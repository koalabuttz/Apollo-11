/* SPDX-License-Identifier: MIT */
#ifndef P66_RUNTIME_REGODSPR_H
#define P66_RUNTIME_REGODSPR_H
#include "p66_regodspr.h"
/* Same actual REGODSPR admission boundary as p66_regodspr, with pending
 * NEWJOB0, valid core displacements12..84 referencing active positive-priority
 * cores, or original idle marker-0. NOVAC compares the new display priority
 * with the pending core and preserves/replaces NEWJOB as the source requires.
 * Source NOCORE reaches BAILOUT1. Unsupported image/io unchanged. Caller owns
 * exclusion; no job is consumed or polled at an invented basic boundary. */
enum exec_status p66_runtime_regodspr(apollo_overlay_state *,p66_bias_io *);
#endif
