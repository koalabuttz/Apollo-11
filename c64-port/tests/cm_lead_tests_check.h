/* SPDX-License-Identifier: MIT */
#ifndef CM_LEAD_TESTS_CHECK_H
#define CM_LEAD_TESTS_CHECK_H
#include <stdint.h>
/* Case n of tests/cm_lead_tests_cases.c against its frozen yaAGC digest; for
 * n < 3 also that the word refuses with an EXTEND pending and that FB 033 under
 * superbank 0100 (bank 043) does not resolve it. Adds to *checks, returns failures. */
uint16_t clt_check(uint16_t n, uint16_t *checks);
#endif
