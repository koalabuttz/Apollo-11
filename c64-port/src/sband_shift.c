/* SPDX-License-Identifier: MIT */
#include "sband_shift.h"
#include "guidance_shift.h"
/* Before the first overflow, all preceding source components are plain DAS.
 * Observe whether any source TC OVERFLOW occurs; after the first such call
 * genuine TC Q leaves Q3. No continuation address is inferred from MPAC. */
static uint8_t invokes_overflow(const p66_preparation *state,uint8_t count){
 static const uint8_t at[3]={0,3,5};agc_dp pairs[3];unsigned i,j;
 for(j=0;j<3u;++j){pairs[j].high=state->mpac[at[j]];pairs[j].low=state->mpac[at[j]+1u];}
 for(i=0;i<count;++i){for(j=0;j<3u;++j){agc_register_pair r={agc_extend(pairs[j].high),agc_extend(pairs[j].low)};agc_double_add(&r,&pairs[j]);if(r.a)return 1;}}
 return 0;
}
uint8_t sband_shift_left(apollo_overlay_state *m,enum apollo_fixed_mission mission,uint8_t count){
 p66_preparation p={0};p66_scale_scratch sc;p66_elapsed_state el={0};unsigned i;uint8_t called;
 if(!m||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM)||!count||count>8u)return 0;
 if(m->words[5]!=02121u||m->words[4]||m->words[3]>03400u||(m->words[3]&0377u)||m->words[6]!=(m->words[3]>>8)||m->words[7]||m->words[0163]!=077776u||m->words[020]!=(agc_word)(020000u+count-1u))return 0;
 for(i=2;i<2048u;++i){if(m->words[i]>AGC_MASK)return 0;}
 for(i=0;i<8u;++i){p.mpac[i]=m->words[0154u+i];}
 p.rate.registers.a=m->words[0];p.rate.registers.l=m->words[1];sc.mptemp=m->words[0135];sc.ovfind=m->words[0121];el.buf=m->words[0130];called=invokes_overflow(&p,count);
 (void)p66_vector_shift_left(&p,&sc,&el,count,1);
 for(i=0;i<8u;++i){m->words[0154u+i]=p.mpac[i];}
 m->words[0]=p.rate.registers.a;m->words[1]=p.rate.registers.l;m->words[0135]=sc.mptemp;m->words[0121]=sc.ovfind;m->words[0130]=el.buf;
 m->words[020]=(agc_word)((m->words[020]>>1)|((m->words[020]&1u)<<14));if(called)m->words[2]=3;
 m->words[5]=mission==APOLLO_FIXED_LM?06061u:06030u;return 1;
}
