/* SPDX-License-Identifier: MIT */
#include "ephemeris_add.h"
#include "agc_multiword.h"
static agc_register carry(int ov){return ov>0?1u:0177776u;}
uint8_t ephemeris_triple_add(apollo_overlay_state *m,enum apollo_fixed_mission mission)
{
 unsigned at,i;agc_register a;agc_register_pair r;agc_dp minor;int ov;
 if(!m||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return 0;
 if(m->words[5]!=(mission==APOLLO_FIXED_LM?07071u:07040u)||m->words[7]||m->words[3]>03400u||(m->words[3]&0377u)||m->words[4]>076000u||(m->words[4]&01777u)||m->words[6]!=(uint16_t)(m->words[4]|(m->words[3]>>8)))return 0;
 if(m->words[0116]<030u||m->words[0116]>01775u)return 0;
 at=m->words[0116];if(at>=01400u)at=(unsigned)(m->words[3]+(at&0377u));
 if(at<030u||at>03775u)return 0;
 for(i=2;i<2048u;++i)if(m->words[i]>077777u)return 0;
 r.a=agc_extend(m->words[at+1u]);r.l=agc_extend(m->words[at+2u]);
 minor.high=m->words[0155];minor.low=m->words[0156];agc_double_add(&r,&minor);
 m->words[0155]=minor.high;m->words[0156]=minor.low;m->words[1]=r.l;
 a=agc_add_register(r.a,agc_extend(m->words[at]));a=agc_add_register(a,agc_extend(m->words[0154]));
 m->words[0154]=agc_correct(a);ov=agc_overflow(a);
 if(ov){
  agc_word limit=ov>0?037777u:040000u;
  /* Both OVERFLOW exits execute literal TC Q, retaining its Q=3 debris. */
  m->words[2]=3;
  m->words[0130]=limit;
  a=agc_add_register(agc_extend(limit),carry(ov));a=agc_add_register(a,agc_extend(m->words[0155]));m->words[0155]=agc_correct(a);ov=agc_overflow(a);
  a=agc_add_register(ov?carry(ov):0u,agc_extend(limit));a=agc_add_register(a,agc_extend(m->words[0154]));m->words[0154]=agc_correct(a);ov=agc_overflow(a);
  if(ov){a=carry(ov);m->words[0121]=agc_correct(a);}
 }
 m->words[0]=a;m->words[5]=mission==APOLLO_FIXED_LM?06061u:06030u;return 1;
}
