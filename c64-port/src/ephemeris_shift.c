/* SPDX-License-Identifier: MIT */
#include "ephemeris_shift.h"
#include "agc_multiword.h"
uint8_t ephemeris_scalar_left(apollo_overlay_state *m,enum apollo_fixed_mission mission)
{
 unsigned i,count;agc_register_pair r;agc_word cyr;int ov;
 if(!m||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return 0;
 if(m->words[0]!=0177u||m->words[5]!=02214u||m->words[7]||m->words[4]||m->words[3]>03400u||(m->words[3]&0377u)||m->words[6]!=(m->words[3]>>8)||m->words[0163])return 0;
 count=m->words[0116]&0177u;
 if((count!=5u&&count!=8u&&count!=16u)||m->words[0116]!=(020200u+count))return 0;
 for(i=2;i<2048u;++i)if(m->words[i]>077777u)return 0;
 r=agc_multiply(agc_extend((agc_word)(m->words[0116]^077777u)),040u);
 cyr=agc_correct(r.a);cyr=(agc_word)((cyr>>1)|((cyr&1u)<<14));m->words[020]=cyr;
 for(i=0;i<count;++i){agc_dp low={m->words[0155],m->words[0156]};r.a=agc_extend(low.high);r.l=agc_extend(low.low);agc_double_add(&r,&low);m->words[0155]=low.high;m->words[0156]=low.low;r.a=agc_add_register(r.a,agc_extend(m->words[0154]));r.a=agc_add_register(r.a,agc_extend(m->words[0154]));m->words[0154]=agc_correct(r.a);ov=agc_overflow(r.a);if(ov)m->words[0121]=ov>0?1u:077776u;}
 m->words[020]=(agc_word)((cyr>>1)|((cyr&1u)<<14));m->words[0]=agc_extend(agc_diminished_abs(cyr));m->words[1]=r.l;m->words[0135]=0;m->words[5]=mission==APOLLO_FIXED_LM?06061u:06030u;return 1;
}
