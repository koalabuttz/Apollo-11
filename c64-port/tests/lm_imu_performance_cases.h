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
#define IPT_STATES 512u
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
/* C64 parts: the interpretive order-pair words (IPT_IN, by LOC) and basic
 * blocks (IPT_BASIC_IN, by first address) each diagnostic links; a run
 * reaching anything else ends as kind 3. The ALFLT/NORMLOP loop
 * (02605-02772) with its vector and matrix orders does not fit beside the
 * rest in C64 RAM, so cases reaching it are checked on the host only. */
#if !defined IPT_C64_PART
#define IPT_IN(loc) 1
#elif IPT_C64_PART==0
/* Everything, with the parts' jump dispatch (host equivalence check). */
#define IPT_IN(loc) 1
#define IPT_BASIC_IN(a) 1
#elif IPT_C64_PART==99
/* IPT_IN and IPT_BASIC_IN given on the command line. */
#elif IPT_C64_PART==1
/* All basic code, no interpretive code. */
#define IPT_IN(loc) 0
#define IPT_BASIC_IN(a) 1
#elif IPT_C64_PART==2
/* LATAZCHK through POSGMBL (TEST_2's first interpretive code) and the
 * basic code around it. */
#define IPT_IN(loc) ((loc)<02100u)
#define IPT_BASIC_IN(a) ((a)==02004u||(a)==02027u||(a)==02067u)
#elif IPT_C64_PART==3
/* SGNAGREE through the drift tests (02200-02377) and the basic blocks
 * their cases reach. */
#define IPT_IN(loc) ((loc)>=02200u&&(loc)<02400u)
#define IPT_BASIC_IN(a) ((a)==02166u||(a)==02226u||(a)==02301u||(a)==02332u||(a)==02457u||(a)==04102u)
#elif IPT_C64_PART==4
/* ERTHRVSE, ESTIMS's interpretive code and SETUPER1/GEOSTRT4. */
#define IPT_IN(loc) (((loc)>=02400u&&(loc)<02414u)||((loc)>=02500u&&(loc)<02605u)||(loc)>=03000u)
#define IPT_BASIC_IN(a) ((a)==02367u||(a)==02447u||(a)==02500u||(a)==02773u||(a)==03034u)
#elif IPT_C64_PART==5
/* EARTHR and its callers (02414-02477). */
#define IPT_IN(loc) ((loc)>=02414u&&(loc)<02500u)
#define IPT_BASIC_IN(a) ((a)==02126u||(a)==02166u||(a)==02226u||(a)==02332u||(a)==02447u||(a)==02457u||(a)==04102u)
#endif
void ipt_input(apollo_overlay_state *, ipt_case *, uint16_t n);
/* Where both programs stop: 1 when the original leaves the file and
 * interpreter at this instruction boundary (Z under FB/superbank; LOC under
 * BANKSET at NEWOPS). */
int ipt_leaves(agc_word z, agc_word fb, agc_word superbank, agc_word loc, agc_word bankset);
/* Runs case n natively; returns the kind (3: anything else, such as a
 * case left out of this C64 part) and the final
 * interrupt-enable state. */
unsigned ipt_run(apollo_overlay_state *, uint16_t n, uint16_t *interrupt_enabled);
uint32_t ipt_digest(apollo_overlay_state *, uint16_t n);
#endif
