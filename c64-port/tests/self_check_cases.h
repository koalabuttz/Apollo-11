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
const agc_routine_table *self_check_table(unsigned mission);
const uint16_t *self_check_entry_list(unsigned mission);
void self_check_input(unsigned mission,apollo_overlay_state *,p66_bias_io *,uint16_t n);
/* 1 when the original's Z (at an instruction boundary, with FB and
 * SUPERBNK) is outside what the native table owns. */
int self_check_leaves(unsigned mission,agc_word z,agc_word fb,agc_word superbank);
enum exec_status self_check_run(unsigned mission,apollo_overlay_state *,p66_bias_io *,uint16_t n,uint16_t *steps);
/* Digest in the oracle's order (tools/self_check_freeze.py). */
uint32_t self_check_digest(unsigned mission,apollo_overlay_state *,uint16_t n,enum exec_status *);
#endif
