/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_load.h"
static agc_word source_read(uint16_t *w,agc_word address)
{
    agc_word value=address<=2u?agc_correct(w[address]):w[address];
    if(address==020u)w[address]=(agc_word)((value>>1)|((value&1u)<<14));
    else if(address==021u)w[address]=(agc_word)((value>>1)|(value&040000u));
    else if(address==022u)w[address]=(agc_word)((((unsigned)value<<1)&AGC_MASK)|(value>>14));
    else if(address==023u)w[address]=(agc_word)((value>>7)&0177u);
    return value;
}
enum p66_bias_status dsky_decimal_load(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission)
{
    uint16_t *w;agc_word field,component,count,address,value;
    if(s==0||io==0||(unsigned)mission>1u)return P66_BIAS_OUT_OF_SCOPE;
    w=s->words;
    if(w[5]!=02523u||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(062000u|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||(w[0140]!=1u&&w[0140]!=2u)||w[0145]<010u||w[0145]>01777u)return P66_BIAS_OUT_OF_SCOPE;
    field=(agc_word)((w[w[0140]==1u?0147u:0146u]&076000u)>>10);component=(agc_word)(field&3u);
    if(component>2u)return P66_BIAS_OUT_OF_SCOPE;
    w[1]=agc_extend(agc_correct(w[1]));w[022]=agc_negate(field);w[0117]=component;
    w[2]=3;count=component;
    for(;;){
        w[0122]=count;address=(agc_word)(w[0145]+count);w[0]=address;w[5]=02533;
        /* A three-word noun at the top of erasable (the LM's TCDH at
         * 01776) reads its last words from fixed memory: FB 31 with the
         * superbank is bank 41, whose first words are TC NVSUB1 and
         * TC LOADLV1 in both missions. */
        if(address>=02000u){if(io->superbank!=0100u)return P66_BIAS_OUT_OF_SCOPE;value=address==02000u?03544u:02775u;}
        else{if(address>=01400u)address=(agc_word)(w[3]+(address&0377u));value=source_read(w,address);}w[01003u+count]=agc_negate(value);
        if(count==0)break;
        --count;w[2]=02537;
    }
    w[0]=0;w[5]=02537;return P66_BIAS_OK;
}
