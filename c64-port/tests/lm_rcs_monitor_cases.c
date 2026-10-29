/* SPDX-License-Identifier: MIT */
#include "lm_rcs_monitor_cases.h"
void lm_rcs_monitor_input(apollo_overlay_state *m,p66_bias_io *io,agc_word *ch,uint32_t n)
{
    unsigned i,j,context=(unsigned)(n>>16),pool=(unsigned)((n>>12)%9u),eb=(unsigned)((n>>9)&7u);
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)((i*173u+context*1777u+012345u)&077777u);
    m->words[0]=(agc_word)(n&0177777u);m->words[1]=(agc_word)((n*29u)&0177777u);
    m->words[3]=(agc_word)(eb*0400u);m->words[4]=014000;m->words[5]=03156;
    m->words[6]=(agc_word)(014000u|eb);m->words[7]=0;
    m->words[01262]=(agc_word)(context?077777u:(n*73u)&077777u);
    m->words[01263]=(agc_word)(context?0u:(n*41u)&077777u);
    m->words[01276]=(agc_word)((n>>8)&0377u);
    *ch=(agc_word)(((n&0377u)^0377u)|((n*53u)&077400u));
    io->interrupt_enabled=0;io->superbank=(agc_word)(((n>>6)&7u)*020u);
    for(i=0;i<EXEC_CORE_COUNT;++i){
        for(j=0;j<EXEC_CORE_WORDS;++j)m->words[0154+i*12u+j]=(agc_word)((01234u+i*311u+j*97u+context*421u)&077777u);
        m->words[0154+i*12u+EXEC_PRIORITY]=(agc_word)(01000u+(i+1u)*17u);
        if(context&&i%3u==0u)m->words[0154+i*12u+EXEC_PRIORITY]=agc_negate(m->words[0154+i*12u+EXEC_PRIORITY]);
        if(i==pool)m->words[0154+i*12u+EXEC_PRIORITY]=077777;
    }
    m->words[067]=(agc_word)((n&04000u)?077777u:((n>>4)%8u)*12u);
    if(pool==0u&&m->words[067]==0)m->words[067]=12;
}
