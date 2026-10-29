/* SPDX-License-Identifier: MIT */
#include "lm_admission_raw_cases.h"
#include "lm_admission_image_cases.h"
exec_address lm_admission_raw_input(apollo_overlay_state *m,p66_bias_io *io,uint16_t n){
 static const agc_word priorities[]={0,1,0377,01007,036077,037000};
 exec_address job=lm_admission_image_input(m,io,n);uint8_t kind=(uint8_t)(n%4u);
 agc_word priority=priorities[(n/4u)%6u],q=n&0100u?0155:0705;
 m->words[063]=priority;if(kind<2u)m->words[0]=priority;
 m->words[2]=q;if(kind<2u){m->words[q]=job.location;m->words[q+1u]=job.bank;}
 if(m->words[0167]==AGC_NEG_ZERO&&n%3u==0){m->words[067]=014;m->words[0203]=035110;}
 return job;
}
