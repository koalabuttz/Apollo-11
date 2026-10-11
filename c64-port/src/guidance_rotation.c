/* SPDX-License-Identifier: MIT */
#include "guidance_rotation.h"
#include "guidance_product.h"
static void exchange(agc_register_pair *r,agc_word *high,agc_word *low){
    agc_word old_high=*high,old_low=*low;*high=agc_correct(r->a);*low=agc_correct(r->l);
    r->a=agc_extend(old_high);r->l=agc_extend(old_low);
}
static void load(agc_register_pair *r,const agc_word *pair,uint8_t negative){
    r->a=agc_extend(negative?agc_negate(pair[0]):pair[0]);
    r->l=agc_extend(negative?agc_negate(pair[1]):pair[1]);
}
static void multiply(p66_preparation *s,p66_scale_scratch *sc,const agc_word *pair){
    agc_dp factor={pair[0],pair[1]};agc_word mode=s->mpac[7];
    s->mpac[7]=0;(void)p66_scalar_product(s,sc,&factor);s->mpac[7]=mode;
}
static void double_register_pair(agc_register_pair *r){
    agc_register low=agc_add_register(r->l,r->l),high=agc_add_register(r->a,r->a);
    int carry=agc_overflow(low);
    if(carry>0)high=agc_add_register(high,1);
    else if(carry<0)high=agc_add_register(high,agc_extend(077776));
    r->a=high;r->l=agc_extend(agc_correct(low));
}
uint8_t p66_axis_rotate(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,
    p66_divide_scratch *d,p66_norm_scratch *n,p66_trig_scratch *t,p66_rotation_scratch *rot,
    agc_word direction,agc_word return_word){
    static const uint8_t indices[4]={4,2,0,4};
    agc_register_pair *r=&s->rate.registers;uint8_t stage,negative;
    if((direction!=3u&&direction!=077774u)||return_word>07777u)return 0;
    negative=(uint8_t)(direction==077774u);t->tem2=direction;rot->rtnsaver=return_word;
    for(stage=0;stage<3u;++stage){
        uint8_t axis=(uint8_t)(negative?2u-stage:stage),iteration;
        t->tem3=indices[axis];rot->dex2=indices[axis+1u];
        el->buf=1;load(r,&n->vbuf[t->tem3],1);
        for(iteration=0;iteration<2u;++iteration){
            uint8_t component;agc_dp term;
            if(iteration){r->a=0;exchange(r,&el->buf,&d->buf_low);}
            exchange(r,&s->mpac[0],&s->mpac[1]);n->addrwd=(agc_word)(0736u+t->tem3);
            multiply(s,sc,&t->sin[t->tem3]);
            if(!negative){r->a=(agc_register)(2u-stage);exchange(r,&s->mpac[0],&s->mpac[1]);}
            else load(r,&s->mpac[0],1);
            exchange(r,&s->mpac[3],&s->mpac[4]);n->addrwd=(agc_word)(0744u+t->tem3);
            component=el->buf? (uint8_t)rot->dex2 : (uint8_t)t->tem3;
            load(r,&n->vbuf[component],0);exchange(r,&s->mpac[0],&s->mpac[1]);
            multiply(s,sc,&t->cos[t->tem3]);exchange(r,&s->mpac[0],&s->mpac[1]);
            term.high=s->mpac[3];term.low=s->mpac[4];agc_double_add(r,&term);
            s->mpac[3]=term.high;s->mpac[4]=term.low;exchange(r,&s->mpac[3],&s->mpac[4]);
            double_register_pair(r);exchange(r,&n->vbuf[component],&n->vbuf[component+1u]);
            exchange(r,&el->buf,&d->buf_low);
        }
        t->tem2=(agc_word)(stage==2u?077777u:negative?077777u-(2u-stage):2u-stage);r->a=0;
    }
    return 1;
}
