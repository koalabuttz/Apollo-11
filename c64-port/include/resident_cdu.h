/* SPDX-License-Identifier: MIT */
#ifndef RESIDENT_CDU_H
#define RESIDENT_CDU_H
#include "agc_transfer.h"
/* rk_cdu_drive is the IMU CDU drive for one 10 ms period, as yaAGC's
 * BurstOutput: while channel 14 bit 15, 14 or 13 is set it empties
 * CDUXCMD, CDUYCMD or CDUZCMD (050-052) into the pulses that axis owes, and
 * a burst sends at most 24 of each axis's pulses (the pulses themselves go
 * to the IMU, which no AGC word sees). A burst comes at once when the last
 * one left nothing owed, else 75 ms after the last. A count written during
 * a period empties at its end, where yaAGC empties it an instruction after
 * the write; the 75 ms bursts after it then land in the same periods as
 * yaAGC's. rk_cdu holds the drive's state (all zero at a fresh start, as
 * yaAGC's).
 *
 * The C64 flight resident runs it after each rk_timer_tick through the
 * pinned module's tick hook; host builds call it after rk_timer_tick. */
typedef struct { int16_t owed[3]; uint16_t elapsed; uint8_t active; } rk_cdu_state;
extern rk_cdu_state rk_cdu;
void rk_cdu_drive(apollo_overlay_state *s, agc_hardware *hw);
#endif
