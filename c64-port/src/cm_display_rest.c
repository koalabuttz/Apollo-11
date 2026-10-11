/* SPDX-License-Identifier: MIT */
#include "cm_display_rest.h"
enum exec_status cm_display_rest(apollo_overlay_state *s,cm_native_io *io)
{
    static const agc_word wake[3]={020764,020624,021037};
    uint16_t *w;agc_word mask,index,c;agc_register bits;
    if(s==0||io==0)return EXEC_OUT_OF_SCOPE;
    w=s->words;
    if((w[5]!=03343u&&w[5]!=03467u&&w[5]!=03473u)||w[4]!=020000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(020000u|(w[3]>>8))||w[7]!=0||w[0164]>2u||w[0162]>AGC_MASK||w[01042]>AGC_MASK||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
    c=w[01042];w[1]=agc_extend(agc_correct(w[1]));
    if(w[5]!=03473u&&c!=0&&c!=AGC_NEG_ZERO){w[0]=agc_diminished_abs(c);w[5]=05217;return EXEC_OK;}
    mask=(agc_word)(w[0162]&0704u);index=(agc_word)(mask&7u);mask&=077770u;
    w[1]=index;w[061]=mask;bits=agc_extend((agc_word)((AGC_MASK^w[074u+index])&mask));bits=agc_add_register(bits,agc_extend(w[074u+index]));w[074u+index]=agc_correct(bits);io->interrupt_enabled=1;
    w[0]=agc_extend(wake[w[0164]]);w[2]=03502;w[5]=04456;
    return EXEC_OK;
}
