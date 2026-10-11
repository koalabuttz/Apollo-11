/* SPDX-License-Identifier: MIT */
#include "dsky_nv_blank.h"
enum p66_bias_status dsky_nv_blank(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel11,enum dsky_nv_mission mission)
{
    uint16_t *w;agc_word code,old,n;
    if(s==0||io==0||channel11==0||(mission!=DSKY_NV_LM&&mission!=DSKY_NV_CM))return P66_BIAS_OUT_OF_SCOPE;
    w=s->words;
    if(w[5]!=03516u||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||*channel11>AGC_MASK||w[0123]<077773u||w[0123]>077776u||w[0]!=(agc_word)(077776u-w[0123])||w[01016]>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
    for(n=0;n<11u;++n)if(w[01023u+n]>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
    code=(agc_word)(w[0]+7u);
    for(;;){
        w[0124]=code;old=w[01023u+code];w[01023u+code]=073777;
        if(agc_classify(old)==AGC_POSITIVE)w[01016]=agc_correct(agc_add_register(agc_extend(w[01016]),1));
        /* CCS -zero skips the following CCS CODE and loops with A=+0. */
        if(old==AGC_NEG_ZERO){code=0;continue;}
        if(code==0)break;
        --code;
    }
    n=(agc_word)(AGC_MASK^w[0123]);
    if(n>=3u)w[01001]=0;
    if(n>=2u)w[01002]=0;
    w[01015]=0;w[0777]=077754;
    *channel11&=077737u;w[0]=agc_extend(*channel11);
    w[1]=agc_extend(agc_correct(w[1]));w[2]=03544;w[5]=03566;
    io->interrupt_enabled=1;
    return P66_BIAS_OK;
}
