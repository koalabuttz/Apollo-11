/* SPDX-License-Identifier: MIT */
#include "sband_dot.h"
#include "agc_multiword.h"
/* Additive original DOT caller qualification of the frozen raw vector math. */
static void load(apollo_overlay_state *m,unsigned at,uint8_t negate){m->words[0]=agc_extend(negate?agc_negate(m->words[at]):m->words[at]);m->words[1]=agc_extend(negate?agc_negate(m->words[at+1u]):m->words[at+1u]);}
static void exchange(apollo_overlay_state *m,unsigned at){agc_register a=m->words[0],l=m->words[1];load(m,at,0);m->words[at]=agc_correct(a);m->words[at+1u]=agc_correct(l);}
static void add_pair(apollo_overlay_state *m,unsigned at){agc_register_pair r={m->words[0],m->words[1]};agc_dp p={m->words[at],m->words[at+1u]};agc_double_add(&r,&p);m->words[at]=p.high;m->words[at+1u]=p.low;m->words[0]=r.a;m->words[1]=r.l;}
static void multiply(apollo_overlay_state *m,unsigned at){agc_register_pair r=agc_multiply(m->words[0],m->words[at]);m->words[0]=r.a;m->words[1]=r.l;}
static void swap_a(apollo_overlay_state *m,unsigned at){agc_register a=m->words[0];m->words[0]=agc_extend(m->words[at]);m->words[at]=agc_correct(a);}
static unsigned address(const apollo_overlay_state *m);
static void product(apollo_overlay_state *m,const agc_word *fixed,agc_word base){
 unsigned at=address(m),offset=(unsigned)(m->words[0116]-base);
 /* The major operand is fetched after the minor partial products. This
  * ordering matters when operands alias MPTEMP/MPAC/BUF. */
 m->words[0]=agc_extend(fixed?fixed[offset+1u]:m->words[at+1u]);m->words[0156]=agc_correct(m->words[0]);
 m->words[0]=0;swap_a(m,0155);m->words[0135]=agc_correct(m->words[0]);
 multiply(m,0156);swap_a(m,0156);multiply(m,0154);add_pair(m,0155);
 m->words[0]=agc_extend(fixed?fixed[offset]:m->words[at]);swap_a(m,0135);multiply(m,0135);add_pair(m,0155);
 swap_a(m,0154);multiply(m,0135);add_pair(m,0154);m->words[2]=3;
}
static unsigned address(const apollo_overlay_state *m){unsigned at=m->words[0116];return at<01400u?at:(unsigned)(m->words[3]+(at&0377u));}
static uint8_t valid(apollo_overlay_state *m,enum apollo_fixed_mission mission,uint16_t first,const agc_word *fixed){
 unsigned at,i;
 if(m==0||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return 0;

 if(m->words[5]!=first||m->words[0163]!=077776u||m->words[7]!=0||m->words[3]>03400u||(m->words[3]&0377u)||m->words[4]>076000u||(m->words[4]&01777u)||m->words[6]!=(uint16_t)(m->words[4]|(m->words[3]>>8)))return 0;
 if(fixed){
  if(m->words[0116]<02000u||m->words[0116]>07772u||(m->words[0116]>03772u&&m->words[0116]<04000u))return 0;
  for(i=0;i<6u;++i)if(fixed[i]>077777u)return 0;
 }else{
  if(m->words[0116]<030u||m->words[0116]>01772u)return 0;
  at=address(m);if(at<030u||at>03772u)return 0;
 }
 for(i=2;i<2048u;++i)if(m->words[i]>077777u)return 0;
 return 1;
}
static void add_address(apollo_overlay_state *m,agc_word amount){m->words[0]=agc_add_register(agc_extend(amount),agc_extend(m->words[0116]));m->words[0116]=agc_correct(m->words[0]);}
static void sum(apollo_overlay_state *m,unsigned at){m->words[0]=agc_add_register(m->words[0],agc_extend(m->words[at]));}
static void store(apollo_overlay_state *m,unsigned at){int ov=agc_overflow(m->words[0]);m->words[at]=agc_correct(m->words[0]);if(ov)m->words[0]=ov>0?1u:0177776u;}
static void overflow_first(apollo_overlay_state *m){
 agc_word limit=m->words[0]==1u?037777u:040000u;agc_register a;int ov;
 m->words[0130]=limit;a=agc_add_register(agc_extend(limit),m->words[0]==1u?1u:0177776u);
 a=agc_add_register(a,agc_extend(m->words[0155]));m->words[0155]=agc_correct(a);ov=agc_overflow(a);a=ov?(ov>0?1u:0177776u):0;
 a=agc_add_register(a,agc_extend(limit));a=agc_add_register(a,agc_extend(m->words[0154]));m->words[0154]=agc_correct(a);ov=agc_overflow(a);
 if(ov){a=ov>0?1u:0177776u;m->words[0121]=agc_correct(a);}m->words[0]=a;
}
static void dot(apollo_overlay_state *m,uint16_t ret,const agc_word *fixed,agc_word base){
 m->words[0136]=2;m->words[0137]=ret;
 product(m,fixed,base);exchange(m,0157);exchange(m,0154);exchange(m,0130);
 m->words[0]=agc_extend(m->words[0156]);m->words[0132]=agc_correct(m->words[0]);add_address(m,m->words[0136]);
 product(m,fixed,base);exchange(m,0155);add_pair(m,0131);sum(m,0154);sum(m,0130);
 {int ov=agc_overflow(m->words[0]);store(m,0130);if(ov)m->words[0121]=agc_correct(m->words[0]);}
 exchange(m,0161);exchange(m,0154);add_address(m,m->words[0136]);product(m,fixed,base);
 exchange(m,0131);add_pair(m,0155);sum(m,0154);sum(m,0130);
 {int ov=agc_overflow(m->words[0]);store(m,0154);if(ov)overflow_first(m);}
 m->words[2]=0140;
}

uint8_t sband_dot(apollo_overlay_state *m,enum apollo_fixed_mission mission,const agc_word *fixed){uint16_t first=mission==APOLLO_FIXED_LM?07331u:07300u;agc_word base=m?m->words[0116]:0;
 if(!valid(m,mission,first,fixed))return 0;
 dot(m,(uint16_t)(first+1u),fixed,base);m->words[0]=0;m->words[0163]=0;m->words[5]=mission==APOLLO_FIXED_LM?06061u:06030u;return 1;
}
