/* SPDX-License-Identifier: MIT */
#include "interpreted_sign_cases.h"
void interpreted_sign_input(apollo_overlay_state *m,agc_word *irq,
 enum interpreted_sign_mission mission,uint32_t n)
{
    static const agc_word edge[16]={0,077777,1,077776,020000,057777,037777,040000,
                                  2,077775,017777,060000,00044,077733,00045,077732};
    uint32_t seed=UINT32_C(0xc513fe72)+(n>>14)*UINT32_C(0x93213);
    unsigned i,kind=n>>16;
    for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(agc_word)(seed&AGC_MASK);}
    m->words[0]=(agc_word)(n*37u);m->words[1]=(agc_word)(n*71u);
    m->words[3]=(agc_word)(((n>>4)&7u)<<8);m->words[4]=(agc_word)(((n>>7)&037u)<<10);
    m->words[5]=(agc_word)(mission==INTERPRETED_SIGN_LM?(kind?07655:07670):(kind?07624:07637));
    m->words[6]=(agc_word)(m->words[4]|(m->words[3]>>8));m->words[7]=0;
    m->words[0116]=0260;
    m->words[0154]=(agc_word)(n&AGC_MASK);m->words[0155]=(agc_word)((n*13u)&AGC_MASK);
    m->words[0156]=(agc_word)((n*17u)&AGC_MASK);
    m->words[0157]=(agc_word)((n*19u)&AGC_MASK);m->words[0160]=(agc_word)((n*23u)&AGC_MASK);
    m->words[0161]=(agc_word)((n*29u)&AGC_MASK);m->words[0162]=(agc_word)((n*31u)&AGC_MASK);
    m->words[0163]=(agc_word)((n&65535u)>>1);
    if((n&1u)!=0)m->words[0163]=agc_negate(m->words[0163]);
    if(m->words[0163]==077777)m->words[0163]=077776;
    m->words[0260]=(agc_word)(n&AGC_MASK);m->words[0261]=(agc_word)((n*7u)&AGC_MASK);
    if((n&65535u)<4096u){
        for(i=0;i<7u;++i)m->words[0154u+i]=edge[(n>>(i%3u*4u))&15u];
        m->words[0260]=edge[n&15u];m->words[0261]=edge[(n>>4)&15u];
        m->words[0163]=(agc_word)((n>>8)%3u==2u?077776u:(n>>8)%3u);
    }
    *irq=(agc_word)((n>>3)&1u);
}
