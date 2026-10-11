/* SPDX-License-Identifier: MIT */
#ifndef P66_MAJOR_MODE_H
#define P66_MAJOR_MODE_H
#include "p66_bias.h"
#include "executive.h"
/* Fixed STARTP66 NEWMODEX call (inline66 atsource2501) through caller2502
 * or BAILOUT1. Updates MODREG and admits the real basic DSPMMJB NOVAC job
 * with priority030000 and 2CADR03534/060102. Does not execute that job here.
 * Physical bank31/EB7, Q2501,L01400 (FASTCHNG output), running core0 required.
 * Image authoritative, caller excludes producers, no mutation on bad domain. */
enum exec_status p66_major_mode_start(apollo_overlay_state *,p66_bias_io *);
#endif
