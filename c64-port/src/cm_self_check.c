/* SPDX-License-Identifier: MIT */
/* Comanche055 AGC_BLOCK_TWO_SELF-CHECK (43,3234-3725): src/self_check_body.inc
 * with Comanche055's addresses and constants. */
#include "self_check.h"
#include "cm_self_check_operands.inc"
#define SC_D 0u
#define SCK(n) CM_SC_K_##n
#define SC_ALARM2 05652u
#define SC_CHANG1 05164u
#define SC_LEFT5 04345u
#define SC_SUPDACAL 04715u
#define SC_POSTJUMP 04701u
#define SC_SDISPLAY 02773u
#define SC_ENDSUMS 03011u
#define SC_TABLE cm_self_check_routines
#define SC_ENTRIES cm_self_check_entries
#include "self_check_body.inc"
