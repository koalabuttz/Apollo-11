/* SPDX-License-Identifier: MIT */
#ifndef SELF_CHECK_CASES_H
#define SELF_CHECK_CASES_H
#include "self_check.h"
/* Each case (mission 0 Luminary099, 1 Comanche055) enters one entry of
 * AGC_BLOCK_TWO_SELF-CHECK (case n: entry n % 11) with FB 33 and SUPERBNK
 * selecting bank 43, and runs through the trampoline until a point the table
 * does not own (a routine outside the file, ENDSUMS, or FXADRS). The image is
 * pseudo-random within the program's domain: SMODE +0, -0, an option +-1-8
 * or out of range; SKEEP1 and SFAIL a TC to where the file continues there
 * (ERASCHK or ROPECHK; a -1CHK return) or out of the file; ERASCHK's address
 * SKEEP7 an erasable pair at or above 061 after the EB window, SKEEP3 its
 * limit (often the next word), SKEEP2 and SKEEP4 (EB) any bank step; ERESTORE
 * +0 or SKEEP7; ROPECHK's SKEEP3 address, often a bank's last, SKEEP4 a bank
 * (often the last, or one before a bank-numbering step), SKEEP5/SKEEP6/SKEEP7
 * +-1, +-0; NEWJOB mostly +0; EB, the interrupt-enable state and the rest
 * pseudo-random. tools/self_check_reference.c runs the same states in the
 * original program until self_check_leaves(). */
#define SELF_CHECK_CASES 4096u
/* The mission's device record: p66_bias_io (0) or cm_native_io (1). */
typedef union { p66_bias_io lm; cm_native_io cm; } self_check_io;
agc_word *self_check_superbank(unsigned mission,self_check_io *);
agc_word *self_check_interrupts(unsigned mission,self_check_io *);
/* The mission's SELF_CHECK_TABLES routine tables: the idle loop, then the rest. */
#define SELF_CHECK_TABLES 2u
const agc_routine_table *self_check_table(unsigned mission);
const uint16_t *self_check_entry_list(unsigned mission);
const agc_site_operand *self_check_site_table(unsigned mission,uint16_t *count);
void self_check_input(unsigned mission,apollo_overlay_state *,self_check_io *,uint16_t n);
/* 1 when the original's Z (at an instruction boundary, with FB and
 * SUPERBNK) is outside what the native table owns. */
int self_check_leaves(unsigned mission,agc_word z,agc_word fb,agc_word superbank);
enum exec_status self_check_run(unsigned mission,apollo_overlay_state *,self_check_io *,uint16_t n,uint16_t *steps);
/* Digest in the oracle's order (tools/self_check_freeze.py). */
uint32_t self_check_digest(unsigned mission,apollo_overlay_state *,uint16_t n,enum exec_status *);
/* Whole-rope runs: case n of SELF_CHECK_ROPE_CASES enters SELFCHK with SMODE 5
 * (ROPECHK) and NEWJOB -0 (0), a change job waiting (1) or a job in position
 * (2), STSHOSUM with SKEEP6 -0, +0, +1 or -1 (3-6), or SHOWSUM's STSHOSUM
 * (SKEEP6 +1) through n - 6 displays (7-42: at SDISPLAY, its two LXCH and
 * TC NXTBNK as after PROCEED; verb 91's display itself is the DSKY lane's),
 * and runs through the
 * self-check table, INTER-BANK_COMMUNICATION (SUPDACAL with the rope image,
 * POSTJUMP) and stand-ins for ADVAN and LEFT5 to an unmapped point: ENDSUMS,
 * SDISPLAY, NUCHANG2, NUDIRECT, CHANG1 or ALARM2 (self_check_rope_ends).
 * Needs the rope image (include/rope_image.h). */
#define SELF_CHECK_ROPE_CASES 43u
#define SELF_CHECK_ROPE_STEPS UINT32_C(2000000)
void self_check_rope_input(unsigned mission,apollo_overlay_state *,self_check_io *,uint16_t n);
enum exec_status self_check_rope_run(unsigned mission,apollo_overlay_state *,self_check_io *,uint16_t n,uint32_t *steps);
uint32_t self_check_rope_digest(unsigned mission,apollo_overlay_state *,uint16_t n,enum exec_status *);
const agc_word *self_check_rope_ends(unsigned mission,unsigned *count);
unsigned self_check_rope_proceeds(uint16_t n);
void self_check_showsum_proceed(unsigned mission,apollo_overlay_state *);
#endif
