/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_basic_finish_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_basic_finish_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){unsigned i,local=n%8192u;uint32_t seed=0x05217055u+n;
 static const agc_word base[8]={0,0110,0401,0455,0531,0605,0661,1};static const agc_register q[8]={0,0177777,1,0177776,040000,0137777,077777,0100000};
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>11)&AGC_MASK);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 memset(io,0,sizeof *io);io->superbank=(agc_word)((n&7u)<<4);io->interrupt_enabled=(agc_word)(n&1u);io->channel11=(agc_word)((n*977u)&AGC_MASK);
 s->words[3]=(agc_word)((n&7u)<<8);s->words[4]=(agc_word)(((n>>3)&31u)<<10);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=05217;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);s->words[2]=q[n%8u];
 s->words[0167]=(agc_word)(022000u+base[(n/128u)%8u]);s->words[0166]=0110;s->words[0165]=s->words[6];s->words[0121]=(agc_word)(n%4u==3u?AGC_NEG_ZERO:n%3u);
 for(i=1;i<7u;++i){unsigned a=0154u+i*12u;
  s->words[a+8u]=(agc_word)(02000u+i);s->words[a+9u]=(agc_word)(02000u*((i+n)%32u)|((i+n)%8u)|(((i+n)%8u)*020u));
  s->words[a+10u]=n%3u?0110:077667;s->words[a+11u]=(agc_word)(01000u*(((uint32_t)n*13u+(uint32_t)i*7u)%31u+1u)+i*17u);
  if((n+i)%5u==0u)s->words[a+11u]=AGC_NEG_ZERO;else if((n+i)%5u==1u)s->words[a+11u]=agc_negate(s->words[a+11u]);
 }
 if(n%4u==0u)for(i=1;i<7u;++i)s->words[0167u+12u*i]=AGC_NEG_ZERO;
 if(local<128u){unsigned tiny=local%6u+1u,larger=tiny%6u+1u;for(i=1;i<7u;++i)s->words[0167u+12u*i]=AGC_NEG_ZERO;s->words[0167u+12u*tiny]=1;if(local&8u)s->words[0167u+12u*larger]=2;}
 for(i=1;i<7u;++i)if(n>=8192u&&i%2u==n%2u)s->words[0164u+12u*i]=agc_negate((agc_word)(03360u+i));
}
