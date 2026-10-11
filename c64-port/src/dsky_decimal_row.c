/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_row.h"
enum p66_bias_status dsky_decimal_row(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_decimal_row_entry entry)
{
    uint16_t *w;agc_word pc,bank,count;
    if(s==0||io==0||(unsigned)mission>1u||(unsigned)entry>1u)return P66_BIAS_OUT_OF_SCOPE;
    w=s->words;pc=entry==DSKY_DECIMAL_ROW_CALL?02566u:mission==DSKY_NV_LM?04631u:04675u;bank=entry==DSKY_DECIMAL_ROW_CALL?062000u:060000u;
    if(w[5]!=pc||w[4]!=bank||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(bank|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||w[0117]>2u)return P66_BIAS_OUT_OF_SCOPE;
    if(entry==DSKY_DECIMAL_ROW_RETURN&&(w[0133]!=02570u||w[0134]!=062000u))return P66_BIAS_OUT_OF_SCOPE;
    w[0]=agc_extend(agc_correct(w[0]));w[1]=agc_extend(agc_correct(w[1]));
    if(entry==DSKY_DECIMAL_ROW_CALL){w[0133]=02570;w[0134]=062000;w[4]=060000;w[6]=(agc_word)(060000u|(w[3]>>8));w[5]=mission==DSKY_NV_LM?03226u:03133u;w[2]=mission==DSKY_NV_LM?04631u:04675u;}
    else {count=w[0117];w[0134]=060000;w[4]=062000;w[6]=(agc_word)(062000u|(w[3]>>8));w[0]=count?(agc_word)(count-1u):0u;if(count){w[0117]=(agc_word)(count-1u);w[2]=02575;w[5]=02537;}else{w[2]=02573;w[5]=0136;}}
    return P66_BIAS_OK;
}
