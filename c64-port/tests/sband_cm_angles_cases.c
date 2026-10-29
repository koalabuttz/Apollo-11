/* SPDX-License-Identifier: MIT */
#include "sband_cm_angles_cases.h"
void sband_cm_angles_input(apollo_overlay_state *s,p66_bias_io *io,uint32_t id,enum apollo_fixed_mission *mission){unsigned i,n=(unsigned)(id%65536u),eb=n%8u,base;uint32_t seed=UINT32_C(0x53424246)+(n/16u)*UINT32_C(0x9e3779b9);
 *mission=APOLLO_FIXED_CM;
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;s->words[i]=(agc_word)(seed&AGC_MASK);}
 s->words[0]=(agc_register)(n*257u);s->words[1]=(agc_register)(n*977u);s->words[2]=(agc_word)((n*43u)&AGC_MASK);s->words[3]=(agc_word)(eb*0400u);s->words[4]=064000;s->words[6]=(agc_word)(064000u|eb);s->words[7]=0;
 s->words[035]=s->words[036]=s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[067]=0; s->words[0120]=(agc_word)(0401u+(n%5u)*054u);base=s->words[0120];s->words[0166]=(agc_word)(base+016u);
 s->words[0164]=03724;s->words[0165]=(agc_word)(064000u|eb);s->words[0115]=040000;s->words[023]=0;s->words[0163]=077776;s->words[020]=060017;s->words[0117]=*mission==APOLLO_FIXED_CM?065737u:065654u;
 s->words[5]=06037;s->words[0121]=(agc_word)(n&AGC_MASK);
 if(n<16u){for(i=0;i<7u;++i)s->words[0154u+i]=(agc_word)(n&1u?077777u:0);if(n>=2u)s->words[0154]=(agc_word)(n*01000u);}
 if(n>=16u&&n<4096u){unsigned r=*mission==APOLLO_FIXED_CM?base+2u:02204u;for(i=0;i<3u;++i){s->words[r+i*2u]=(agc_word)(01000u+(n*(i+1u)*7u)%010000u);s->words[r+i*2u+1u]=(agc_word)((n*(i+1u)*113u)&037777u);}s->words[0121]=(agc_word)(n&1u);}
 if(n<16u){unsigned r=*mission==APOLLO_FIXED_CM?base+2u:02204u;static const agc_word flags[4]={0,1,077776,077777};for(i=0;i<6u;++i)s->words[r+i]=(agc_word)(n&1u?077777u:0u);s->words[0121]=flags[(n/4u)%4u];if(n>=2u)s->words[r+(n%3u)*2u]=(agc_word)(n&1u?057777u:020000u);}
 if(id>=65536u){s->words[0164]=03737;s->words[020]=070007;s->words[0166]=(agc_word)(base+016u);s->words[0117]=065737;}
 io->superbank=(agc_word)(0100u+((n/8u)%4u)*020u);io->interrupt_enabled=(agc_word)(n&1u);
}
