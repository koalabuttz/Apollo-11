/* SPDX-License-Identifier: MIT */
#include "resident_rupt_cases.h"
/* One rnd call per full expression: C leaves the order of two calls in one
 * expression unspecified, and the cases must be the same under every
 * compiler (gcc for yaAGC, clang and llvm-mos for the checks). */
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
uint32_t rr_mix(uint32_t h,uint16_t v){h=(h^(v&0xffu))*16777619u;return (h^(v>>8))*16777619u;}
uint32_t rr_fold(uint32_t h,uint32_t digest){return rr_mix(rr_mix(h,(uint16_t)digest),(uint16_t)(digest>>16));}
void rr_entry_input(unsigned mission,uint16_t n,rr_rope rope,rr_case *c){
 uint32_t r=0x52555054u+mission*0x9e3779b9u+n*2654435761u;unsigned i;agc_word fb,eb,z,sb,w;
 for(i=0;i<2048u;++i)c->s->words[i]=(agc_word)(rnd(&r)&AGC_MASK);
 c->s->words[7]=0;
 c->s->words[0]=agc_extend(c->s->words[0]);
 if(rnd(&r)%4u==0u)c->s->words[0]^=0100000u;               /* overflow in A */
 c->s->words[1]=agc_extend(c->s->words[1]);c->s->words[2]=agc_extend(c->s->words[2]);
 for(;;){fb=(agc_word)(rnd(&r)%32u);sb=(agc_word)(rnd(&r)%3u==0u?0100u:rnd(&r)%2u?060u:0u);
  if(!(fb>=030u&&(sb&0100u)&&fb+8u>043u))break;}
 eb=(agc_word)(rnd(&r)%8u);
 c->s->words[3]=(agc_word)(eb<<8);c->s->words[4]=(agc_word)(fb<<10);c->s->words[6]=(agc_word)((fb<<10)|eb);
 if(rnd(&r)%8u==0u){z=(agc_word)(060u+rnd(&r)%(02000u-060u));
  if(rnd(&r)%4u==0u){static const agc_word ops[3]={3,4,6};
   c->s->words[z<01400u?z:(uint16_t)(eb*0400u+(z&0377u))]=ops[rnd(&r)%3u];}}
 else{z=(agc_word)(02000u+rnd(&r)%06000u);
  if(rope&&rnd(&r)%8u==0u)                                  /* walk to a RELINT, INHINT or EXTEND */
   for(i=0;i<06000u;++i,z=(agc_word)(z==07777u?02000u:z+1u)){
    w=rope(mission,agc_fixed_physical(z,c->s->words[4],sb));if(w==3u||w==4u||w==6u)break;}}
 c->s->words[5]=z;
 c->superbank=sb;c->interrupt_enabled=(agc_word)(rnd(&r)%8u!=0u);c->in_interrupt=(uint16_t)(rnd(&r)%8u==0u);
 if(rnd(&r)%16u==0u)c->pending=0;
 else{uint32_t a=rnd(&r);c->pending=(uint16_t)((a&rnd(&r))&03776u);}
}
void rr_t3_input(unsigned mission,uint16_t n,rr_case *c){uint32_t r=0x54335255u+mission*0x9e3779b9u+n*2654435761u;unsigned i,k;agc_word z,w;
 rr_entry_input(mission,n,0,c);
 c->s->words[0]&=0137777u;c->s->words[0]=agc_extend(agc_correct(c->s->words[0]));
 z=(agc_word)(060u+rnd(&r)%(01400u-060u));
 do{w=(agc_word)(rnd(&r)&AGC_MASK);}while(w==3u||w==4u||w==6u);
 c->s->words[z]=w;c->s->words[5]=z;
 /* The Waitlist as WAITLIST keeps it: TIME3 just past its overflow, k
  * tasks with LST1 = -(dt)+1, the free slots at NEG1/2. */
 c->s->words[026]=(agc_word)(rnd(&r)%8u);
 k=1u+rnd(&r)%8u;
 for(i=0;i<8u;++i){uint32_t span=rnd(&r)%2u?4u:16383u,dt=1u+rnd(&r)%span;
  c->s->words[01400u+i]=i<k?agc_from_int((int16_t)(1-(int32_t)dt)):057777u;}
 for(i=0;i<9u;++i){agc_word bb=(agc_word)((rnd(&r)%32u)*02000u);bb|=(agc_word)((rnd(&r)%8u)*020u);bb|=(agc_word)(rnd(&r)%8u);
  c->s->words[01410u+2u*i]=mission?05324u:05261u;c->s->words[01411u+2u*i]=bb;}
 c->interrupt_enabled=1;c->in_interrupt=0;c->pending=1u<<3;
}
void rr_timer_input(uint32_t n,agc_word time[6],agc_word *channel13){uint32_t r=0x54494d45u+n*2654435761u;unsigned i;
 static const agc_word edge[8]={037777,037776,037770,0,077777,1,077776,040000};
 if(n<32768u){for(i=0;i<6u;++i)time[i]=(agc_word)n;*channel13=040000u;return;}
 for(i=0;i<6u;++i)time[i]=(agc_word)(rnd(&r)%2u?edge[rnd(&r)%8u]:i==5u&&rnd(&r)%2u?rnd(&r)%040u:rnd(&r)&AGC_MASK);
 *channel13=(agc_word)(rnd(&r)%4u?040000u:0u);
}
