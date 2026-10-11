/* SPDX-License-Identifier: MIT */
#include "guidance_throttle.h"
#include "guidance_dp_divide.h"
#include "guidance_product.h"
#include "guidance_sum.h"
#include "guidance_vector.h"
static agc_dp pair(const p66_pushdown *p,uint8_t offset){agc_dp x={p->words[offset],p->words[offset+1u]};return x;}
static void push_load(p66_preparation *s,p66_pushdown *p,agc_dp source,uint8_t load){
    uint8_t offset=(uint8_t)(p->pushloc-p->base);agc_word old_low=p->words[offset+1u];
    p->words[offset]=s->mpac[0];p->words[offset+1u]=s->mpac[1];p->pushloc=(agc_word)(p->pushloc+2u);
    if(load){s->mpac[0]=source.high;s->mpac[1]=source.low;s->mpac[2]=0;}
    s->rate.registers.a=0;s->rate.registers.l=agc_extend(old_low);
}
static agc_dp pop(p66_pushdown *p){p->pushloc=(agc_word)(p->pushloc-2u);return pair(p,(uint8_t)(p->pushloc-p->base));}
static void load(p66_preparation *s,agc_dp source){agc_word low=s->mpac[1];s->mpac[0]=source.high;s->mpac[1]=source.low;s->mpac[2]=0;s->mpac[7]=0;s->rate.registers.a=0;s->rate.registers.l=agc_extend(low);}
static void store_load(p66_preparation *s,agc_dp *destination,agc_dp source){destination->high=s->mpac[0];destination->low=s->mpac[1];load(s,source);}
static uint8_t negative(const p66_preparation *s){uint8_t i;for(i=0;i<3u;++i){int value=agc_to_int(s->mpac[i]);if(value)return (uint8_t)(value<0);}return 0;}
enum p66_feedback_status p66_throttle_feedback_compute(p66_preparation *s,p66_scale_scratch *sc,
    p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,
    p66_throttle_feedback *f,const p66_throttle_inputs *in,const agc_word bias[6]){
    static const agc_dp kpip={00321,026706},shftfact={0,04000},bit1h={1,0},scalefac={00307,017534};
    agc_dp operand;enum p66_feedback_status status=P66_FEEDBACK_NORMAL;
    if(s->mpac[7]!=077776||p->base<0401||p->base>0661||(uint16_t)(p->base-0401u)%054u!=0||p->pushloc!=p->base)return P66_FEEDBACK_OUT_OF_SCOPE;
    (void)p66_vector_multiply(s,sc,&kpip);(void)p66_vector_add(s,sc,el,bias);
    if(p66_vector_length(s,sc,el,d,n,p)!=P66_NORM_OK)return P66_FEEDBACK_ALARM_1302;
    push_load(s,p,s->this_piptime,1);(void)p66_scalar_subtract(s,sc,el,&f->lasttpip);
    push_load(s,p,s->this_piptime,1);operand=pop(p);store_load(s,&f->lasttpip,operand);
    (void)p66_dp_divide(s,sc,el,d,shftfact,0);operand=pop(p);(void)p66_dp_divide(s,sc,el,d,operand,1);
    push_load(s,p,f->fweight,1);(void)p66_scalar_product(s,sc,&bit1h);
    (void)p66_dp_divide(s,sc,el,d,in->mass,0);(void)p66_dp_divide(s,sc,el,d,scalefac,0);
    operand=pair(p,0);(void)p66_scalar_add(s,sc,el,&operand);push_load(s,p,pair(p,20),1);
    (void)p66_dp_divide(s,sc,el,d,pair(p,22),0);operand=pop(p);(void)p66_scalar_subtract(s,sc,el,&operand);
    (void)p66_scalar_product(s,sc,&in->lag_tau);(void)p66_scalar_add(s,sc,el,&f->afc);
    push_load(s,p,in->maxforce,1);(void)p66_dp_divide(s,sc,el,d,in->mass,0);
    push_load(s,p,in->minforce,1);(void)p66_dp_divide(s,sc,el,d,in->mass,0);
    push_load(s,p,bit1h,0);operand=pair(p,2);(void)p66_scalar_reverse_subtract(s,sc,el,&operand);
    if(negative(s)){status=P66_FEEDBACK_MINIMUM;}
    else{
        load(s,pop(p));load(s,pop(p));push_load(s,p,bit1h,0);operand=pair(p,2);(void)p66_scalar_reverse_subtract(s,sc,el,&operand);
        if(!negative(s))status=P66_FEEDBACK_MAXIMUM;
        else load(s,pop(p));
    }
    load(s,pop(p));p->pushloc=(agc_word)(p->base+2u);operand=pop(p);store_load(s,&f->afc,operand);n->addrwd=p->base;
    return status;
}
