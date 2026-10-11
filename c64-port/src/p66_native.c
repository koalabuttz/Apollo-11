/* SPDX-License-Identifier: MIT */
#include "p66_native.h"
#include <string.h>
#include <stddef.h>
#include "guidance_velocity.h"
#include "guidance_divide.h"
#include "guidance_store.h"
#ifndef P66_NATIVE_DISPATCH_ONLY
static const uint16_t preparation_addresses[41]={
    03746,03756,03644,03645,0,1,037,040,041,024,025,
    03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121
};
/* Explicit field views avoid structure padding, register truncation and any
 * persistent second copy of scratch/VAC. Physical input vectors remain const. */
static void preparation_view(p66_preparation *p,p66_scale_scratch *s,
                              uint16_t *v[41]) {
    unsigned i,k=0;
    v[k++]=&p->rate.count;v[k++]=&p->rate.scale;
    v[k++]=&p->rate.desired.high;v[k++]=&p->rate.desired.low;
    v[k++]=&p->rate.registers.a;v[k++]=&p->rate.registers.l;
    for(i=0;i<3;++i)v[k++]=&p->pipa[i];
    v[k++]=&p->time.high;v[k++]=&p->time.low;
    for(i=0;i<3;++i)v[k++]=&p->old_pipa[i];
    for(i=0;i<3;++i)v[k++]=&p->ruptreg[i];
    v[k++]=&p->this_piptime.high;v[k++]=&p->this_piptime.low;
    for(i=0;i<3;++i)v[k++]=&p->pipatmp[i];
    for(i=0;i<3;++i)v[k++]=&p->tem[i];
    for(i=0;i<8;++i)v[k++]=&p->mpac[i];
    for(i=0;i<6;++i)v[k++]=&p->delvrod[i];
    v[k++]=&s->mptemp;v[k]=&s->ovfind;
}
uint16_t p66_native_entry(apollo_overlay_state *image,uint16_t selector) {
    p66_preparation p; p66_scale_scratch s; p66_elapsed_state e;
    p66_divide_scratch d; p66_pushdown push;
    uint16_t *view[41],base=image->words[0120];unsigned i;
    if(selector<1 || selector>5)return P66_NATIVE_UNSUPPORTED;
#ifdef P66_NATIVE_ONLY_GROUP
    if(selector!=P66_NATIVE_ONLY_GROUP)return P66_NATIVE_UNSUPPORTED;
#endif
    if(base<0401 || base>0661 || (base-0401u)%054u!=0 ||
       image->words[base-1u]!=0 ||
       (image->words[0167]&0777u)!=base)return P66_NATIVE_BAD_VAC;
    if(selector!=P66_NATIVE_PREPARE && image->words[0163]!=
       (selector==P66_NATIVE_SCALE_TIME || selector==P66_NATIVE_VELOCITY ? 077776u : 0u))
        return P66_NATIVE_BAD_MODE;
    if(image->words[0166]!=(selector==P66_NATIVE_PREPARE ||
       selector==P66_NATIVE_SCALE_TIME ? base : selector==P66_NATIVE_VELOCITY ? base+8u : base+6u))
        return P66_NATIVE_BAD_VAC;
    preparation_view(&p,&s,view);
    for(i=0;i<41;++i)*view[i]=image->words[preparation_addresses[i]];
    push.base=base;push.pushloc=image->words[0166];
    for(i=0;i<P66_VAC_WORDS;++i)push.words[i]=image->words[base+i];
    e.piptime.high=image->words[01234];e.piptime.low=image->words[01235];e.buf=image->words[0130];
    d.buf_low=image->words[0131];d.dvsign=image->words[0136];
    d.dvnormct=image->words[0137];d.maxdvsw=image->words[0140];
    switch(selector) {
#if !defined(P66_NATIVE_ONLY_GROUP) || P66_NATIVE_ONLY_GROUP==1
    case P66_NATIVE_PREPARE:(void)p66_prepare(&p);break;
#endif
#if !defined(P66_NATIVE_ONLY_GROUP) || P66_NATIVE_ONLY_GROUP==2
    case P66_NATIVE_SCALE_TIME:
        p66_scale_velocity(&p,&s);(void)p66_push_timestamp(&p,&push);break;
#endif
#if !defined(P66_NATIVE_ONLY_GROUP) || P66_NATIVE_ONLY_GROUP==3
    case P66_NATIVE_ELAPSED_STORE:
        (void)p66_elapsed_time(&p,&s,&e);(void)p66_store_elapsed(&p,&push);break;
#endif
#if !defined(P66_NATIVE_ONLY_GROUP) || P66_NATIVE_ONLY_GROUP==4
    case P66_NATIVE_DIVIDE_GRAVITY:
        (void)p66_divide_four_seconds(&p,&s,&e,&d);
        (void)p66_push_gravity(&p,&push,&image->words[01236]);break;
#endif
#if !defined(P66_NATIVE_ONLY_GROUP) || P66_NATIVE_ONLY_GROUP==5
    case P66_NATIVE_VELOCITY:
        (void)p66_update_velocity(&p,&s,&e,&push,&image->words[02630],
                                 &image->words[03526],&image->words[03520]);break;
#endif
    default:return P66_NATIVE_UNSUPPORTED;
    }
    for(i=0;i<41;++i)image->words[preparation_addresses[i]]=*view[i];
    image->words[0166]=push.pushloc;
    for(i=0;i<P66_VAC_WORDS;++i)image->words[base+i]=push.words[i];
    image->words[0130]=e.buf;image->words[0131]=d.buf_low;
    image->words[0136]=d.dvsign;image->words[0137]=d.dvnormct;
    image->words[0140]=d.maxdvsw;
    /* Final compiled operand addressing at original pair-consumed DANZIG.
     * These are source data addresses, not fetched/decoded instructions. */
    if(selector!=P66_NATIVE_PREPARE) {
        image->words[0116]=selector==2 ? 01761u : selector==3 ?
            (agc_word)(base+30u) : selector==4 ? 01636u : 01520u;
        image->words[023]=0;image->words[7]=0;
        image->words[6]=(agc_word)(selector==4 ?
            ((image->words[0165]&076000u)|2u) : (image->words[0165]&076007u));
        image->words[3]=(agc_word)((image->words[6]&7u)<<8);
        image->words[4]=(agc_word)(image->words[6]&076000u);
    }
    return P66_NATIVE_OK;
}
#endif
#ifndef P66_NATIVE_ARITH_ONLY
/* Keep the canonical byte copy compact on a6502. Volatile stores prevent
 * whole-program unrolling into hundreds of fixed-address load/store pairs;
 * all copied words retain their raw bits. */
