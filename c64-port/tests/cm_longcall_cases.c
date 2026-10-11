/* SPDX-License-Identifier: MIT */
#include "cm_longcall_cases.h"
#include "agc_word.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_longcall_input(apollo_overlay_state *s,exec_address *task,uint16_t n){
 unsigned i,row=n%1024u;uint32_t seed=0x1c055u+n;
 static const agc_word times[]={0,1,2,017777,020000,020001,037776,037777};
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>11)&AGC_MASK);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[3]=01400;s->words[4]=02000;s->words[6]=02003;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 task->location=(agc_word)(02000u+(row&01777u));task->bank=(agc_word)(((row%32u)<<10)|((row>>5)&7u)|060u);
 s->words[01100]=task->location;s->words[01101]=task->bank;
 s->words[01434]=(agc_word)(01200u+row);s->words[01435]=(agc_word)((((row+5u)%32u)<<10)|((row>>5)&7u));
 s->words[01133]=task->location;s->words[01134]=task->bank;
 if(n<2048u){agc_word high=row<64u?times[row/8u]:(agc_word)(s->words[01137]&037777u);agc_word low=row<64u?times[row%8u]:(agc_word)(s->words[01140]&037777u);
  if(n>=1024u&&row>=64u&&high>0)low=s->words[01140];
  s->words[01137]=high;s->words[01140]=low;s->words[0]=agc_extend(high);s->words[1]=agc_extend(low);
 }
 if(n<1024u){s->words[6]=(agc_word)((row%32u)<<10|(row&7u));s->words[3]=(agc_word)((row&7u)<<8);s->words[4]=(agc_word)((row%32u)<<10);s->words[2]=01100;s->words[5]=05342;}
 else s->words[5]=n<2048u?03465:n<3072u?03505:03515;
}
