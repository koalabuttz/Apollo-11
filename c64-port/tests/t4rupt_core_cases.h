/* SPDX-License-Identifier: MIT */
#ifndef T4RUPT_CORE_CASES_H
#define T4RUPT_CORE_CASES_H
#include "t4rupt_core.h"
#include "p66_bias.h"
#include "cm_native_image.h"
/* t4rupt-core cases: 2,048 states at T4RUPT and 64 at each return entry of
 * lm_t4rupt_core_routines (mission 0, 2,240 cases) and
 * cm_t4rupt_core_routines (mission 1, 2,176), entered at the original
 * address and run for one routine; tools/t4rupt_core_reference.c runs the
 * same entry in the original program until it first reaches the
 * continuation the native run left in Z and FB. DSRUPTSW is a phase count
 * (0-8) or a quick-display value, DSPCNT 0-012 or -0, and the display
 * table, NOUT, FLAGWRD5, IMODES33 and channel 32 are random. */
#define T4_T4RUPT_CASES 2048u
#define T4_RETURN_CASES 64u
#define T4_LM_CASES 2240u
#define T4_CM_CASES 2176u
#define T4_RETURNED 0u
#define T4_REFUSED 2u
typedef struct {p66_bias_io lm;cm_native_io cm;agc_hardware hw;} t4_io;
unsigned t4_cases(unsigned mission);
agc_word t4_entry(unsigned mission,uint16_t n);
void t4_input(unsigned mission,apollo_overlay_state *,t4_io *,uint16_t n);
/* Runs case n natively (one routine); returns the outcome kind (3: not
 * entered). */
unsigned t4_run(unsigned mission,apollo_overlay_state *,t4_io *,uint16_t n);
uint32_t t4_digest(unsigned mission,apollo_overlay_state *,uint16_t n);
#endif
