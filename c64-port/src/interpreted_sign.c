/* SPDX-License-Identifier: MIT */
#include "interpreted_sign.h"
static void complement_pair(apollo_overlay_state *m,unsigned offset)
{
    agc_word high=m->words[0154u+offset],low=m->words[0155u+offset];
    m->words[0154u+offset]=agc_negate(high);
    m->words[0155u+offset]=agc_negate(low);
    /* DCS followed by DXCH loads the original memory pair back into A/L. */
    m->words[0]=agc_extend(high);
    m->words[1]=agc_extend(low);
}
enum interpreted_sign_status interpreted_sign(apollo_overlay_state *m,
 enum interpreted_sign_mission mission,enum interpreted_sign_kind kind,
 const agc_dp *operand)
{
    unsigned expected;
    agc_word mode,sign_word;
    enum agc_ccs_class cl;
    if(mission!=INTERPRETED_SIGN_LM&&mission!=INTERPRETED_SIGN_CM)
        return INTERPRETED_SIGN_OUT_OF_SCOPE;
    if(kind!=INTERPRETED_COMP&&kind!=INTERPRETED_SIGN)
        return INTERPRETED_SIGN_OUT_OF_SCOPE;
    expected=(unsigned)(mission==INTERPRETED_SIGN_LM?
               (kind==INTERPRETED_COMP?07670u:07655u):
               (kind==INTERPRETED_COMP?07637u:07624u));
    mode=m->words[0163];
    if(m->words[5]!=expected||m->words[7]!=0||mode>=077777u||
       m->words[3]>03400u||(m->words[3]&0377u)!=0||
       m->words[6]!=(agc_word)((m->words[4]&076000u)|(m->words[3]>>8)))
        return INTERPRETED_SIGN_OUT_OF_SCOPE;
    if(kind==INTERPRETED_SIGN&&(operand==0||operand->high>077777u||operand->low>077777u))
        return INTERPRETED_SIGN_OUT_OF_SCOPE;
    m->words[1]=agc_extend(agc_correct(m->words[1]));
    if(kind==INTERPRETED_SIGN){
        sign_word=operand->high;
        cl=agc_classify(sign_word);
        m->words[0]=agc_diminished_abs(sign_word);
        if(cl==AGC_POSITIVE)goto done;
        if(cl==AGC_NEGATIVE)goto comp;
        sign_word=operand->low;
        cl=agc_classify(sign_word);
        m->words[0]=agc_diminished_abs(sign_word);
        if(cl!=AGC_NEGATIVE)goto done;
    }
comp:
    complement_pair(m,0);
    m->words[0]=agc_diminished_abs(mode);
    if(agc_classify(mode)==AGC_NEGATIVE){
        complement_pair(m,3);
        complement_pair(m,5);
    }else{
        m->words[0156]=agc_negate(m->words[0156]);
        m->words[0]=agc_extend(m->words[0156]);
    }
done:
    m->words[5]=(agc_word)(mission==INTERPRETED_SIGN_LM?06061u:06030u);
    return INTERPRETED_SIGN_OK;
}
