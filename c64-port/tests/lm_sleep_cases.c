/* SPDX-License-Identifier: MIT */
#include "lm_sleep_cases.h"
#include "lm_priority_cases.h"
void lm_sleep_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel,uint16_t n){uint8_t i;
 lm_priority_input(s,io,n);s->words[0]=agc_extend((agc_word)(((uint32_t)n*13u)&077777u));s->words[5]=05133;
 if(n%4u==0u)for(i=1;i<8u;++i)s->words[0167u+i*12u]=AGC_NEG_ZERO;
 if(n<128u){uint8_t tiny=(uint8_t)(n%7u+1u),larger=(uint8_t)(tiny%7u+1u);for(i=1;i<8u;++i)s->words[0167u+12u*i]=AGC_NEG_ZERO;s->words[0167u+12u*tiny]=1;if(n&8u)s->words[0167u+12u*larger]=2;}
 *channel=(agc_word)(((uint32_t)n*977u)&AGC_MASK);
}
