/* SPDX-License-Identifier: MIT */
#ifndef DANZIG_PENDING_CHECK_H
#define DANZIG_PENDING_CHECK_H
#include <stdint.h>
#include "agc_transfer.h"
/* Checks for row n of tests/danzig_pending_vectors.h (the yaAGC state at the
 * pending DANZIG after an RTB-left pair): the transfer point and resolution
 * with the mission's marker, without it, with the other mission's marker,
 * with EDOP +0, negative and -0, at NEWOPS, and one trampoline step into the
 * routine registered at the pair word +1. Adds to *checks, returns failures. */
uint16_t dzp_check(uint16_t n, uint16_t *checks);
uint16_t dzp_row_count(void);
#endif
