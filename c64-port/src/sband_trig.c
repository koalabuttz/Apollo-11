/* SPDX-License-Identifier: MIT */
#include "sband_trig.h"
#include "guidance_trig.h"
uint8_t sband_trig(apollo_overlay_state *m,enum apollo_fixed_mission mission,uint8_t cosine){
 p66_preparation p={0};p66_scale_scratch sc;p66_divide_scratch d={0};p66_norm_scratch n={0};p66_trig_scratch t={0};unsigned i;uint16_t entry;
 if(!m||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM)||cosine>1u)return 0;
 entry=mission==APOLLO_FIXED_CM?(cosine?03516u:03527u):(cosine?03517u:03530u);
 if(m->words[5]!=entry||m->words[0163]!=0||m->words[7]||m->words[3]>03400u||(m->words[3]&0377u)||m->words[4]||m->words[6]!=(agc_word)(m->words[3]>>8))return 0;
 for(i=2;i<2048u;++i)if(m->words[i]>AGC_MASK)return 0;
 for(i=0;i<8u;++i)p.mpac[i]=m->words[0154u+i];
 p.rate.registers.a=m->words[0];p.rate.registers.l=m->words[1];sc.mptemp=m->words[0135];sc.ovfind=m->words[0121];d.maxdvsw=m->words[0140];n.addrwd=m->words[0116];n.tem1=m->words[0141];
 for(i=0;i<6u;++i)n.vbuf[i]=m->words[0122u+i];
 t.buf2.high=m->words[0133];t.buf2.low=m->words[0134];t.polish=m->words[0117];
 if(cosine)(void)p66_cosine(&p,&sc,&d,&n,&t);else(void)p66_sine(&p,&sc,&d,&n,&t);
 for(i=0;i<8u;++i)m->words[0154u+i]=p.mpac[i];
 m->words[0]=p.rate.registers.a;m->words[1]=p.rate.registers.l;m->words[2]=3;m->words[5]=mission==APOLLO_FIXED_CM?06030u:06061u;m->words[0135]=sc.mptemp;m->words[0121]=sc.ovfind;m->words[0140]=d.maxdvsw;m->words[0116]=n.addrwd;
 m->words[0141]=mission==APOLLO_FIXED_CM?03572u:n.tem1;m->words[0117]=mission==APOLLO_FIXED_CM?03557u:t.polish;
 for(i=0;i<6u;++i)m->words[0122u+i]=n.vbuf[i];
 m->words[0133]=t.buf2.high;m->words[0134]=t.buf2.low;return 1;
}
