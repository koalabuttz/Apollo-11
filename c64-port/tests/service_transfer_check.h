/* SPDX-License-Identifier: MIT */
#ifndef SERVICE_TRANSFER_CHECK_H
#define SERVICE_TRANSFER_CHECK_H
#include <stdint.h>
/* Case n of tests/service_transfer_cases.c against its frozen yaAGC digest
 * (tests/service_transfer_vectors.h on the host; the C64 passes its rows'). For n < 4, also the refusals: UPFLAG
 * and DOWNFLAG at Q 077 and at a fixed Q that is no genuine site, and the
 * routine tables' order. Adds to *checks, returns failures. */
uint16_t svt_check(uint16_t n, uint32_t expected, uint16_t *checks);
#endif
