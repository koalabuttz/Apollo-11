/* SPDX-License-Identifier: MIT */
/* Luminary099 AGC_BLOCK_TWO_SELF-CHECK (43,3232-3723): src/self_check_body.inc
 * with Luminary099's addresses and constants. */
#include "self_check.h"
#include "lm_self_check_operands.inc"
#define SC_MISSION 0u
#define SC_IO p66_bias_io
#define SC_D 2u
#define SCK(n) LM_SC_K_##n
#define SC_ALARM2 05571u
#define SC_CHANG1 05122u
#define SC_LEFT5 04331u
#define SC_SUPDACAL 04651u
#define SC_POSTJUMP 04635u
#define SC_SDISPLAY 03113u
#define SC_ENDSUMS 03131u
#define SC_TABLE lm_self_check_routines
#define SC_ENTRIES lm_self_check_entries
#define SC_ROPE_DATA lm_self_check_rope_data
#define SC_SITE_TABLE lm_self_check_site_table
#define SC_OPERANDS lm_sc_operands
#define SC_OPERAND_COUNT LM_SC_OPERAND_COUNT
#include "self_check_body.inc"
