/* SPDX-License-Identifier: MIT */
/* The LM self-check idle loop, lm_self_check_idle_routines: SELFCHK and the
 * return through SELFRET of src/self_check_body.inc, a table of its own so
 * the flight store loads it without the rest of the file. */
#include "self_check.h"
/* The one inline operand the idle loop reads, CHECKNJ's CADR ADVAN after TC
 * POSTJUMP (43,3335), so the idle module carries it (tests/test_self_check.c
 * checks it against src/lm_self_check_operands.inc). */
const agc_site_operand lm_self_check_idle_sites[1]={{36573u,003214}};
#define SC_IDLE 1
#define SC_MISSION 0u
#define SC_IO p66_bias_io
#define SC_D 2u
#define SC_POSTJUMP 04635u
#define SC_IDLE_TABLE lm_self_check_idle_routines
#include "self_check_body.inc"
