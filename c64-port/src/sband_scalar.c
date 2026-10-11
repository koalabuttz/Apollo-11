/* SPDX-License-Identifier: MIT */
#include "sband_scalar.h"
#include "agc_multiword.h"
/* Native raw arithmetic semantics reused from the independently source-
 * qualified vector kernels. New actual scalar entry/control proof is separate.
 * Live operand major is read after DMPSUB minor partial-product writes. */
static void load(apollo_overlay_state *m,unsigned at,uint8_t negate){m->words[0]=agc_extend(negate?agc_negate(m->words[at]):m->words[at]);m->words[1]=agc_extend(negate?agc_negate(m->words[at+1u]):m->words[at+1u]);}
static void add_pair(apollo_overlay_state *m,unsigned at){agc_register_pair r={m->words[0],m->words[1]};agc_dp p={m->words[at],m->words[at+1u]};agc_double_add(&r,&p);m->words[at]=p.high;m->words[at+1u]=p.low;m->words[0]=r.a;m->words[1]=r.l;}
static void multiply(apollo_overlay_state *m,unsigned at){agc_register_pair r=agc_multiply(m->words[0],m->words[at]);m->words[0]=r.a;m->words[1]=r.l;}
static void swap_a(apollo_overlay_state *m,unsigned at){agc_register a=m->words[0];m->words[0]=agc_extend(m->words[at]);m->words[at]=agc_correct(a);}
static unsigned address(const apollo_overlay_state *m);
static void product(apollo_overlay_state *m,const agc_word *fixed,agc_word base){
 unsigned at=address(m),offset=(unsigned)((m->words[0116]&07777u)-base);
 /* The major operand is fetched after the minor partial products. This
  * ordering matters when operands alias MPTEMP/MPAC/BUF. */
 m->words[0]=agc_extend(fixed?fixed[offset+1u]:m->words[at+1u]);m->words[0156]=agc_correct(m->words[0]);
 m->words[0]=0;swap_a(m,0155);m->words[0135]=agc_correct(m->words[0]);
 multiply(m,0156);swap_a(m,0156);multiply(m,0154);add_pair(m,0155);
 m->words[0]=agc_extend(fixed?fixed[offset]:m->words[at]);swap_a(m,0135);multiply(m,0135);add_pair(m,0155);
 swap_a(m,0154);multiply(m,0135);add_pair(m,0154);m->words[2]=3;
}
static void component_add(apollo_overlay_state *m,unsigned at,uint8_t offset){add_pair(m,at);if(m->words[0]){agc_word limit=m->words[0]==1u?037777u:040000u;agc_register a;int overflow;m->words[0130]=limit;m->words[1]=offset;a=agc_add_register(agc_extend(limit),m->words[0]==1u?1u:0177776u);a=agc_add_register(a,agc_extend(m->words[at+1u]));m->words[at+1u]=agc_correct(a);overflow=agc_overflow(a);a=overflow?(overflow>0?1u:0177776u):0u;a=agc_add_register(a,agc_extend(limit));a=agc_add_register(a,agc_extend(m->words[at]));m->words[at]=agc_correct(a);overflow=agc_overflow(a);if(overflow){a=overflow>0?1u:0177776u;m->words[0121]=agc_correct(a);}m->words[0]=a;m->words[2]=3;}}
static unsigned address(const apollo_overlay_state *m){unsigned at=m->words[0116]&07777u;return at<01400u?at:(unsigned)(m->words[3]+(at&0377u));}

static uint8_t valid(const apollo_overlay_state *m,enum apollo_fixed_mission mission,enum sband_scalar_operation op,const agc_word *fixed){
 unsigned i,at;uint16_t entry;
 if(!m||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM)||(unsigned)op>SBAND_DSU)return 0;
 entry=mission==APOLLO_FIXED_LM?(op==SBAND_DMP?07572u:op==SBAND_DAD?06775u:07005u):(op==SBAND_DMP?07541u:op==SBAND_DAD?06744u:06754u);
 if(m->words[5]!=entry||m->words[7]||m->words[4]>076000u||(m->words[4]&01777u)||m->words[3]>03400u||(m->words[3]&0377u)||m->words[6]!=(agc_word)(m->words[4]|(m->words[3]>>8))||m->words[0163])return 0;
 if(fixed){if(m->words[0116]<02000u||m->words[0116]>07776u||(m->words[0116]>03776u&&m->words[0116]<04000u))return 0;for(i=0;i<2u;++i)if(fixed[i]>AGC_MASK)return 0;}
 else{if(m->words[0116]<030u||m->words[0116]>01776u)return 0;at=address(m);if(at<030u||at>03776u)return 0;}
 for(i=2;i<2048u;++i)if(m->words[i]>AGC_MASK)return 0;
 return 1;
}
uint8_t sband_scalar(apollo_overlay_state *m,enum apollo_fixed_mission mission,enum sband_scalar_operation op,const agc_word fixed_dp[2]){
 unsigned at;uint8_t negative;
 if(!valid(m,mission,op,fixed_dp))return 0;
 if(op==SBAND_DMP)product(m,fixed_dp,m->words[0116]);
 else{negative=(uint8_t)(op==SBAND_DSU);at=address(m);
  if(fixed_dp){m->words[0]=agc_extend(negative?agc_negate(fixed_dp[0]):fixed_dp[0]);m->words[1]=agc_extend(negative?agc_negate(fixed_dp[1]):fixed_dp[1]);}
  else load(m,at,negative);
  component_add(m,0154,0);
 }
 m->words[5]=mission==APOLLO_FIXED_LM?06061u:06030u;return 1;
}
