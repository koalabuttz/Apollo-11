/* SPDX-License-Identifier: MIT */
/* T4RUPT (T4RUPT_PROGRAM.agc): the DSKY's relays, the PRO key, and the
 * once-a-second checks.
 *
 * TIME4 interrupts every 120 ms, or every 20 ms while the DSKY's relays are
 * being driven. Each 120 ms T4RUPT sends one relay word to the DSKY
 * (channel 10, OUT0) from the display table DSPTAB, watches the PRO key,
 * and goes on to one of four checks in turn, so each comes round about
 * once a second (T4JUMP: in the LM the RCS jet monitor, the rendezvous
 * radar, the IMU and the autopilot's T4 work; in the CM the optics twice,
 * the IMU and nothing). A relay word sent is turned off again 20 ms later
 * (the quick display phase, QUIKDSP), when DSRUPTSW is negative.
 *
 * This file holds the display and dispatch part (part 1 of the listing
 * file); the checks T4JUMP goes to are still the faithful port's. */
#ifndef READABLE_T4RUPT_H
#define READABLE_T4RUPT_H

#include "../agc/agc.h"

void t4rupt(void);
void t4jump(void);
#if LUMINARY099
void t4jump_plus3(void);
void t4jump_plus7(void);
#else
void opttest(void);
#endif

#endif
