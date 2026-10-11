/* SPDX-License-Identifier: MIT */
#include "guidance_push.h"
enum p66_push_status p66_push_timestamp(p66_preparation *s,p66_pushdown *p) {
    uint16_t k;
    agc_word old1,old2,old3,old4,old5;
    if(s->mpac[7]!=077776 || p->base<0401 || p->base>0661 ||
       (uint16_t)(p->base-0401u)%054u!=0 || p->pushloc<p->base ||
       (uint16_t)(p->pushloc-p->base)>P66_VAC_WORDS-6u)
        return P66_PUSH_OUT_OF_SCOPE;
    k=(uint16_t)(p->pushloc-p->base);
    old1=p->words[k+1u]; old2=p->words[k+2u]; old3=p->words[k+3u];
    old4=p->words[k+4u]; old5=p->words[k+5u];
    p->words[k]=s->mpac[0]; p->words[k+1u]=s->mpac[1];
    p->words[k+2u]=s->mpac[3]; p->words[k+3u]=s->mpac[4];
    p->words[k+4u]=s->mpac[5]; p->words[k+5u]=s->mpac[6];
    p->pushloc=(agc_word)(p->pushloc+6u);
    s->mpac[0]=s->this_piptime.high; s->mpac[1]=s->this_piptime.low;
    /* CCS MODE discards the old first pushed word. The subsequent DXCH
     * operations retain the other old pushdown words in MPAC and A/L. */
    s->mpac[2]=0; s->mpac[3]=0; s->mpac[4]=old1;
    s->mpac[5]=old2; s->mpac[6]=old3; s->mpac[7]=0;
    s->rate.registers.a=agc_extend(old4); s->rate.registers.l=agc_extend(old5);
    return P66_PUSH_OK;
}
