/* SPDX-License-Identifier: MIT */
#include "dsky_nv_return.h"
enum p66_bias_status dsky_nv_return(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission)
{
    agc_word ret,bank;agc_register a,l;uint16_t *w;
    if(s==0||io==0||(mission!=DSKY_NV_LM&&mission!=DSKY_NV_CM))return P66_BIAS_OUT_OF_SCOPE;
    w=s->words;
    if(w[5]!=(mission==DSKY_NV_LM?04202u:04216u)||w[7]!=0||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u)return P66_BIAS_OUT_OF_SCOPE;
    ret=w[01037];bank=w[01040];
    if(ret<02000u||ret>07777u||(bank&~076167u))return P66_BIAS_OUT_OF_SCOPE;
    a=w[0];l=w[1];w[01037]=agc_correct(a);w[01040]=agc_correct(l);
    w[0]=agc_extend(bank);w[1]=agc_extend(ret);io->superbank=(agc_word)(bank&0160u);
    w[3]=(agc_word)((bank&7u)<<8);w[4]=(agc_word)(bank&076000u);w[6]=(agc_word)(w[4]|(bank&7u));w[2]=2;w[5]=ret;
    return P66_BIAS_OK;
}
