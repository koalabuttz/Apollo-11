/* SPDX-License-Identifier: MIT */
#ifndef LM_INTERBANK_ROUTINES_CASES_H
#define LM_INTERBANK_ROUTINES_CASES_H
#include "lm_interbank_routines.h"
/* Case n: genuine site n%sites, context n/sites. The image is a caller that
 * has just executed its `TC <callee>`: Z at the callee, Q at the site's inline
 * CADR, FB/SUPERBNK addressing the caller's bank, deterministic erasable fill. */
#define LM_IBR_CONTEXTS 4u
uint32_t lm_ibr_cases(void);
/* Returns the callee entry (04616 BANKCALL, 04674 IBNKCALL, 04635 POSTJUMP, 04713 USPRCADR). */
agc_word lm_ibr_input(apollo_overlay_state *, p66_bias_io *, uint32_t n);
enum lm_interbank_entry lm_ibr_entry(agc_word callee);
/* Physical address of the site's operand word (Q) for case n. */
uint16_t lm_ibr_site(uint32_t n);
/* The program under test: INTER-BANK routines and the compiled operand table. */
extern const agc_native_program lm_ibr_program;
/* One case as the C64 diagnostic runs it: the routine at the callee via the
 * registry, then for BANKCALL/IBNKCALL the trampoline round trip through a
 * native callee and SWRETURN/ISWRETRN. Returns a digest of status, steps,
 * the final image and io (two 16-bit running word sums). */
uint32_t lm_ibr_run(uint32_t n);
#endif
