/* SPDX-License-Identifier: MIT */
#include "sband_vector_linear.h"
#include "agc_multiword.h"
/* Native raw helpers shared semantically with the frozen VXV/VPROJ
 * translation; all new caller controls have an independent original oracle.
 * No source instruction word is decoded or dispatched. */
static void load(apollo_overlay_state *m,unsigned at,uint8_t negate){m->words[0]=agc_extend(negate?agc_negate(m->words[at]):m->words[at]);m->words[1]=agc_extend(negate?agc_negate(m->words[at+1u]):m->words[at+1u]);}
static void exchange(apollo_overlay_state *m,unsigned at){agc_register a=m->words[0],l=m->words[1];load(m,at,0);m->words[at]=agc_correct(a);m->words[at+1u]=agc_correct(l);}
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

static void round_product(apollo_overlay_state *m){
 agc_register a;int ov;swap_a(m,0156);a=agc_add_register(m->words[0],m->words[0]);m->words[1]=agc_extend(agc_correct(a));ov=agc_overflow(a);
 if(ov){a=agc_add_register(ov>0?1u:0177776u,agc_extend(m->words[0155]));m->words[0155]=agc_correct(a);ov=agc_overflow(a);
  if(ov){a=agc_add_register(ov>0?1u:0177776u,agc_extend(m->words[0154]));m->words[0154]=agc_correct(a);ov=agc_overflow(a);if(ov){a=ov>0?1u:0177776u;m->words[0121]=agc_correct(a);}}}
 m->words[0]=a;m->words[2]=3;
}

static void advance(apollo_overlay_state *m,agc_word n){m->words[0]=agc_add_register(agc_extend(n),agc_extend(m->words[0116]));m->words[0116]=agc_correct(m->words[0]);}
static uint8_t valid(const apollo_overlay_state *m,enum apollo_fixed_mission mission,enum sband_vector_operation op,const agc_word *fixed){unsigned i,at;uint16_t entry;
 if(!m||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM)||(unsigned)op>SBAND_VSU)return 0;
 entry=mission==APOLLO_FIXED_LM?(op==SBAND_VXSC?07401u:op==SBAND_VAD?06751u:06747u):(op==SBAND_VXSC?07350u:op==SBAND_VAD?06720u:06716u);
 if(m->words[5]!=entry||m->words[7]||m->words[3]>03400u||(m->words[3]&0377u)||m->words[4]>076000u||(m->words[4]&01777u)||m->words[6]!=(agc_word)(m->words[4]|(m->words[3]>>8))||(m->words[0163]!=077776u&&!(op==SBAND_VXSC&&m->words[0163]==0)))return 0;
 if(fixed){if(m->words[0116]<02000u||m->words[0116]>07772u||(m->words[0116]>03772u&&m->words[0116]<04000u))return 0;for(i=0;i<6u;++i)if(fixed[i]>AGC_MASK)return 0;}
 else{if(m->words[0116]<030u||m->words[0116]>01772u)return 0;at=address(m);if(at<030u||at>03772u)return 0;}
 for(i=2;i<2048u;++i)if(m->words[i]>AGC_MASK)return 0;
 return 1;
}
uint8_t sband_vector_linear(apollo_overlay_state *m,enum apollo_fixed_mission mission,enum sband_vector_operation op,const agc_word *fixed){agc_word base=m?m->words[0116]:0;unsigned at;uint8_t negative;
 if(!valid(m,mission,op,fixed))return 0;
 if(op==SBAND_VXSC){
  if(m->words[0163]==0){
   load(m,0154,0);exchange(m,0157);product(m,fixed,base);round_product(m);
   advance(m,2);load(m,0157,0);exchange(m,0154);exchange(m,0161);product(m,fixed,base);round_product(m);
   advance(m,2);exchange(m,0157);exchange(m,0154);exchange(m,0157);product(m,fixed,base);round_product(m);
   exchange(m,0154);exchange(m,0161);exchange(m,0154);m->words[0]=0177776;m->words[0163]=077776;
  }else{
   product(m,fixed,base);round_product(m);exchange(m,0157);exchange(m,0154);exchange(m,0157);
   product(m,fixed,base);round_product(m);exchange(m,0161);exchange(m,0154);exchange(m,0161);
   product(m,fixed,base);round_product(m);exchange(m,0154);exchange(m,0161);exchange(m,0157);exchange(m,0154);
  }
 }else{
  negative=(uint8_t)(op==SBAND_VSU);advance(m,negative?040000u:030000u);
  at=address(m);if(fixed){m->words[0]=agc_extend(negative?agc_negate(fixed[2]):fixed[2]);m->words[1]=agc_extend(negative?agc_negate(fixed[3]):fixed[3]);}else load(m,at+2u,negative);component_add(m,0157,3);
  at=address(m);if(fixed){m->words[0]=agc_extend(negative?agc_negate(fixed[4]):fixed[4]);m->words[1]=agc_extend(negative?agc_negate(fixed[5]):fixed[5]);}else load(m,at+4u,negative);component_add(m,0161,5);
  at=address(m);if(fixed){m->words[0]=agc_extend(negative?agc_negate(fixed[0]):fixed[0]);m->words[1]=agc_extend(negative?agc_negate(fixed[1]):fixed[1]);}else load(m,at,negative);component_add(m,0154,0);
 }
 m->words[5]=mission==APOLLO_FIXED_LM?06061u:06030u;return 1;
}
