/* SPDX-License-Identifier: MIT */
#include "rtb_agreement_cases.h"
void rtb_agreement_input(apollo_overlay_state *m,p66_bias_io *io,enum rtb_mission mission,enum rtb_agreement_entry entry,uint32_t n)
{
 static const uint16_t start[2][4]={{03612,03712,0,0},{03544,03712,03662,03676}};
 static const uint16_t corner[16]={0,077777,1,077776,037777,040000,020000,057777,017777,060000,2,077775,077,077700,010000,067777};
 unsigned i,context=(unsigned)(n>>15),bank=(unsigned)((n>>4)&7u);
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((i*173u+context*731u+(unsigned)mission*191u+012345u)&077777u);
 m->words[0]=(agc_register)(n*43u&0177777u);m->words[1]=(agc_register)(n*19u&0177777u);
 m->words[2]=(agc_word)(02000u+n%3072u);m->words[3]=(agc_word)(bank*0400u);m->words[4]=(agc_word)(mission==RTB_LM?020000u:044000u);m->words[5]=start[mission][entry];m->words[6]=(agc_word)(m->words[4]|bank);m->words[7]=0;
 for(i=0;i<7u;++i)m->words[0154+i]=(agc_word)((n*(i*2u+1u)+i*513u)&077777u);
 if(n<4096u)for(i=0;i<7u;++i)m->words[0154+i]=corner[((n>>(i%3u*4u))+i)&15u];
 m->words[0163]=(agc_word)(n&077777u);io->interrupt_enabled=(agc_word)(n&1u);io->superbank=(agc_word)(((n>>7)&7u)*020u);
 if(entry==RTB_TRANSPOSE1||entry==RTB_TRANSPOSE2){
  unsigned base=entry==RTB_TRANSPOSE1?1483u:1388u;
  for(i=0;i<18u;++i)m->words[base+i]=(agc_word)((n*(i*2u+1u)+i*313u)&077777u);
 }
}
