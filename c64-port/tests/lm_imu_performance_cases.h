/* SPDX-License-Identifier: MIT */
#ifndef LM_IMU_PERFORMANCE_CASES_H
#define LM_IMU_PERFORMANCE_CASES_H
#include "lm_imu_performance.h"
/* lm-imu-performance cases: every routine of lm_imu_performance_routines
 * and lm_imu_performance_interpretive, IPT_STATES states each, run from its
 * key until control leaves the file (include/lm_imu_performance.h).
 * tools/lm_imu_performance_reference.c runs the same image, interrupt
 * state and scaler words in the unmodified rope from the same Z and stops
 * where ipt_leaves() says; the 2048 erasable words and the interrupt-enable
 * state are compared. States are drawn inside the routines' domain; the
 * refusals are exercised by tests/test_lm_imu_performance.c. */
#define IPT_STATES 384u
#define IPT_CASES (IPT_ROUTINES*IPT_STATES)
#define IPT_STOPPED 0u
#define IPT_REFUSED 2u
typedef struct {
 agc_word superbank;
 uint16_t interrupt_enabled;
 agc_word hiscalar,loscalar;
} ipt_case;
/* The QPRET each genuine CALL stores (CALL ERTHRVSE at 02532 and 03030,
 * CALL EARTHR at 02452): checked against lmi_call by the unit test. */
#define IPT_QPRET_ERTHRVSE_2532 076534u
#define IPT_QPRET_ERTHRVSE_3030 077032u
#define IPT_QPRET_EARTHR_2452 076454u
/* C64 parts: the interpretive order-pair words (IPT_P_IN, by LOC) and basic
 * blocks (IPT_P_BASIC_IN, by first address, ipt_block()) part p links. A
 * part's run stops where control reaches a registered key the part leaves
 * out (a chained step: the key's own cases continue from there in the part
 * that has it); reaching other code left out ends as kind 3. Part 1 is
 * the basic code with SETUPER1 and GEOSTRT4 (03000 on). The ALFLT
 * loop is split at ALKLP (02676; no loop crosses it): parts 6 and 7. Part
 * 0 links everything and takes the part from ipt_part at run time (the
 * host's replay of every part's rows). */
#define IPT_PARTS 7u
#define IPT_P_IN(p,loc) ((p)==1u?(loc)>=03000u: \
 (p)==2u?(loc)<02100u: \
 (p)==3u?((loc)>=02200u&&(loc)<02400u): \
 (p)==4u?(((loc)>=02400u&&(loc)<02414u)||((loc)>=02500u&&(loc)<02605u)): \
 (p)==5u?((loc)>=02414u&&(loc)<02500u): \
 (p)==6u?((loc)>=02605u&&(loc)<02676u): \
 (p)==7u?((loc)>=02676u&&(loc)<03000u):1)
#define IPT_P_BASIC_IN(p,a) ((p)==1u?1: \
 (p)==2u?((a)==02004u||(a)==02027u||(a)==02067u): \
 (p)==3u?((a)==02166u||(a)==02226u||(a)==02301u||(a)==02332u||(a)==02447u||(a)==02457u||(a)==04102u): \
 (p)==4u?((a)==02367u||(a)==02447u||(a)==02500u||(a)==02773u||(a)==03034u): \
 (p)==5u?((a)==02126u||(a)==02166u||(a)==02226u||(a)==02301u||(a)==02332u||(a)==02447u||(a)==02457u||(a)==04102u): \
 (p)==6u?((a)==02577u): \
 (p)==7u?((a)==02773u):1)
#if defined IPT_C64_PART && IPT_C64_PART>0
#define ipt_part ((unsigned)IPT_C64_PART)
#define IPT_BASIC_IN(a) IPT_P_BASIC_IN(IPT_C64_PART,a)
#define IPT_IN(loc) IPT_P_IN(IPT_C64_PART,loc)
#else
#define IPT_BASIC_IN(a) 1
extern unsigned ipt_part;   /* 0: everything */
/* The run-time forms (functions, so a coverage build of the native file
 * sees one branch per test). */
#define IPT_IN(loc) ipt_in(loc)
int ipt_in(agc_word loc);
int ipt_basic_in(agc_word a);
#endif
/* The first address of the basic block holding a (an #if IPT_BASIC_IN
 * block of src/lm_imu_performance.c). */
agc_word ipt_block(agc_word a);
/* 1 when physical key ph is one of ipt_routine_keys, and when part
 * ipt_part links its code. */
int ipt_is_key(uint16_t ph);
int ipt_part_has(uint16_t ph);
void ipt_input(apollo_overlay_state *, ipt_case *, uint16_t n);
/* Where both programs stop: 1 when the original leaves the file and
 * interpreter at this instruction boundary (Z under FB/superbank; LOC under
 * BANKSET at NEWOPS). */
int ipt_leaves(agc_word z, agc_word fb, agc_word superbank, agc_word loc, agc_word bankset);
/* The two idle points where the native routine stops as a continuation
 * (the original spins there until hardware moves an input): CHECKG1
 * (02335) after the BZF at 02352 found PIPAX unchanged, and FINETIME +1
 * (04103) after the BZF at 04117 found LOSCALAR 37777. prev is the last
 * instruction boundary before z. */
int ipt_idle(agc_word z, agc_word prev);
/* Runs case n natively; returns the kind (3: anything else, such as a
 * case left out of this C64 part) and the final
 * interrupt-enable state. */
unsigned ipt_run(apollo_overlay_state *, uint16_t n, uint16_t *interrupt_enabled);
uint32_t ipt_digest(apollo_overlay_state *, uint16_t n);
#endif
