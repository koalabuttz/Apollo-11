/* SPDX-License-Identifier: MIT */
#ifndef RESIDENT_RUPT_CASES_H
#define RESIDENT_RUPT_CASES_H
/* Cases for the flight resident's timers and interrupt entry
 * (include/resident_rupt.h), shared by the yaAGC reference
 * (tools/resident_rupt_reference.c) and tests/test_resident_rupt.c.
 *
 * Entry cases: a random image at a native boundary (Z in fixed memory under
 * a random FB, EB and superbank, or in erasable), A with or without
 * overflow, interrupts enabled or not, in an interrupt or not, and a random
 * set of pending requests 1-10. One case in eight puts Z on a RELINT,
 * INHINT or EXTEND word. Timer cases: TIME1-TIME6 and channel 13 bit 15;
 * the first 32768 set every timer to the case number, the rest are random
 * values biased to the overflow and zero edges.
 *
 * T3RUPT cases: the whole interrupt through the thin kernel. A random image
 * with Z in unswitched erasable on a word that is not RELINT, INHINT or
 * EXTEND, A without overflow, interrupts enabled, no interrupt in progress
 * and only T3RUPT pending; the Waitlist in WAITLIST's own form (TIME3 just
 * past its overflow, one to eight tasks with LST1 = -(dt)+1, the free LST1
 * slots at NEG1/2), and every task 2CADR in LST2 is TASKOVER's,
 * so T3RUPT, each task due and TASKOVER run until RESUME ends the interrupt
 * at ZRUPT - 1 (the yaAGC run stops where RESUME substitutes BRUPT). */
#include <stdint.h>
#include "agc_transfer.h"
#define RR_ENTRY_CASES 8192u
#define RR_TIMER_CASES (32768u + 8192u)
#define RR_T3_CASES 2048u
typedef struct {
    apollo_overlay_state *s;   /* the caller's image */
    agc_word superbank, interrupt_enabled;
    uint16_t in_interrupt, pending;
} rr_case;
/* The mission's rope word at physical bank*1024+offset (0 for none). */
typedef agc_word (*rr_rope)(unsigned mission, uint16_t physical);
void rr_entry_input(unsigned mission, uint16_t n, rr_rope rope, rr_case *c);
void rr_t3_input(unsigned mission, uint16_t n, rr_case *c);
/* Timer case n: words 024-031 and channel13 (bit 15 only). */
void rr_timer_input(uint32_t n, agc_word time[6], agc_word *channel13);
/* FNV-1a over 16-bit values; a case's digest starts from RR_BASIS, and
 * the frozen vectors fold each block of RR_BLOCK case digests (rr_fold). */
#define RR_BASIS UINT32_C(2166136261)
#define RR_BLOCK 64u
uint32_t rr_mix(uint32_t h, uint16_t v);
uint32_t rr_fold(uint32_t h, uint32_t digest);
#endif
