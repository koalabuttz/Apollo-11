/* SPDX-License-Identifier: MIT */
/* The keyboard and uplink interrupts (KEYRUPT_UPRUPT.agc): a key pressed
 * on the main DSKY (KEYRUPT1), or a word sent up from the ground
 * (UPRUPT), becomes a five-bit key code handed to a new CHARIN job, the
 * keyboard program (PINBALL).
 *
 * Each routine runs inside the interrupt, from its label to the
 * instruction that leaves it: TC LODSAMPT (the time of the key, for noun
 * 65), TC NOVAC (CHARIN's job) or TC RESUME (the end of the interrupt).
 * The flight program runs that routine next and comes back to the
 * _after_ routine here. */
#ifndef READABLE_KEYRUPT_H
#define READABLE_KEYRUPT_H

#include "../agc/agc.h"

void keyrupt1(void);
void keyrupt1_after_lodsampt(void);
void keycom(void);
void acceptup_after_novac(void);
void uprupt(void);
void uprupt_after_lodsampt(void);

#endif
