/* SPDX-License-Identifier: MIT */
#ifndef WAITLIST_ENTRIES_CASES_H
#define WAITLIST_ENTRIES_CASES_H
/* Cases for the WAITLIST entries other files reach (milestone
 * waitlist-entries). Kinds:
 *  0 LM T3RUPT 01,3407 and 1 CM T3RUPT 01,3416: each T3 vector case of
 *    lm-exec-jobs / cm-exec-services after the vector's own words (DXCH ARUPT;
 *    CAF T3RPTBB; XCH BBANK), to the dispatched task;
 *  2 LM and 3 CM DLY2 -1: each DELAYJOB entry case of service-transfer, from
 *    DELAYJOB through DLY2 -1, WAIT2 and TCGETCAD to JOBSLEEP (or BAILOUT);
 *  4 LM NOQBRSM +1 05275 (the P-axis autopilot's RELINT; RESUME): random
 *    erasable inside an interrupt, to the end of the interrupt.
 * The case list (kind, base case) is frozen in waitlist_entries_vectors.h by
 * tools/waitlist_entries_freeze.py; WLE_SCAN builds the scan that picks it. */
#include "agc_transfer.h"
#define WLE_KINDS 5u
#define WLE_RESUME_CASES 128u
#ifndef WLE_SCAN
uint16_t wle_cases(void);
uint8_t wle_kind(uint16_t n);
uint16_t wle_case_base_of(uint16_t n);
uint32_t wle_frozen_digest(uint16_t n);
#endif
/* Fills image, io (p66_bias_io or cm_native_io by kind) and hardware. */
void wle_input(apollo_overlay_state *s, void *io, agc_hardware *hw, uint8_t kind, uint16_t base);
/* Superbank and interrupt-enable of the io record. */
void wle_io(const void *io, uint8_t kind, uint16_t *superbank, uint16_t *enabled);
/* 0 refused, 1 reached an unmapped continuation or ended the interrupt. */
unsigned wle_run_base(apollo_overlay_state *s, void *io, agc_hardware *hw, uint8_t kind, uint16_t base);
#ifndef WLE_SCAN
unsigned wle_run(apollo_overlay_state *s, void *io, agc_hardware *hw, uint16_t n);
uint32_t wle_digest(apollo_overlay_state *s, uint16_t n);
#endif
/* io storage large enough for either record. */
typedef union { char lm[8]; char cm[64]; } wle_io_storage;
#endif
