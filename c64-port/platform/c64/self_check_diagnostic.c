/* SPDX-License-Identifier: MIT */
/* self-check on the C64, one mission per image (-DSELF_CHECK_LM_ONLY:
 * Luminary099, -DSELF_CHECK_CM_ONLY: Comanche055): the outcome-chosen rows
 * of its AGC_BLOCK_TWO_SELF-CHECK cases (every 64th case plus a greedy cover
 * of the gcov branch arcs the host cases take, tools/a2_c64_rows.py) through
 * the trampoline, and the outcome-chosen whole-rope cases (ROPECHK over all 36 banks, SHOWSUM's
 * first display, the Executive exits), against digests frozen from the
 * original program (tools/self_check_freeze.py). Both ropes are in the REU
 * (build/self-check.reu, include/rope_image.h).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "self_check_cases.h"
#ifdef SELF_CHECK_LM_ONLY
#include "lm_self_check_c64_vectors.h"
#define MISSION 0u
#define ROWS LM_SELF_CHECK_C64_COUNT
#define ROW lm_self_check_c64_case
#define DIGEST lm_self_check_c64
#define ROPE_ROWS LM_SELF_CHECK_ROPE_C64_COUNT
#define ROPE_ROW lm_self_check_rope_c64_case
#define ROPE_DIGEST lm_self_check_rope_c64
#else
#include "cm_self_check_c64_vectors.h"
#define MISSION 1u
#define ROWS CM_SELF_CHECK_C64_COUNT
#define ROW cm_self_check_c64_case
#define DIGEST cm_self_check_c64
#define ROPE_ROWS CM_SELF_CHECK_ROPE_C64_COUNT
#define ROPE_ROW cm_self_check_rope_c64_case
#define ROPE_DIGEST cm_self_check_rope_c64
#endif
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;enum exec_status st;
 for(n=0;n<ROWS;++n){++checks;if(self_check_digest(MISSION,&image,ROW[n],&st)!=DIGEST[n]||st!=EXEC_OK)++failures;}
 for(n=0;n<ROPE_ROWS;++n){++checks;if(self_check_rope_digest(MISSION,&image,ROPE_ROW[n],&st)!=ROPE_DIGEST[n]||st!=EXEC_OK)++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
