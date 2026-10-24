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
 * In the CM the same drive empties the optics counters too: CDUTCMD (053)
 * on channel 14 bit 12 and CDUSCMD (054) on bit 11, which in TVC mode are
 * the SPS gimbal commands TVCYAW and TVCPITCH (TVCDAPS.agc POUT, YOUT).
 * Their pulses go to the optics error counters (whose DAC drives the
 * gimbal, 85.41 arcseconds a pulse), at most 240 a burst (3,200 a second).
 * yaAGC is no model for them: it dumps CDUS/CDUT, not the command counters
 * (tools/flight_host_differential.py frozen_engine). In the LM 053/054
 * drive the rendezvous radar, which the host's radar model empties, so the
 * LM drive stops at 052.
 *
 * The C64 flight resident runs it (as platform/c64/resident_cdu.S, the
 * same drive in 6502) once each period, at the work clock's TIME5 step
 * (platform/c64/resident_flight.c rk_rupts_due); the flight host harness
 * runs the C at the same step. The counters it runs over: RK_CM 050-054,
 * RK_LM 050-052; a build for neither (the host's shared object, the yaAGC
 * vectors) reads rk_cdu_count, 3 (050-052) unless set to 5. */
typedef struct {
    int16_t owed[5];
    uint16_t elapsed;
    uint8_t active;
} rk_cdu_state;
extern rk_cdu_state rk_cdu;
/* Defined only in a build for neither mission (the flight host harness
 * sets it for the CM; see above). */
extern uint8_t rk_cdu_count;
void rk_cdu_drive(apollo_overlay_state *s, agc_hardware *hw);
#endif
