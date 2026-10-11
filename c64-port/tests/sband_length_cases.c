/* SPDX-License-Identifier: MIT */
#include "sband_length_cases.h"
void sband_length_input(apollo_overlay_state *s,p66_bias_io *io,uint32_t id,
                      enum apollo_fixed_mission *mission)
{
 unsigned i,n=(unsigned)(id%65536u),eb=n%8u;uint32_t seed=UINT32_C(0x524f354f)+(n/32u)*UINT32_C(0x9e3779b9);
 *mission=id<131072u?(id>=65536u?APOLLO_FIXED_CM:APOLLO_FIXED_LM):((id-131072u)>=98304u?APOLLO_FIXED_CM:APOLLO_FIXED_LM);
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;s->words[i]=(agc_word)(seed&AGC_MASK);}
 s->words[0]=(agc_register)(n*257u);s->words[1]=(agc_register)(n*977u);s->words[2]=(agc_word)((n*43u)&AGC_MASK);
 s->words[3]=(agc_word)(eb*0400u);s->words[4]=0;s->words[6]=(agc_word)eb;s->words[7]=0;
 s->words[035]=s->words[036]=s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[067]=0;s->words[0120]=(agc_word)(0401u+(n%5u)*054u);
 s->words[0164]=*mission==APOLLO_FIXED_LM?03627u:03673u;
 s->words[0165]=s->words[6];s->words[0115]=040000;s->words[023]=0;
 s->words[5]=03201;
 s->words[0163]=077776;
 if(n<16u){for(i=0;i<7u;++i)s->words[0154u+i]=(agc_word)(n&1u?077777u:0);if(n>=2u)s->words[0154]=(agc_word)(n*01000u);}
 if(n>=16u&&n<32u){static const agc_word tiny[16]={1,2,3,16,64,127,128,129,255,256,512,1024,2048,4096,8192,16383};for(i=0;i<7u;++i)s->words[0154u+i]=(agc_word)(n&1u?077777u:0);s->words[0155]=tiny[n-16u];}
if(id>=131072u){unsigned axis=(unsigned)(((id-131072u)%98304u)/32768u);for(i=0;i<7u;++i)s->words[0154u+i]=(agc_word)(n&1u?077777u:0);s->words[0154u+(axis==0u?1u:axis==1u?4u:6u)]=(agc_word)(id&AGC_MASK);}
 io->superbank=(agc_word)(0100u+((n/8u)%4u)*020u);io->interrupt_enabled=(agc_word)(n&1u);
}
