/* SPDX-License-Identifier: MIT */
#ifndef RESIDENT_CDU_CASES_H
#define RESIDENT_CDU_CASES_H
#include "agc_word.h"
/* The IMU CDU drive cases (src/resident_cdu.c rk_cdu_drive): one timeline
 * of RC_PERIODS 10 ms periods from a fresh start. Period n first sets
 * channel 14 to rc_input's ch14 and stores each axis's write[k] into
 * CDUXCMD + k (050-052) where set[k], then runs 10 ms; its digest is
 * 050-052 at the end. tools/resident_cdu_reference.c runs it on yaAGC
 * (agc_engine's own BurstOutput); tests/test_resident_cdu.c runs
 * rk_cdu_drive once per period. */
#define RC_PERIODS 65536u
#define RC_BLOCK 64u
void rc_input(uint32_t n, agc_word *ch14, uint8_t set[3], agc_word write[3]);
uint32_t rc_digest(const agc_word *cmd);
uint32_t rc_fold(uint32_t h, uint32_t digest);
#define RC_BASIS UINT32_C(2166136261)
#endif
