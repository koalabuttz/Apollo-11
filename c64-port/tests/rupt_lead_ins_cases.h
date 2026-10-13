/* SPDX-License-Identifier: MIT */
#ifndef RUPT_LEAD_INS_CASES_H
#define RUPT_LEAD_INS_CASES_H
#include "rupt_lead_ins.h"
#include "p66_bias.h"
#include "cm_native_image.h"
/* rupt-lead-ins cases: 128 states at every entry of lm_lead_in_routines
 * (mission 0, 1,152 cases) and cm_lead_in_routines (mission 1, 1,920),
 * entered at the original address and run for one routine;
 * tools/rupt_lead_ins_reference.c runs the same entry in the original
 * program until it first reaches the continuation the native run left in Z
 * and FB, or the interrupt ends. */
#define RLI_PER_ENTRY 128u
#define RLI_LM_CASES 1152u
#define RLI_CM_CASES 1920u
#define RLI_RETURNED 0u
#define RLI_REFUSED 2u
typedef struct {p66_bias_io lm;cm_native_io cm;agc_hardware hw;} rli_io;
unsigned rli_cases(unsigned mission);
agc_word rli_entry(unsigned mission,uint16_t n);
void rli_input(unsigned mission,apollo_overlay_state *,rli_io *,uint16_t n);
/* Runs case n natively (one routine); returns the outcome kind (3: not
 * entered). */
unsigned rli_run(unsigned mission,apollo_overlay_state *,rli_io *,uint16_t n);
uint32_t rli_digest(unsigned mission,apollo_overlay_state *,uint16_t n);
#endif
