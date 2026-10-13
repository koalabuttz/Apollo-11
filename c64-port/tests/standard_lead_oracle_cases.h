/* SPDX-License-Identifier: MIT */
#ifndef STANDARD_LEAD_ORACLE_CASES_H
#define STANDARD_LEAD_ORACLE_CASES_H
/* Cases for milestone standard-lead-oracle: every registered entry of both
 * missions' SYSTEM_TEST_STANDARD_LEAD_INS (E/BKCALL, its three SWRETURN
 * landing points, E/CALL, the post-EXIT resume, E/JOBWAK), the INTERPRETER
 * points its paths reach (INTPRET, INTPRET+2) and its two interpretive words
 * at the fetch point (CALL EBUF2, a CHANG2 resume LOC, and EXIT), entered from
 * a deterministic image and run through agc_transfer_run to the first
 * continuation outside the file. Case n: mission n%2, kind (n/2)%SLO_KINDS.
 * Callers are erasable (the file's purpose: the flight rope has no E/BKCALL or
 * E/CALL caller), callees sit in banks 020 and up. */
#include "agc_transfer.h"
#define SLO_KINDS 11u
#define SLO_PER_KIND 64u
#define SLO_CASES (2u*SLO_KINDS*SLO_PER_KIND)
#define SLO_MAX_STEPS 16u
uint8_t slo_mission(uint16_t n);
uint8_t slo_kind(uint16_t n);
void slo_input(apollo_overlay_state *s, uint16_t *superbank, uint16_t *inhint, uint16_t n);
/* 0 refused, 1 reached a continuation outside the file (image and *inhint left there). */
unsigned slo_run(apollo_overlay_state *s, uint16_t *superbank, uint16_t *inhint, uint16_t n);
uint32_t slo_digest(apollo_overlay_state *s, uint16_t n);
#endif
