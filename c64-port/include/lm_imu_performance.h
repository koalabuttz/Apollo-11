/* SPDX-License-Identifier: MIT */
#ifndef LM_IMU_PERFORMANCE_H
#define LM_IMU_PERFORMANCE_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 IMU_PERFORMANCE_TEST_2 and IMU_PERFORMANCE_TESTS_4 (P07, the
 * IMU performance test: bank 37 02002-03112 and FINETIME at 04102-04123)
 * as native routines of the control-transfer ABI. See
 * docs/LM-IMU-PERFORMANCE.md.
 *
 * Every basic instruction is one ai_* effect on the image in source order;
 * every interpretive order pair is the compiled lm_interp_steps /
 * lm_interp_orders path for its words (SIN and COS are the original bank-0
 * bodies of src/sband_trig.c). A routine starts at its key (Z, or LOC at
 * NEWOPS 06070 for an interpretive key) and runs the file's code, local
 * subroutines, the erasable returns it executes (TC Q, TC QPLACE, TC
 * QPLACES, TC ZERONDX) and the interpreter, until control leaves: a basic
 * address of another file, NEWOPS with LOC outside the file (CALL or
 * STCALL to CALCGA, an RVQ or GOTO to a caller elsewhere), the target of
 * an RTB (1STO2S, CDULOGIC, SGNAGREE, LOADTIME, PULSEIMU), CHANG2 at a
 * DANZIG with NEWJOB positive, or an erasable Z other than those returns
 * (TC ATIGINC).
 *
 * Keys: every entry and resume point. Basic: the files' entries, the
 * returns from outside calls, 02027 (1STO2S's EXIT), the hardware events
 * 02354 (CHECKG after a PIPA pulse moved PIPAX between its two reads) and
 * 04112 (FINETIME after LOSCALAR moved between READ and RXOR), and the two
 * idle points below. Interpretive: every order-pair and store word, since
 * each can follow a DANZIG, where a job switch (CHANG2) suspends the job
 * and the Executive resumes it at NEWOPS with LOC there. Each table is
 * sorted by key: FINETIME (fixed-fixed) sorts first.
 *
 * Idle points: the original spins there until hardware moves an input, so
 * the run ends there as a continuation (EXEC_OK) and the key resumes it:
 * CHECKG1 (02335) after the BZF at 02352 found PIPAX unchanged, and
 * FINETIME +1 (04103) after the BZF at 04117 found LOSCALAR 37777.
 *
 * io is lm_ipt_io; INHINT and RELINT set control.interrupt_enabled, READ
 * LOSCALAR and READ HISCALAR (FINETIME) read the frozen scaler words.
 *
 * Refused (EXEC_OUT_OF_SCOPE): states the original does not produce, or
 * that the compiled interpreter orders do not follow:
 *   - Z or LOC not one of the file's instruction or order-pair words (a
 *     CCS taking a slot that holds data or another routine's words);
 *   - an INDEX whose sum leaves the erasable words the instruction names
 *     (POSITON, PIPINDEX, RESULTCT, L in ZEROING; NDXCTR's computed TC
 *     beyond bank 37);
 *   - a TC Q or erasable TC whose word is not a TC to fixed memory;
 *   - NEWJOB negative at a DANZIG;
 *   - an RVQ whose QPRET is not a fixed CADR, a fixed operand outside the
 *     constants the code names (VXSC GEORGEJ with a scalar in MPAC names a
 *     vector there), and SIN/COS outside src/sband_trig.c's domain;
 *   - an indexed operand or store with X1 or X2 outside +-0377 (only a
 *     store through another wild index leaves one there);
 *   - AINGOTN's DDV with a divisor whose major part is +-0 once its words
 *     agree in sign (BUFZERO, which the compiled DDV does not follow);
 *   - an interpretive order with FIXLOC not one of the five VAC areas or
 *     PUSHLOC outside FIXLOC..FIXLOC+42, or EDOP not +0;
 *   - no undo image (io->undo NULL), or more than 4000 steps.
 * A refusal can come after the run has started; the image and io are then
 * restored from io->undo, which the caller supplies (a full image: on the
 * C64 it lives outside the overlay slot). */
typedef struct {
 p66_bias_io control;
 agc_word hiscalar,loscalar;   /* input channels 3 and 4 */
 apollo_overlay_state *undo;   /* scratch for the entry image */
} lm_ipt_io;
extern const agc_routine_table lm_imu_performance_routines;
extern const agc_routine_table lm_imu_performance_interpretive;
#define IPT_BASIC_ROUTINES 43u
#define IPT_INTERPRETIVE_ROUTINES 125u
#define IPT_ROUTINES (IPT_BASIC_ROUTINES+IPT_INTERPRETIVE_ROUTINES)
/* Physical keys, basic then interpretive, in table order. */
extern const uint16_t ipt_routine_keys[IPT_ROUTINES];
#endif
