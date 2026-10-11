/* SPDX-License-Identifier: MIT */
#ifndef CM_INTERBANK_ROUTINES_CASES_H
#define CM_INTERBANK_ROUTINES_CASES_H
#include "cm_interbank_routines.h"
/* Case n: genuine site n%sites, context n/sites. The image is a caller that
 * has just executed its `TC <callee>`: Z at the callee, Q at the site's inline
 * CADR, FB/SUPERBNK addressing the caller's bank, deterministic erasable fill. */
#define CM_IBR_CONTEXTS 4u
uint32_t cm_ibr_cases(void);
/* Returns the callee entry (04662 BANKCALL, 04740 IBNKCALL, 04701 POSTJUMP, 04757 USPRCADR). */
agc_word cm_ibr_input(apollo_overlay_state *, cm_native_io *, uint32_t n);
enum cm_interbank_complete_entry cm_ibr_entry(agc_word callee);
/* Physical address of the site's operand word (Q) for case n. */
uint16_t cm_ibr_site(uint32_t n);
/* The program under test: INTER-BANK routines and the compiled operand table. */
extern const agc_native_program cm_ibr_program;
/* One case as the C64 diagnostic runs it: the routine at the callee via the
 * registry, then for BANKCALL/IBNKCALL the trampoline round trip through a
 * native callee and SWRETURN/ISWRETRN. Returns a digest of status, steps,
 * the final image and io (two 16-bit running word sums). */
uint32_t cm_ibr_run(uint32_t n);
#endif
