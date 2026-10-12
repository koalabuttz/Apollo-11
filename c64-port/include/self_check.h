/* SPDX-License-Identifier: MIT */
#ifndef SELF_CHECK_H
#define SELF_CHECK_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* AGC_BLOCK_TWO_SELF-CHECK (bank 43) in both missions, as native routines of
 * the control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md): the self-check
 * idle loop SMODECHK with its options, ERRORS/PRERRORS, ERASCHK (every
 * erasable word, with the ERESTORE repair), CNTRCHK, CYCLSHFT and
 * ROPECHK/STSHOSUM (the rope sum and verb 91's show-banksum), but FXADRS.
 * Entries are SELFCHK (the Executive's idle job), STSHOSUM and NXTBNK
 * (SHOWSUM in EXTENDED_VERBS) and the return point after each TC out of the
 * file (ALARM2, POSTJUMP ADVAN through SELFRET, SUPDACAL, CHANG1, LEFT5,
 * SDISPLAY). A run leaves at those routines, at ENDSUMS, or at FXADRS (the
 * read of the fixed-fixed rope, not translated). io is a p66_bias_io: its
 * interrupt-enable state takes the file's INHINT/RELINT, and its SUPERBNK
 * must select bank 43. Both missions share src/self_check_body.inc;
 * Luminary099's words sit two lower than Comanche055's. */
#define SELF_CHECK_ROUTINES 11u
extern const agc_routine_table lm_self_check_routines, cm_self_check_routines;
/* Entry addresses (bank 43), in the order of the routine tables. */
extern const uint16_t lm_self_check_entries[SELF_CHECK_ROUTINES], cm_self_check_entries[SELF_CHECK_ROUTINES];
#endif
