/* SPDX-License-Identifier: MIT */
#include "rtb_agreement.h"
static void agree(uint16_t *w,uint16_t returned)
{
    agc_register a=0;agc_word limit;int sign=0,overflow;unsigned i;
    for(i=0;i<3u;++i)if((sign=agc_to_int(w[0154+i]))!=0)break;
    if(sign==0){w[0154]=w[0155]=w[0156]=0;}
    else{
        limit=(agc_word)(sign>0?037777u:040000u);
        a=agc_add_register(agc_extend(limit),(agc_register)(sign>0?1u:0177776u));
        for(i=2u;i>0u;--i){
            a=agc_add_register(a,agc_extend(w[0154+i]));w[0154+i]=agc_correct(a);
            overflow=agc_overflow(a);a=(agc_register)(overflow?(overflow>0?1u:0177776u):0u);
            a=agc_add_register(a,agc_extend(limit));
        }
        a=agc_add_register(a,agc_extend(w[0154]));w[0154]=agc_correct(a);
        overflow=agc_overflow(a);
        if(overflow)a=(agc_register)(overflow>0?1u:0177776u);else w[0155]=agc_correct(a);
    }
    w[0]=a;w[1]=returned;w[2]=2;
}
enum rtb_status rtb_agreement(apollo_overlay_state *image,p66_bias_io *io,enum rtb_mission mission,enum rtb_agreement_entry entry)
{
    static const uint16_t start[2][4]={{03612,03712,0,0},{03544,03712,03662,03676}};
    uint16_t *w=image->words,fb;unsigned i;uint16_t high,low;
    if((unsigned)mission>1u||(unsigned)entry>3u)return RTB_OUT_OF_SCOPE;
    fb=(uint16_t)(mission==RTB_LM?020000u:044000u);
    if(!start[mission][entry]||w[5]!=start[mission][entry]||w[4]!=fb||w[3]>03400u||
       (w[3]&0377u)||w[6]!=(fb|(w[3]>>8))||w[7]||io->interrupt_enabled>1u||
       io->superbank>0160u||(io->superbank&017u))return RTB_OUT_OF_SCOPE;
    if(entry==RTB_SGN_AGREE)agree(w,(uint16_t)(start[mission][entry]+1u));
    else if(entry==RTB_SIGN_MPAC){
        high=w[0154];low=w[0155];
        w[0154]=w[0155]=(agc_word)((high&040000u)?040000u:037777u);
        w[0156]=w[0163]=0;w[0]=0;w[1]=agc_extend((agc_word)((high&040000u)?037777u:low));
    }else{
        static const unsigned left[3]={2,4,10},right[3]={6,12,14};
        unsigned base=entry==RTB_TRANSPOSE1?1483u:1388u;
        for(i=0;i<3u;++i){high=w[base+left[i]];low=w[base+left[i]+1u];w[base+left[i]]=w[base+right[i]];w[base+left[i]+1u]=w[base+right[i]+1u];w[base+right[i]]=high;w[base+right[i]+1u]=low;}
        w[0]=(agc_word)base;w[1]=agc_extend(agc_correct(w[1]));w[3]=02400;w[6]=(uint16_t)(fb|5u);
    }
    w[5]=(uint16_t)(mission==RTB_LM?06061u:06030u);return RTB_OK;
}

uint8_t rtb_agreement_constant_read(enum rtb_mission mission,uint16_t address,uint16_t *out)
{
    if((unsigned)mission!=1u||out==0||address<19376u||address>19377u)return 0;
    *out=(uint16_t)(address==19376u?1483u:1388u);return 1;
}
