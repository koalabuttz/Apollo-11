/* SPDX-License-Identifier: MIT */
#include "lm_single_trig_cases.h"
void lm_single_trig_input(apollo_overlay_state *m,agc_word *irq,uint16_t n,uint8_t selector){uint32_t seed=UINT32_C(0x35555075)+((uint32_t)n>>8)*UINT32_C(0x9e3779b9)+selector;unsigned i;for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(agc_word)(seed&AGC_MASK);}
 m->words[0]=n;m->words[1]=(agc_word)((uint32_t)n*37u+selector*8191u);m->words[2]=(agc_word)(02000u+n%3072u);m->words[3]=(agc_word)(((n/32u)%8u)<<8);m->words[4]=(agc_word)((n%32u)<<10);m->words[5]=selector?05032:05033;m->words[6]=(agc_word)(m->words[4]|(m->words[3]>>8));m->words[7]=0;m->words[020]=0;m->words[023]=0;m->words[067]=0;m->words[0163]=(agc_word)(n&AGC_MASK);
 *irq=(agc_word)(n&1u);
}