static __attribute__((noinline)) void source_copy(void *to,const void *from,uint16_t bytes){
 volatile unsigned char *d=to;const unsigned char *p=from;
 while(bytes!=0){*d++=*p++;--bytes;}
}
typedef struct{uint16_t address,offset;} executive_field;
#define EF(a,m) {a,(uint16_t)offsetof(executive,m)}
static const executive_field executive_fields[]={
 EF(0067,newjob),EF(0063,newprio),EF(0065,newloc[0]),EF(0066,newloc[1]),
 EF(0064,locctr),EF(0120,fixloc),EF(0121,ovfind)
};
#undef EF
static __attribute__((noinline)) void scalar_exchange(apollo_overlay_state *m,executive *s,uint8_t to_image){
 uint8_t i;
 for(i=0;i<sizeof executive_fields/sizeof executive_fields[0];++i){
  const executive_field *f=&executive_fields[i];
  uint16_t *v=(uint16_t *)((unsigned char *)s+f->offset);
  if(to_image)m->words[f->address]=*v;else *v=m->words[f->address];
 }
}
void p66_native_export_executive(apollo_overlay_state *m,const executive *s) {
 source_copy(&m->words[0154],s->core,sizeof s->core);
 source_copy(&m->words[0400],s->vac,sizeof s->vac);
 scalar_exchange(m,(executive *)s,1);
}
void p66_native_import_executive(executive *s,const apollo_overlay_state *m) {
 source_copy(s->core,&m->words[0154],sizeof s->core);
 source_copy(s->vac,&m->words[0400],sizeof s->vac);
 scalar_exchange((apollo_overlay_state *)m,s,0);
}
__attribute__((noinline)) enum exec_action p66_native_run_group(executive *s,p66_native_binding *b,
    exec_address *resume,uint16_t selector,agc_word completed,agc_word next) {
    uint8_t token=b->producers->enter(b->producers->user);
    resume->location=next;
    if(selector!=0) {
        p66_native_export_executive(b->image,s);
        if(b->invoke!=0)b->status=(enum p66_native_status)b->invoke(b->modules,b->image,selector);
#ifndef P66_NATIVE_DISPATCH_ONLY
        else b->status=(enum p66_native_status)p66_native_entry(b->image,selector);
#else
        else b->status=P66_NATIVE_UNSUPPORTED;
#endif
        p66_native_import_executive(s,b->image);
        if(b->status==P66_NATIVE_OK) {
            if(selector!=1) {
                s->core[0][EXEC_LOC]=completed;
                p66_native_export_executive(b->image,s);
            }
            if(b->observe!=0)b->observe(b->observer,b->image,completed);
        }
    }else b->status=P66_NATIVE_BOUNDARY;
    if(b->status!=P66_NATIVE_OK)b->runtime->fault=EXEC_OUT_OF_SCOPE;
    resume->bank=b->runtime->current.bank;
    b->producers->leave(b->producers->user,token);
    return selector==1 ? EXEC_ENTER_INTERPRETER : EXEC_YIELD;
}
static enum exec_action continuation(executive *s,void *user,exec_address *resume) {
    p66_native_binding *b=user;uint16_t selector=0;agc_word completed=0,next=resume->location;
    switch(b->runtime->current.location) {
    case 03275:selector=1;completed=03353;next=03354;break;
    case 03354:selector=2;completed=03356;next=03357;break;
    case 03357:selector=3;completed=03361;next=03362;break;
    case 03362:selector=4;completed=03364;next=03365;break;
    case 03365:selector=5;completed=03373;next=03374;break;
    default:break;
    }
    return p66_native_run_group(s,b,resume,selector,completed,next);
}
#define PAIR(bank) \
 {{03275,bank},EXEC_CONTEXT_BASIC,continuation},\
 {{03354,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03357,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03362,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03365,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03374,bank},EXEC_CONTEXT_INTERPRETIVE,continuation}
