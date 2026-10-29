/* SPDX-License-Identifier: MIT */
#include "rtb_vector_cases.h"
void rtb_vector_input(apollo_overlay_state *m,p66_bias_io *io,enum rtb_mission mission,enum rtb_vector_entry entry,uint32_t n)
{
 static const uint16_t start[2][5]={{03010,03772,03726,03724,03705},{03010,03772,03726,03724,03653}};
 static const uint16_t corner[16]={0,077777,1,077776,037777,040000,020000,057777,017777,060000,2,077775,077,077700,010000,067777};
 unsigned i,context=(unsigned)(n>>15),bank=(unsigned)((n>>4)&7u);
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((i*173u+context*731u+(unsigned)mission*191u+012345u)&077777u);
 m->words[0]=(agc_register)(n*43u&0177777u);m->words[1]=(agc_register)(n*19u&0177777u);
 m->words[2]=(agc_word)(02000u+n%3072u);m->words[3]=(agc_word)(bank*0400u);m->words[4]=(agc_word)(entry==RTB_VECAGREE?0u:(mission==RTB_LM?020000u:044000u));m->words[5]=start[mission][entry];m->words[6]=(agc_word)(m->words[4]|bank);m->words[7]=0;
 for(i=0;i<7u;++i)m->words[0154+i]=(agc_word)((n*(i*2u+1u)+i*513u)&077777u);
 if(n<4096u)for(i=0;i<7u;++i)m->words[0154+i]=corner[((n>>(i%3u*4u))+i)&15u];
 m->words[0163]=(agc_word)(n&077777u);io->interrupt_enabled=(agc_word)(n&1u);io->superbank=(agc_word)(((n>>7)&7u)*020u);
 {static const agc_word bases[5]={0401,0455,0531,0605,0661};m->words[0120]=bases[n%5u];}
 if(entry==RTB_NORMUNIT_FRONT||entry==RTB_NORMUNX1_FRONT){
  if(n&1u)m->words[0154]=m->words[0157]=m->words[0161]=(agc_word)((n&2u)?077777u:0u);
 }
 m->words[m->words[0120]+38u]=(agc_word)(n&077777u);
}
