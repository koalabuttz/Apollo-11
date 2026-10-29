/* SPDX-License-Identifier: MIT */
#ifndef INTERBANK_Q_CHECK_H
#define INTERBANK_Q_CHECK_H
#include <stdint.h>
/* Case n of tests/interbank_q_cases.c against its frozen yaAGC digest
 * (tests/interbank_q_vectors.h), plus the refusals at the Q edges: SWCALL -1
 * with Q 07777 and BANKJUMP with Q 07776 or 07777. Adds to *checks, returns
 * failures. */
uint16_t ibq_check(uint16_t n, uint16_t *checks);
#endif
