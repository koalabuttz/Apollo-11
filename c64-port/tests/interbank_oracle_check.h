/* SPDX-License-Identifier: MIT */
#ifndef INTERBANK_ORACLE_CHECK_H
#define INTERBANK_ORACLE_CHECK_H
#include <stdint.h>
/* Case n of tests/interbank_oracle_cases.c against its frozen yaAGC digest
 * (tests/interbank_oracle_vectors.h on the host; the C64 passes its rows').
 * For n < 2, also BANKCALL refusing a fixed Q that follows no genuine site
 * and the routine tables' order. Adds to *checks, returns failures. */
uint16_t ibo_check(uint16_t n, uint32_t expected, uint16_t *checks);
#endif
