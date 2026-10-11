/* SPDX-License-Identifier: MIT */
#include "lm_waitlist_image_cases.h"
exec_address lm_waitlist_image_input(apollo_overlay_state *m,p66_bias_io *io,uint16_t n){uint16_t i;uint8_t count=(uint8_t)((n/2u)%10u);uint32_t seed=UINT32_C(0x995203)+n/20u;exec_address task={03263,064067};agc_word delay;
 for(i=0;i<2048u;++i){seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);m->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
 io->superbank=(agc_word)(((n/20u)%8u)*020u);io->interrupt_enabled=(uint16_t)((n/160u)%2u);
 m->words[7]=m->words[035]=m->words[036]=m->words[047]=0;for(i=050;i<=054u;++i)m->words[i]=0;
 m->words[3]=(agc_word)(((n/20u)%8u)*0400u);m->words[4]=(agc_word)(((n/160u)%32u)*02000u);m->words[6]=(agc_word)(m->words[4]|(m->words[3]>>8));m->words[5]=(n&1u)?05173:05203;m->words[2]=0705;m->words[1]=agc_extend(m->words[1]);
 delay=(agc_word)(((n/20u)*173u)%16250u+1u);if(n%17u==0)delay=0;else if(n%19u==0)delay=077777;else if(n%23u==0)delay=077676;m->words[0]=agc_extend(delay);
 m->words[026]=(agc_word)(((n/20u)*37u)&037777u);for(i=0;i<8u;++i)m->words[01400u+i]=(agc_word)(077777u-((n/20u+i)*71u)%6000u);
 for(i=0;i<9u;++i){m->words[01410u+2u*i]=i<count?(agc_word)(02000u+i):WL_END_LOCATION;m->words[01411u+2u*i]=i<count?(agc_word)(060007u|(i%8u)*020u):WL_END_BANK;}
 m->words[0705]=task.location;m->words[0706]=task.bank;return task;
}
