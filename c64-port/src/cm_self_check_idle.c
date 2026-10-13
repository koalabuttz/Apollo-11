/* SPDX-License-Identifier: MIT */
/* The CM self-check idle loop, cm_self_check_idle_routines: SELFCHK and the
 * return through SELFRET of src/self_check_body.inc, a table of its own so
 * the flight store loads it without the rest of the file. */
#include "self_check.h"
/* The one inline operand the idle loop reads, CHECKNJ's CADR ADVAN after TC
 * POSTJUMP (43,3337), so the idle module carries it (tests/test_self_check.c
 * checks it against src/cm_self_check_operands.inc). */
const agc_site_operand cm_self_check_idle_sites[1]={{36575u,003231}};
#define SC_IDLE 1
#define SC_MISSION 1u
#define SC_IO cm_native_io
#define SC_D 0u
#define SC_POSTJUMP 04701u
#define SC_IDLE_TABLE cm_self_check_idle_routines
#include "self_check_body.inc"
