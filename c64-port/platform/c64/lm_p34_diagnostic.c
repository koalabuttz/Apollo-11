/* SPDX-License-Identifier: MIT */
/* lm-p34-transfer on the C64, against digests frozen from the original
 * program (tools/lm_p34_differential.py). Built three times, each linking
 * only the routines its cases reach:
 *   LM_P34_C64_GROUP=1  every 16th basic routine case (this file's basic
 *       routines and the native INTER-BANK routines);
 *   LM_P34_C64_GROUP=2  sampled CLUPDATE, VN1645 and SELECTMU cases whose run
 *       does not reach VN1645's CALL GET+MGA (VN1645 at 3606 and GET45,
 *       SELECTMU, CLUPDATE and GET45 +1);
 *   LM_P34_C64_GROUP=3  sampled GET.LVC cases (GET.LVC).
 * GET+MGA through the trampoline does not fit below the mailbox; its cases
 * run on the host (its phase bodies run on the C64 in lm-midgim).
 * Groups 2-3 build their entries from frozen orbit words
 * (LM_P34_FROZEN_INPUTS). Mailbox (tools/vice_run.py): $c001-2 failures,
 * $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_p34_cases.h"
#include "lm_p34_c64_vectors.h"
#if LM_P34_C64_GROUP!=1
#include "lm_p34_c64_inputs.h"
#endif
#if LM_P34_C64_GROUP==1
#include "lm_interbank_routines.h"
/* The INTER-BANK site table with this file's two TCR sites merged in. */
static agc_site_operand operands[448u+LM_P34_SITES];
#elif LM_P34_C64_GROUP==2
/* These cases leave the interpretive routines only at GET45 +1. */
static const agc_routine_entry get45[1]={{LM_P34_B35(03635),lm_p34_get45_comptgo}};
static const agc_routine_entry some[4]={{LM_P34_B10(02000),lm_p34_selectmu},{LM_P34_B35(03606),lm_p34_vn1645},
 {LM_P34_B35(03634),lm_p34_get45},{LM_P34_B35(03671),lm_p34_clupdate}};
static const agc_routine_table basic[1]={{get45,1}},interp[1]={{some,4}};
#else
static const agc_routine_entry some[1]={{LM_P34_B06(03733),lm_p34_getlvc}};
static const agc_routine_table interp[1]={{some,1}};
#endif
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,k;
 agc_native_program program={0,0,0,0,0,0,LM_P34_FETCH,0164,0165,0,0,0};
 /* Interrupts off and $01=$35: BASIC and KERNAL ROM banked out, so the
  * 4 KB image lives in the RAM under KERNAL at $e000. */
 apollo_overlay_state *image=(apollo_overlay_state *)0xe000u;
 __asm__("sei");*(volatile uint8_t *)1u=0x35u;
#if LM_P34_C64_GROUP==1
 agc_routine_table tables[2];uint16_t a,b,i=0,j=0,total=0;const agc_site_operand *x=lm_interbank_site_table(&a),*y=lm_p34_site_table(&b);
 if(a>448u)a=0;
 while(i<a||j<b)operands[total++]=(j>=b||(i<a&&x[i].physical<y[j].physical))?x[i++]:y[j++];
 tables[0]=lm_p34_basic_routines;tables[1]=lm_interbank_routines;program.basic=tables;program.basic_count=2;program.operands=operands;program.operand_count=total;
#elif LM_P34_C64_GROUP==2
 program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
#else
 program.interpretive=interp;program.interpretive_count=1;
#endif
#if LM_P34_C64_GROUP==1
 for(k=0;k<LM_P34_C64_BASIC;++k){++checks;if(lm_p34_digest(&program,image,(uint32_t)k*LM_P34_C64_STRIDE)!=lm_p34_c64_basic[k])++failures;}
#else
 for(k=0;k<LM_P34_C64_INPUTS;++k){uint32_t n=lm_p34_c64_input_case[k];
  ++checks;if(lm_p34_digest(&program,image,n)!=lm_p34_c64_interp[k])++failures;}
#endif
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
