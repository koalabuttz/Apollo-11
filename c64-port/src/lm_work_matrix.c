/* SPDX-License-Identifier: MIT */
/* See include/lm_work_matrix.h. */
#include "lm_work_matrix.h"
#include "lm_interp_steps.h"
#include "matrix_native.h"
#define W s->words
enum {MXV=07334};
uint8_t lm_work_matrix(apollo_overlay_state *s,agc_word word,uint8_t column){
 agc_word k[18];unsigned i;
 lmi_direct(s,word,(agc_word)(MXV+(column?3u:0u)));
 if(W[0116]>=2048u-18u)return 0;
 for(i=0;i<18u;++i)k[i]=W[W[0116]+i];
 for(i=0;i<4u;++i){
  uint16_t bias=W[0116]<01000u?01000u:0u;W[0116]+=bias;
  if(!matrix_native_phase(s,APOLLO_FIXED_LM,column,k,(uint8_t)i)){W[0116]-=bias;return 0;}
  W[0116]-=bias;if(i==1u||i==2u)W[0]-=bias;
 }
 return 1;
}