const exec_context_mapping p66_native_mappings[12]={PAIR(064007),PAIR(064067)};
enum exec_status p66_native_step(p66_native_binding *b) {
    enum exec_status status;uint8_t token;
    if(b==0 || b->image==0 || b->runtime==0 || b->producers==0 ||
       b->producers->enter==0 || b->producers->leave==0)return EXEC_OUT_OF_SCOPE;
    token=b->producers->enter(b->producers->user);
    p66_native_import_executive(b->runtime->state,b->image);
    b->producers->leave(b->producers->user,token);
    status=exec_native_step(b->runtime);
    token=b->producers->enter(b->producers->user);
    p66_native_export_executive(b->image,b->runtime->state);
    if(status==EXEC_OK){
        /* DANZIG restores BANKSET before its NEWJOB check. Materialize the
         * selected source bank after the callback observation, so a pending
         * basic job inherits the actual entry bank rather than the kernel's
         * pre-DANZIG fixed/erasable scratch bank. */
        agc_word bank=(agc_word)(b->runtime->current.bank&076007u);
        b->image->words[6]=bank;b->image->words[3]=(agc_word)((bank&7u)<<8);
        b->image->words[4]=(agc_word)(bank&076000u);
    }
    b->image->words[0115]=b->runtime->interpreter_control.intbit15;
    b->image->words[023]=b->runtime->interpreter_control.edop;
    b->producers->leave(b->producers->user,token);
    return status;
}

#endif
