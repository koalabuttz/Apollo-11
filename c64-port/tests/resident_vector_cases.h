/* SPDX-License-Identifier: MIT */
#ifndef RESIDENT_VECTOR_CASES_H
#define RESIDENT_VECTOR_CASES_H
/* Interrupt cases for every vector but T3RUPT, through the flight resident:
 * the entry (include/resident_rupt.h), the lead-in and the interrupt bodies
 * the resident's task-slot wrappers run (platform/c64/resident_rupt_module.c),
 * and the thin kernel where a body calls it, until RESUME ends the
 * interrupt or the run reaches a point no table holds. Shared by the yaAGC
 * reference (tools/resident_rupt_reference.c), tests/test_resident_rupt.c
 * and the C64 check's case module.
 *
 * Each case takes its erasable image and io from the Kernel lane's own case
 * generator for the body the vector reaches, so the body's input domain
 * holds: T4RUPT from t4_input (the T4RUPT core), KEYRUPT1 and UPRUPT from
 * ku_input (KEYRUPT/UPRUPT), LM T6RUPT from lm_t6_input (its vector cohort)
 * and every other vector from rli_input (its lead-in). Then the case is
 * moved to before the interrupt: Z on an unswitched erasable word that is
 * not RELINT, INHINT or EXTEND, A without overflow, interrupts enabled, no
 * interrupt in progress and only this vector's request pending.
 * RR_VECTOR_PER cases per vector, vectors in rr_vectors order. */
#include "resident_rupt_cases.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#define RR_VECTOR_PER 128u
#define RR_VECTORS 9u
#define RR_VECTOR_CASES (RR_VECTORS * RR_VECTOR_PER)
typedef struct {
    apollo_overlay_state *s;   /* the caller's image */
    p66_bias_io lm;
    cm_native_io cm;
    agc_hardware hw;
    uint16_t pending;
} rr_vcase;
/* The request (RK_RUPT_*) of case n. */
uint8_t rr_vector(uint16_t n);
void rr_vector_input(unsigned mission, uint16_t n, rr_vcase *c);
/* Run kinds (tests/resident_vector_run.h). */
enum { RR_V_RESUMED = 0, RR_V_STOPPED = 1, RR_V_REFUSED = 2, RR_V_NOT_TAKEN = 3, RR_V_RUNAWAY = 4 };
/* The case's digest after a run of the given kind: the kind, and unless
 * the run was refused or not taken, all 2048 words, the superbank bits,
 * the interrupt enable, the interrupt flag and channels 5, 6, 10, 11, 13. */
uint32_t rr_vector_digest(unsigned mission, const rr_vcase *c, uint8_t kind);
#endif
