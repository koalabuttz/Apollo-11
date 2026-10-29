/* SPDX-License-Identifier: MIT */
#include "rtb_unit_cases.h"
#include "rtb_vector_cases.h"
void rtb_unit_input(apollo_overlay_state *m,p66_bias_io *io,enum rtb_mission mission,uint8_t x1,uint32_t n)
{
 unsigned i;static const agc_word tiny[16]={0,077777,1,077776,2,077775,3,077774,077,077700,037777,040000,020000,057777,017777,060000};
 rtb_vector_input(m,io,mission,x1?RTB_NORMUNX1_FRONT:RTB_NORMUNIT_FRONT,n);
 m->words[0163]=077776;
 if(n<8192u){static const unsigned at[3]={0,3,5};for(i=0;i<3u;++i){m->words[0154+at[i]]=(agc_word)((n&1u)?077777u:0u);m->words[0155+at[i]]=tiny[((n>>(i*4u))+i)&15u];}}
 m->words[0166]=m->words[0120];
 if(x1==2u){m->words[5]=03023;m->words[4]=0;m->words[6]=(agc_word)(m->words[3]>>8);}
}
