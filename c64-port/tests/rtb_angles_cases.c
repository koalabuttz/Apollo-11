/* SPDX-License-Identifier: MIT */
#include "rtb_angles_cases.h"
void rtb_angles_input(apollo_overlay_state *m,p66_bias_io *io,enum rtb_mission mission,enum rtb_angle_entry entry,uint32_t n)
{
 static const uint16_t start[2][9]={{03644,03614,03620,03635,03576,03660,0,03573,0},{03576,03546,03552,03567,03513,03626,03612,03510,03527}};
 static const uint16_t corner[16]={0,077777,1,077776,037777,040000,020000,057777,017777,060000,2,077775,077,077700,010000,067777};
 unsigned i,context=(unsigned)(n>>15),bank=(unsigned)((n>>4)&7u);uint16_t buf;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((i*173u+context*731u+(unsigned)mission*191u+012345u)&077777u);
 m->words[0]=(agc_register)(n*43u&0177777u);m->words[1]=(agc_register)(n*19u&0177777u);
 m->words[2]=(agc_word)(02000u+n%3072u);m->words[3]=(agc_word)(bank*0400u);m->words[4]=(agc_word)(mission==RTB_LM?020000u:044000u);m->words[5]=start[mission][entry];m->words[6]=(agc_word)(m->words[4]|bank);m->words[7]=0;
 for(i=0;i<7u;++i)m->words[0154+i]=(agc_word)((n*(i*2u+1u)+i*513u)&077777u);
 if(n<4096u)for(i=0;i<7u;++i)m->words[0154+i]=corner[((n>>(i%3u*4u))+i)&15u];
 m->words[0163]=(agc_word)(n&077777u);io->interrupt_enabled=(agc_word)(n&1u);io->superbank=(agc_word)(((n>>7)&7u)*020u);
 if(entry==RTB_CDU_INCREMENT){
  m->words[0]=agc_extend((agc_word)(n&077777u));
  buf=(agc_word)((n&0200u)?01440u+(n%0340u):040u+n%01340u);
  m->words[0130]=buf;
  i=buf>=01400u?m->words[3]+buf-01400u:buf;
  m->words[i]=(agc_word)((n*17u)&077777u);
  /* Target aliases may overwrite BUF or TEM2; prevent controls/MPAC image
   * aliases from invalidating the entry context, while preserving scratch.
   */
  m->words[5]=start[mission][entry];m->words[7]=0;
  m->words[0130]=buf;
 }
 if(entry==RTB_LOAD_TIME){m->words[024]=(agc_word)(n&077777u);m->words[025]=(agc_word)((n*23u)&077777u);}
 if(entry==RTB_READ_PIPS){m->words[037]=(agc_word)(n&077777u);m->words[040]=(agc_word)((n*7u)&077777u);m->words[041]=(agc_word)((n*13u)&077777u);}
}
