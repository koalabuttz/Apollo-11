/* SPDX-License-Identifier: MIT */
#include "lm_spectest_handoff_cases.h"
#include "lm_admission_raw_cases.h"
#include "lm_wake_image_cases.h"
exec_address lm_spectest_handoff_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 uint8_t entry=(uint8_t)(n%6u),i;exec_address job={0,0};
 if(entry<4u){uint16_t input=(uint16_t)((n/6u)*4u+entry);job=lm_admission_raw_input(s,io,input);
  s->words[0167]=AGC_NEG_ZERO;s->words[067]=0;
  for(i=0;i<5u;++i)s->words[0400u+44u*i]=i<n%5u?0:(agc_word)(0400u+44u*i);
 }else if(entry==4u){lm_wake_image_input(s,io,(uint16_t)(8192u+n%8192u));s->words[067]=0;}
 else{job=lm_admission_raw_input(s,io,n);s->words[5]=02651;s->words[4]=02000;s->words[6]=(agc_word)(02000u|(s->words[3]>>8));
  s->words[067]=(n/6u)&1u?(agc_word)(040000u+n%037777u):0;io->interrupt_enabled=0;}
 return job;
}
