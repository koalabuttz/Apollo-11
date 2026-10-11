/* SPDX-License-Identifier: MIT */
#include "ephemeris_day.h"
#include "guidance_dp_divide.h"
uint8_t ephemeris_day_divide(apollo_overlay_state *m,enum apollo_fixed_mission mission)
{
 p66_preparation p={0};p66_scale_scratch sc;p66_elapsed_state e={0};p66_divide_scratch d;agc_dp operand={020u,017260u};unsigned i;agc_register old_l;enum p66_divide_result status;
 if(!m||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return 0;
 if(m->words[5]!=02353u||m->words[7]||m->words[0163]||m->words[0130]!=operand.high||m->words[0131]!=operand.low||m->words[4]||m->words[3]>03400u||(m->words[3]&0377u)||m->words[6]!=(m->words[3]>>8))return 0;
 for(i=2;i<2048u;++i)if(m->words[i]>077777u)return 0;
 for(i=0;i<8u;++i){p.mpac[i]=m->words[0154u+i];}
 p.rate.registers.a=m->words[0];p.rate.registers.l=m->words[1];sc.mptemp=m->words[0135];sc.ovfind=m->words[0121];e.buf=m->words[0130];d.buf_low=m->words[0131];d.dvsign=m->words[0136];d.dvnormct=m->words[0137];d.maxdvsw=m->words[0140];
 old_l=agc_extend(agc_correct(m->words[1]));status=p66_dp_divide(&p,&sc,&e,&d,operand,0);
 for(i=0;i<8u;++i){m->words[0154u+i]=p.mpac[i];}
 m->words[0]=p.rate.registers.a;m->words[1]=p.rate.registers.l;m->words[0130]=e.buf;m->words[0131]=d.buf_low;m->words[0136]=d.dvsign;m->words[0137]=d.dvnormct;m->words[0140]=d.maxdvsw;m->words[0121]=sc.ovfind;m->words[0135]=sc.mptemp;
 if(status==P66_DIVIDE_ZERO||(status==P66_DIVIDE_OVERFLOW&&d.dvsign!=077777u))m->words[1]=old_l;
 if(status==P66_DIVIDE_OVERFLOW)m->words[2]=02413u;else if(status==P66_DIVIDE_QUOTIENT)m->words[2]=3;
 m->words[5]=mission==APOLLO_FIXED_LM?06061u:06030u;return 1;
}
