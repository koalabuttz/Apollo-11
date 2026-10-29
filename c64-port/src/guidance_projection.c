/* SPDX-License-Identifier: MIT */
#include "guidance_projection.h"
#include "guidance_dot.h"
static uint8_t valid(const p66_preparation *s,const p66_pushdown *p,uint8_t offset){
    return (uint8_t)(s->mpac[7]==0 && p->base>=0401 && p->base<=0661 &&
        (uint16_t)(p->base-0401u)%054u==0 && p->pushloc==p->base+offset);
}
enum p66_projection_status p66_projection_dot(p66_preparation *s,p66_scale_scratch *sc,
    p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,
    p66_trig_scratch *t,p66_rotation_scratch *rotation,const agc_word cdu[3]){
    static const uint8_t offset[3]={0,3,5};agc_word operand[6],old_z_low;uint8_t i;
    if(!valid(s,p,0))return P66_PROJECTION_OUT_OF_SCOPE;
    p->words[0]=s->mpac[0];p->words[1]=s->mpac[1];p->pushloc=(agc_word)(p->base+2u);
    old_z_low=s->mpac[6];s->mpac[0]=020000;s->mpac[1]=0;
    for(i=3;i<7u;++i)s->mpac[i]=0;
    s->mpac[7]=077776;s->rate.registers.a=agc_extend(077776);s->rate.registers.l=agc_extend(old_z_low);
    /* CALL saves the source P66 continuation in owned QPRET, not a C PC. */
    n->addrwd=06520;p->words[42]=065426;
    (void)p66_cdu_trig(s,sc,d,n,t,cdu,03663,046007);
    for(i=0;i<3u;++i){n->vbuf[2u*i]=s->mpac[offset[i]];n->vbuf[2u*i+1u]=s->mpac[offset[i]+1u];}
    (void)p66_axis_rotate(s,sc,el,d,n,t,rotation,3,03656);
    old_z_low=s->mpac[6];for(i=0;i<3u;++i){s->mpac[offset[i]]=n->vbuf[2u*i];s->mpac[offset[i]+1u]=n->vbuf[2u*i+1u];}
    s->mpac[7]=077776;s->rate.registers.a=agc_extend(077776);s->rate.registers.l=agc_extend(old_z_low);
    /* RVQ consumes the pending order without a NEWJOB check. DOT14 is
     * the first completed-pair scheduling boundary after this basic call. */
    t->polish=065426;n->addrwd=(agc_word)(p->base+14u);
    for(i=0;i<6u;++i)operand[i]=p->words[14u+i];
    (void)p66_dot(s,sc,el,d,&n->buf_third,operand);n->addrwd=(agc_word)(p->base+18u);return P66_PROJECTION_OK;
}
enum p66_projection_status p66_projection_store(p66_preparation *s,p66_norm_scratch *n,p66_pushdown *p){
    agc_word old_lo;if(!valid(s,p,2))return P66_PROJECTION_OUT_OF_SCOPE;
    old_lo=p->words[23];p->words[22]=s->mpac[0];p->words[23]=s->mpac[1];
    /* STORE finishes through CCS MODE: scalar +0 leaves A +0. */
    s->rate.registers.a=0;s->rate.registers.l=agc_extend(old_lo);n->addrwd=(agc_word)(p->base+22u);return P66_PROJECTION_OK;
}
enum p66_projection_status p66_projection_acceleration(p66_preparation *s,p66_scale_scratch *sc,
    p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,agc_dp *afc){
    static const uint8_t offset[3]={0,3,5};agc_dp operand;agc_word old_z_low;uint8_t i;
    if(!valid(s,p,2))return P66_PROJECTION_OUT_OF_SCOPE;
    p->pushloc=p->base;n->addrwd=p->base;operand.high=p->words[0];operand.low=p->words[1];
    (void)p66_dp_divide(s,sc,el,d,operand,1);afc->high=s->mpac[0];afc->low=s->mpac[1];old_z_low=s->mpac[6];
    for(i=0;i<3u;++i){s->mpac[offset[i]]=s->delvrod[2u*i];s->mpac[offset[i]+1u]=s->delvrod[2u*i+1u];}
    s->mpac[7]=077776;s->rate.registers.a=agc_extend(077776);s->rate.registers.l=agc_extend(old_z_low);n->addrwd=01766;return P66_PROJECTION_OK;
}
enum p66_projection_status p66_projection(p66_preparation *s,p66_scale_scratch *sc,
    p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,
    p66_trig_scratch *t,p66_rotation_scratch *rotation,const agc_word cdu[3],agc_dp *afc){
    enum p66_projection_status result=p66_projection_dot(s,sc,el,d,n,p,t,rotation,cdu);
    if(result!=P66_PROJECTION_OK)return result;
    (void)p66_projection_store(s,n,p);return p66_projection_acceleration(s,sc,el,d,n,p,afc);
}
