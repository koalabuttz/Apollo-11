/* SPDX-License-Identifier: MIT */
#ifndef SELF_CHECK_H
#define SELF_CHECK_H
#include "agc_transfer.h"
#include "p66_bias.h"
#include "cm_native_image.h"
/* AGC_BLOCK_TWO_SELF-CHECK (bank 43) in both missions, as native routines of
 * the control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md): the self-check
 * idle loop SMODECHK with its options, ERRORS/PRERRORS, ERASCHK (every
 * erasable word, with the ERESTORE repair), CNTRCHK, CYCLSHFT and
 * ROPECHK/STSHOSUM (the rope sum and verb 91's show-banksum), but FXADRS.
 * Entries are SELFCHK (the Executive's idle job), STSHOSUM and NXTBNK
 * (SHOWSUM in EXTENDED_VERBS) and the return point after each TC out of the
 * file (ALARM2, POSTJUMP ADVAN through SELFRET, SUPDACAL, CHANG1, LEFT5,
 * SDISPLAY). A run leaves at those routines or at ENDSUMS. FXADRS reads
 * the fixed-fixed rope through include/rope_image.h, and a program running
 * ROPECHK gives SUPDACAL the same image as its fixed_data hook
 * ({lm,cm}_self_check_rope_data). io is the mission's device record
 * (p66_bias_io for Luminary099, cm_native_io for Comanche055, as the
 * INTER-BANK_COMMUNICATION routines take): its interrupt-enable state takes
 * the file's INHINT/RELINT, and its SUPERBNK must select bank 43. Both
 * missions share src/self_check_body.inc; Luminary099's words sit two lower
 * than Comanche055's. */
#define SELF_CHECK_ROUTINES 17u
/* Two routine tables per mission: the idle loop (SELFCHK and the return
 * through SELFRET, with the loop's words 3303, 3305, 3310, 3334 and 3336
 * as entries,
 * src/{lm,cm}_self_check_idle.c), which the flight store
 * loads alone, and the rest of the file, with the three words the idle loop
 * continues at after CCS SMODE (3307, 3311 and 3312, Comanche055 numbering)
 * and the loop's TC SKEEP1 (3313) as entries. A program registers both. */
#define SELF_CHECK_IDLE_ROUTINES 7u
#define SELF_CHECK_MAIN_ROUTINES 13u
extern const agc_routine_table lm_self_check_routines, cm_self_check_routines;
extern const agc_routine_table lm_self_check_idle_routines, cm_self_check_idle_routines;
extern const agc_routine_table lm_self_check_idle_hot_routines;
/* The idle table's one inline operand (CHECKNJ's CADR ADVAN). */
extern const agc_site_operand lm_self_check_idle_sites[1], cm_self_check_idle_sites[1];
/* The file's entry addresses (bank 43) as callers reach them, both tables'
 * entries but 3307, 3311 and 3312, ascending. */
extern const uint16_t lm_self_check_entries[SELF_CHECK_ROUTINES], cm_self_check_entries[SELF_CHECK_ROUTINES];
const agc_site_operand *lm_self_check_site_table(uint16_t *count);
const agc_site_operand *cm_self_check_site_table(uint16_t *count);
bool lm_self_check_rope_data(const agc_native_program *,uint16_t physical,agc_word *word);
bool cm_self_check_rope_data(const agc_native_program *,uint16_t physical,agc_word *word);
#endif
