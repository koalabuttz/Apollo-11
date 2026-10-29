/* SPDX-License-Identifier: MIT */
#include "ephemeris_time_scale.h"
uint8_t ephemeris_time_downscale(apollo_overlay_state *m,enum apollo_fixed_mission mission)
{
 unsigned i;
 if(!m||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return 0;
 if(m->words[0]!=0177u||m->words[5]!=02214u||m->words[0116]!=020616u||m->words[0163]||m->words[7]||m->words[4]||m->words[3]>03400u||(m->words[3]&0377u)||m->words[6]!=(m->words[3]>>8))return 0;
 for(i=2;i<2048u;++i)if(m->words[i]>077777u)return 0;
 m->words[0156]=m->words[0155];m->words[0155]=m->words[0154];m->words[0154]=0;m->words[0135]=077777;m->words[0]=0;m->words[1]=03400;m->words[5]=mission==APOLLO_FIXED_LM?06061u:06030u;return 1;
}
