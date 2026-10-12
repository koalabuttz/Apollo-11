/* SPDX-License-Identifier: MIT */
/* The whole file's routine table (the C64 phase groups build their own from
 * the routines they link). */
#include "lm_ephemeris_routines.h"
/* Sorted by key. */
static const agc_routine_entry entries[10]={
 {LM_EPHEMERIS_KEY(03664),lm_ephemeris_route_lspos},{LM_EPHEMERIS_KEY(03700),lm_ephemeris_route_posita},
 {LM_EPHEMERIS_KEY(03722),lm_ephemeris_route_positd},{LM_EPHEMERIS_KEY(03743),lm_ephemeris_route_positf},
 {LM_EPHEMERIS_KEY(03752),lm_ephemeris_route_moon_vdef},{LM_EPHEMERIS_KEY(03753),lm_ephemeris_route_moon_unit},
 {LM_EPHEMERIS_KEY(03756),lm_ephemeris_route_sun_angles},{LM_EPHEMERIS_KEY(03763),lm_ephemeris_route_sun_vdef},
 {LM_EPHEMERIS_KEY(03764),lm_ephemeris_route_sun_unit},{LM_EPHEMERIS_KEY(03775),lm_ephemeris_route_lspos}};
const agc_routine_table lm_ephemeris_routines={entries,10};
