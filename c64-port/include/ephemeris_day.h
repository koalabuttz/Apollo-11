/* SPDX-License-Identifier: MIT */
#ifndef EPHEMERIS_DAY_H
#define EPHEMERIS_DAY_H
#include "overlay_runtime.h"
#include "fixed_data.h"
#include "agc_word.h"
/* Original scalar DDV/BDDV body at PC02353, FB0 and coherent EB/BB,
 * DP MODE+0, with immutable CSTODAY={020,17260} already in BUF. Ordinary
 * canonical RAM/Q and raw16 A/L; ADDRWD is unrelated retained debris.
 * Caller owns canonical storage exclusively and excludes producers.
 * Return at genuine mission DANZIG, without dispatch or device changes.
 * Unsupported image/null/mission preserves storage and returns zero. */
uint8_t ephemeris_day_divide(apollo_overlay_state *,enum apollo_fixed_mission);
#endif
