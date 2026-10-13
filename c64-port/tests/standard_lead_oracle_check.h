/* SPDX-License-Identifier: MIT */
#ifndef STANDARD_LEAD_ORACLE_CHECK_H
#define STANDARD_LEAD_ORACLE_CHECK_H
#include <stdint.h>
/* Case n of tests/standard_lead_oracle_cases.c against its frozen yaAGC digest
 * (tests/standard_lead_oracle_vectors.h on the host; the C64 passes its rows'). For n < 2, also the refusals of the
 * CALL and EXIT words when the banks differ from BANKSET, and the
 * interpretive tables' order. Adds to *checks, returns failures. */
uint16_t slo_check(uint16_t n, uint32_t expected, uint16_t *checks);
#endif
