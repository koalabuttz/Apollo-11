/* SPDX-License-Identifier: MIT */
#ifndef FLIGHT_WATCHMAN_H
#define FLIGHT_WATCHMAN_H
/* The C64 flight resident's night watchman (docs/FLIGHT-RESTART.md, The
 * night watchman): rk_nw holds the clock period count mod 128 in bits 1-7
 * (FLIGHT_WATCHMAN_PERIOD a period, so the count wraps with the byte) and
 * the armed bit in bit 0 (FLIGHT_WATCHMAN_ARMED). The resident counts a
 * period at each T5 step (platform/c64/resident_flight.c rk_rupts_due): it
 * arms at 64 (FLIGHT_WATCHMAN_ARM) and sounds at 0 when still armed. A
 * NEWJOB access (the kernel's CCS and TS NEWJOB, the idle memo, a step from
 * an interpretive point) clears the armed bit. The host harness writes the
 * byte into each snapshot (tools/flight_host_run.c SNAP_NW), so a replay
 * window goes on with the host's watchman. */
extern unsigned char rk_nw;
#define FLIGHT_WATCHMAN_ARMED 1u
#define FLIGHT_WATCHMAN_PERIOD 2u
#define FLIGHT_WATCHMAN_ARM (64u * FLIGHT_WATCHMAN_PERIOD)
#define FLIGHT_WATCHMAN_ACCESS() (rk_nw &= (unsigned char)~FLIGHT_WATCHMAN_ARMED)
#endif
