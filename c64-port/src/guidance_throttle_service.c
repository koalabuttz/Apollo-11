/* SPDX-License-Identifier: MIT */
#include "guidance_throttle_service.h"
#include "guidance_product.h"
#include "p66_initialization.h"
/* The source's basic hardware division is supplied as independent native
 * fixed-point arithmetic; no instruction decoder or engine participates. */
#include "agc_hardware_divide.h"
static agc_register carry(int sign){return sign>0?1:0177776;}
static void exchange(agc_register_pair *r,agc_dp *p){agc_dp old=*p;p->high=agc_correct(r->a);p->low=agc_correct(r->l);r->a=agc_extend(old.high);r->l=agc_extend(old.low);}
static void load(agc_register_pair *r,agc_dp p,uint8_t negative){r->a=agc_extend(negative?agc_negate(p.high):p.high);r->l=agc_extend(negative?agc_negate(p.low):p.low);}
static void multiply(p66_preparation *s,p66_scale_scratch *sc,agc_dp factor){agc_word mode=s->mpac[7];s->mpac[7]=0;(void)p66_scalar_product(s,sc,&factor);s->mpac[7]=mode;}
static void tp_agree(p66_preparation *s){
    agc_register a=0;agc_word limit;int sign=0,ov;uint8_t i;
    for(i=0;i<3u;++i){sign=agc_to_int(s->mpac[i]);if(sign)break;}
    s->rate.registers.l=02410;
    if(!sign){s->mpac[0]=s->mpac[1]=s->mpac[2]=0;s->rate.registers.a=0;return;}
    limit=sign>0?037777:040000;a=agc_add_register(agc_extend(limit),carry(sign));
    for(i=2;i>0;--i){a=agc_add_register(a,agc_extend(s->mpac[i]));s->mpac[i]=agc_correct(a);ov=agc_overflow(a);a=agc_add_register(ov?carry(ov):0,agc_extend(limit));}
    a=agc_add_register(a,agc_extend(s->mpac[0]));s->mpac[0]=agc_correct(a);ov=agc_overflow(a);
    if(ov)a=carry(ov);else s->mpac[1]=agc_correct(a);s->rate.registers.a=a;
}
static void massmult(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,
    p66_norm_scratch *n,const agc_dp *mass,agc_word return_word){
    static const agc_dp scale={00307,017534};agc_dp accumulator={s->mpac[0],s->mpac[1]};
    el->buf=return_word;exchange(&s->rate.registers,&accumulator);s->mpac[0]=accumulator.high;s->mpac[1]=accumulator.low;
    n->addrwd=01244;multiply(s,sc,*mass);n->addrwd=02005;multiply(s,sc,scale);tp_agree(s);
    s->rate.registers.a=agc_extend(s->mpac[0]);
    if(agc_to_int(s->mpac[0]))s->rate.registers.a=037777;
    else{accumulator.high=s->mpac[1];accumulator.low=s->mpac[2];exchange(&s->rate.registers,&accumulator);s->mpac[1]=accumulator.high;s->mpac[2]=accumulator.low;}
}
static agc_register subtract(agc_word x,agc_word y){return agc_add_register(agc_extend(x),agc_extend(agc_negate(y)));}
static uint8_t nonpositive(agc_register a){return (uint8_t)((a&0100000u)||a==0);}
uint8_t p66_throttle_service(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,
    p66_divide_scratch *d,p66_norm_scratch *n,p66_throttle_feedback *f,
    const p66_throttle_inputs *in,p66_throttle_service_state *t){
    agc_register_pair *r=&s->rate.registers;agc_register a;agc_dp critical;uint16_t period;int ov;
    if(t->bbank!=062007u||t->q>07777u||t->superbank>0160u||(t->superbank&0117u)||t->interrupt_enabled>1u)return 0;
    t->rtnhold=t->q;massmult(s,sc,el,n,&in->mass,02224);exchange(r,&t->fp);
    load(r,f->afc,0);massmult(s,sc,el,n,&in->mass,02230);t->fc=agc_correct(r->a);exchange(r,&t->fcodd);
    a=subtract(s->time.low,t->tthrot);a=agc_extend((agc_word)(a&037777u));a=agc_add_register((agc_register)(a^0177777u),agc_extend(0454));
    if(!nonpositive(a)){load(r,f->fweight,0);agc_double_add(r,&t->fp);}
    critical.high=t->lowcrit;critical.low=t->highcrit;load(r,critical,0);
    critical.high=el->buf;critical.low=d->buf_low;exchange(r,&critical);el->buf=critical.high;d->buf_low=critical.low;
    t->pifpset=077777;a=subtract(t->fcold,d->buf_low);
    if(!nonpositive(a)){
        a=subtract(t->fcodd.high,el->buf);
        if(nonpositive(a)){a=subtract(t->fp.high,07401);t->pifpset=agc_correct(a);}
        else{r->a=agc_extend(t->fp.high);exchange(r,&t->fcodd);t->pifpset=010000;}
    }else{
        a=subtract(t->fcodd.high,d->buf_low);
        if(!nonpositive(a)){r->a=06613;exchange(r,&t->fcodd);t->pifpset=010000;}
    }
    {
        p66_basic_initialization phase={0};phase.q=02303;phase.bbank=t->bbank;phase.superbank=t->superbank;phase.interrupt_enabled=t->interrupt_enabled;phase.phase3_address[0]=t->phase3[0];phase.phase3_address[1]=t->phase3[1];
        (void)p66_fastchng(&phase);t->phase3[0]=phase.phase3_address[0];t->phase3[1]=phase.phase3_address[1];r->a=phase.a;r->l=phase.l;t->q=phase.q;
    }
    load(r,t->fcodd,0);t->fcold=agc_correct(r->a);exchange(r,&t->pif);load(r,t->fp,1);agc_double_add(r,&t->pif);
    r->a=agc_add_register(agc_extend(t->pif.high),agc_extend(t->pifpset));t->pseudo55=agc_correct(r->a);ov=agc_overflow(r->a);if(!ov)t->thrust=agc_correct(r->a);
    t->channel14=(agc_word)(t->channel14|010u);t->tthrot=s->time.low;
    el->buf=s->this_piptime.low;a=subtract(0102,t->modreg);
    period=200;if(a!=0&&a!=0177777u){el->buf=el->piptime.low;period=400;}t->q=period;
    *r=agc_multiply(period,040);{agc_register old=agc_extend(d->buf_low);d->buf_low=agc_correct(r->l);r->l=old;}
    a=agc_add_register(subtract(s->time.low,el->buf),agc_extend(024));r->a=(agc_register)(a&0377u);r->l=0;
    *r=agc_hardware_divide(*r,agc_extend(period));*r=agc_multiply(r->a,t->pif.high);r->a=agc_add_register(r->a,r->a);exchange(r,&f->fweight);
    a=agc_extend(agc_from_int((int16_t)(agc_to_int(t->pif.high)<0?-agc_to_int(t->pif.high):agc_to_int(t->pif.high))));
    *r=agc_multiply(a,t->pif.high);*r=agc_hardware_divide(*r,agc_extend(d->buf_low));r->l=0;agc_double_add(r,&f->fweight);
    return 1;
}
