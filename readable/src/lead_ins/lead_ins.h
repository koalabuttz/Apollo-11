/* SPDX-License-Identifier: MIT */
/* The interrupt lead-ins (INTERRUPT_LEAD_INS.agc): where the AGC goes when
 * an interrupt is taken, and how it gets from there to the interrupt's
 * program.
 *
 * The AGC has eleven interrupts, numbered by priority. On interrupt n it
 * saves Z (in ZRUPT) and the next instruction (BRUPT), stops further
 * interrupts and jumps to fixed-fixed address 04000 + 4n: four words per
 * interrupt, the lead-in. 04000 itself is where the AGC starts after a
 * restart (GO). Four words are room for little more than: save A and L
 * (DXCH ARUPT), switch to the handler's banks (a BBCON in BBANK, the old
 * one left in A for the handler to save) and jump. The handler ends the
 * interrupt at WAITLIST's RESUME (or NOQRSM, NOQBRSM), which puts it all
 * back.
 *
 * The T3RUPT lead-in (04014), the Executive's clock for WAITLIST, is in
 * src/waitlist/waitlist.c with T3RUPT. */
#ifndef READABLE_LEAD_INS_H
#define READABLE_LEAD_INS_H

#include "../agc/agc.h"

/* The interrupts by number (the AGC's own, not the listing's): RUPT_10 is
 * the LM's RUPT10 (the landing guidance's) and the CM's HAND CONTROL
 * RUPT. */
enum interrupt_number {
    RESTART_GO,
    RUPT_T6,
    RUPT_T5,
    RUPT_T3,
    RUPT_T4,
    RUPT_KEY1,
    RUPT_KEY2,
    RUPT_UP,
    RUPT_DOWN,
    RUPT_RADAR,
    RUPT_10
};

/* Interrupt n's lead-in: four words from 04000 + 4n. */
#define LEAD_IN(n) ((word)(04000u + 4u * (unsigned)(n)))

void go_lead_in(void);
void t6rupt_lead_in(void);
void t5rupt_lead_in(void);
void t4rupt_lead_in(void);
void keyrupt1_lead_in(void);
void keyrupt2_lead_in(void);
void uprupt_lead_in(void);
void downrupt_lead_in(void);
void radar_rupt_lead_in(void);
#if LUMINARY099
void rupt10_lead_in(void);
#else
void hand_control_rupt_lead_in(void);
void t5rupt(void);
#endif

#endif
