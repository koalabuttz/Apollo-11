/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_row_cases.h"
void dsky_decimal_row_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission,enum dsky_decimal_row_entry *entry)
{
    uint32_t seed=0x256600u+(n%16u);uint16_t i,t=(uint16_t)(n%4096u);
    *mission=n<4096u?DSKY_NV_LM:DSKY_NV_CM;*entry=(enum dsky_decimal_row_entry)((t>>1)&1u);
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_register)((seed*23u)&0177777u);
    if(t<8u){static const agc_register raw[8]={0,0177777,037777,0140000,040000,0137777,1,0177776};s->words[0]=raw[t];s->words[1]=raw[7u-t];}
    s->words[7]=0;s->words[3]=(agc_word)(((t>>2)&7u)*0400u);s->words[4]=*entry==DSKY_DECIMAL_ROW_CALL?062000u:060000u;s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=*entry==DSKY_DECIMAL_ROW_CALL?02566u:*mission==DSKY_NV_LM?04631u:04675u;io->superbank=(agc_word)(0100u+((t&3u)*020u));io->interrupt_enabled=(agc_word)((t>>3)&1u);s->words[0117]=(agc_word)(t%3u);
    if(*entry==DSKY_DECIMAL_ROW_RETURN){s->words[0133]=02570;s->words[0134]=062000;}
}
