/* SPDX-License-Identifier: MIT */
/* Native Luminary099 CONIC_SUBROUTINES CHECKCTR (bank 12), as RTB runs it.
 * Each instruction below is the original's, on the canonical image. */
#include "lm_conic_checkctr.h"
#include "agc_image_ops.h"
#include "rtb_angles.h"
#define W s->words
uint8_t lm_conic_checkctr(apollo_overlay_state *s){
 unsigned a,i;
 if(!s||W[5]!=02644u||(W[4]&076000u)!=024000u||W[7])return RTB_OUT_OF_SCOPE;
 a=W[0120]+026u;   /* INDEX FIXLOC: ITERCTR (22D) in the job's VAC area */
 if(a<010u||a>=01400u)return RTB_OUT_OF_SCOPE;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return RTB_OUT_OF_SCOPE;
 /* CS ONE; AD ITERCTR: -1 overflows only from -16383 (40000), where TS
  * ITERCTR skips the TS MPAC (A then -1). */
 ai_csf(s,1);ai_ad(s,a);
 if(!ai_ts(s,a))(void)ai_ts(s,0154);
 ai_tc(s,02653);W[5]=06061;   /* TC DANZIG */
 return RTB_OK;
}
