/* SPDX-License-Identifier: MIT */
/* Comanche055 AGC_BLOCK_TWO_SELF-CHECK (43,3234-3725): src/self_check_body.inc
 * with Comanche055's addresses and constants. */
#include "self_check.h"
/* Defines the agc_routine_table cm_self_check_routines (src/self_check_body.inc). */
#include "cm_self_check_operands.inc"
#define SC_MISSION 1u
#define SC_IO cm_native_io
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
#define SC_ROPE_DATA cm_self_check_rope_data
#define SC_SITE_TABLE cm_self_check_site_table
#define SC_OPERANDS cm_sc_operands
#define SC_OPERAND_COUNT CM_SC_OPERAND_COUNT
#include "self_check_body.inc"
