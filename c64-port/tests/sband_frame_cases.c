/* SPDX-License-Identifier: MIT */
#include "sband_frame_cases.h"
void sband_frame_input(apollo_overlay_state *s,p66_bias_io *io,uint32_t id,enum apollo_fixed_mission *mission,enum sband_frame_stage *stage){unsigned i,n=(unsigned)(id%8192u),g=(unsigned)(id/8192u),eb=n%8u,base;uint32_t seed=UINT32_C(0x53424652)+(n/16u)*UINT32_C(0x9e3779b9);
 *mission=g==3u||g==5u?APOLLO_FIXED_CM:APOLLO_FIXED_LM;*stage=g<2u?(enum sband_frame_stage)g:g<4u?SBAND_FRAME_MATRIX:SBAND_FRAME_SHIFT;
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;s->words[i]=(agc_word)(seed&AGC_MASK);}
 s->words[0]=(agc_register)(n*257u);s->words[1]=(agc_register)(n*977u);s->words[2]=g<2u?3u:0140u;
 s->words[3]=(agc_word)(eb*0400u);s->words[4]=0;s->words[6]=(agc_word)eb;s->words[7]=0;
 s->words[035]=s->words[036]=s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[067]=0;s->words[0120]=(agc_word)(0401u+(n%5u)*054u);base=s->words[0120];s->words[0166]=(agc_word)base;
 s->words[0164]=03641;s->words[0165]=(agc_word)(064000u|eb);s->words[0115]=040000;s->words[023]=0;s->words[0163]=077776;s->words[020]=5;s->words[0116]=(agc_word)base;s->words[0117]=065636;
 s->words[5]=g==0u?03023u:06061u;
 if(g>=2u){s->words[3]=01400;s->words[4]=064000;s->words[6]=064003;s->words[0164]=*mission==APOLLO_FIXED_CM?03703u:03645u;s->words[023]=*mission==APOLLO_FIXED_CM?0u:5u;s->words[0116]=*mission==APOLLO_FIXED_CM?01735u:01733u;s->words[020]=020013;s->words[0117]=*mission==APOLLO_FIXED_CM?065677u:065644u;s->words[5]=g<4u?(*mission==APOLLO_FIXED_CM?07303u:07334u):(*mission==APOLLO_FIXED_CM?06030u:06061u);if(g>=4u)s->words[0116]=(agc_word)(s->words[0116]+016u);}
 if(n<16u){for(i=0;i<7u;++i)s->words[0154u+i]=(agc_word)((n&1u)?077777u:0);if(n>=2u)s->words[0154]=(agc_word)(n*01000u);}
 io->superbank=(agc_word)(0100u+((n/8u)%4u)*020u);io->interrupt_enabled=(agc_word)(n&1u);
}
