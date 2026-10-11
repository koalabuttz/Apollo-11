/* SPDX-License-Identifier: MIT */
#include "irq_delivery.h"
static int mixed(const exec_native *r){return r->context_mapping!=0 || r->resolve!=0;}
static exec_continuation lookup_kind(const exec_native *r, exec_address a,
                                     enum exec_context_kind kind) {
    uint8_t i;
    if(r->resolve!=0)return r->resolve(r->registry_user,a,kind);
    if (r->context_mapping!=0) {
        for(i=0;i<r->count;++i)
            if(r->context_mapping[i].address.location==a.location &&
               r->context_mapping[i].address.bank==a.bank &&
               r->context_mapping[i].kind==kind) return r->context_mapping[i].call;
        return 0;
    }
    if(kind!=EXEC_CONTEXT_BASIC) return 0;
    for (i=0; i<r->count; ++i)
        if (r->mapping[i].address.location == a.location &&
            r->mapping[i].address.bank == a.bank) return r->mapping[i].call;
    return 0;
}
static exec_continuation lookup(const exec_native *r,exec_address a) {
    return lookup_kind(r,a,r->current_kind);
}
int exec_native_is_mapped(const exec_native *r,exec_address a,
                          enum exec_context_kind kind) {
    return lookup_kind(r,a,kind)!=0;
}
static enum exec_status settle(exec_native *r, enum exec_status status) {
    if (status == EXEC_IDLE) { r->running=0; return status; }
    if (status == EXEC_OK && lookup(r,r->current) == 0) status=EXEC_UNMAPPED;
    if (status != EXEC_OK) { r->fault=status; r->running=0; }
    return status;
}
enum exec_status exec_native_init(exec_native *r, executive *s,
                                 const exec_mapping *m, uint8_t count, void *user) {
    uint8_t i,j;
    if (r == 0 || s == 0 || m == 0 || count == 0) return EXEC_OUT_OF_SCOPE;
    r->state=s; r->mapping=m; r->count=count; r->user=user;
    r->requests=0; r->exclusion=0;
    r->services=0; r->service_user=0;
    r->resolve=0; r->registry_user=0;
    r->requested_priority=0; r->in_callback=0;
    r->context_mapping=0; r->current_kind=EXEC_CONTEXT_BASIC;
    r->interpreter_control.intbit15=0; r->interpreter_control.edop=0;
    r->current.location=0; r->current.bank=0; r->running=0; r->fault=EXEC_OK;
    for (i=0; i<count; ++i) {
        if (m[i].call == 0 || m[i].address.location == 0 || m[i].address.location > 07777 ||
            m[i].address.bank > AGC_MASK) return settle(r,EXEC_OUT_OF_SCOPE);
        for (j=0; j<i; ++j)
            if (m[i].address.location == m[j].address.location &&
                m[i].address.bank == m[j].address.bank)
                return settle(r,EXEC_OUT_OF_SCOPE);
    }
    return EXEC_OK;
}
enum exec_status exec_native_context_init(exec_native *r,executive *s,
                                         const exec_context_mapping *m,
                                         uint8_t count,void *user) {
    uint8_t i,j;
    if(r==0 || s==0 || m==0 || count==0) return EXEC_OUT_OF_SCOPE;
    r->state=s; r->mapping=0; r->context_mapping=m; r->count=count; r->user=user;
    r->requests=0; r->exclusion=0; r->requested_priority=0; r->in_callback=0;
    r->services=0; r->service_user=0;
    r->resolve=0; r->registry_user=0;
    r->current.location=0; r->current.bank=0; r->running=0; r->fault=EXEC_OK;
    r->current_kind=EXEC_CONTEXT_BASIC;
    r->interpreter_control.intbit15=0; r->interpreter_control.edop=0;
    for(i=0;i<count;++i) {
        if(m[i].call==0 || m[i].address.location==0 || m[i].address.location>07777 ||
           (m[i].address.bank & (agc_word)~UINT16_C(076167))!=0 ||
           (m[i].kind!=EXEC_CONTEXT_BASIC && m[i].kind!=EXEC_CONTEXT_INTERPRETIVE))
            return settle(r,EXEC_OUT_OF_SCOPE);
        for(j=0;j<i;++j)
            if(m[i].address.location==m[j].address.location &&
               m[i].address.bank==m[j].address.bank && m[i].kind==m[j].kind)
                return settle(r,EXEC_OUT_OF_SCOPE);
    }
    return EXEC_OK;
}
enum exec_status exec_native_resolver_init(exec_native *r,executive *s,
 exec_continuation (*resolve)(void *,exec_address,enum exec_context_kind),void *registry_user,void *callback_user){
    if(r==0 || s==0 || resolve==0)return EXEC_OUT_OF_SCOPE;
    r->state=s;r->mapping=0;r->context_mapping=0;r->count=0;r->user=callback_user;
    r->requests=0;r->exclusion=0;r->services=0;r->service_user=0;
    r->resolve=resolve;r->registry_user=registry_user;
    r->requested_priority=0;r->in_callback=0;r->current.location=0;r->current.bank=0;
    r->running=0;r->fault=EXEC_OK;r->current_kind=EXEC_CONTEXT_BASIC;
    r->interpreter_control.intbit15=0;r->interpreter_control.edop=0;
    return EXEC_OK;
}
static enum exec_status fail(exec_native *r, enum exec_status status) {
    if (status != EXEC_OK) { r->fault=status; r->running=0; }
    return status;
}
enum exec_status exec_native_bind_services(exec_native *r,const exec_native_services *services,void *user) {
    if(r==0 || services==0 || services->prepare==0 || services->finish==0 ||
       r->running!=0 || r->in_callback!=0 || r->fault!=EXEC_OK)return EXEC_OUT_OF_SCOPE;
    r->services=services;r->service_user=user;return EXEC_OK;
}
#if !defined(EXEC_NATIVE_FINISH_FAMILY)||EXEC_NATIVE_FINISH_FAMILY==2
static uint8_t candidate(const executive *s,agc_word threshold) {
    uint8_t i,slot=0;
    for(i=1;i<EXEC_CORE_COUNT;++i)
        if(agc_classify(s->core[i][EXEC_PRIORITY])==AGC_POSITIVE &&
           s->core[i][EXEC_PRIORITY]>threshold) {
            threshold=s->core[i][EXEC_PRIORITY]; slot=i;
        }
    return slot;
}
#endif
static enum exec_status saved_target(const exec_native *r,uint8_t slot,
                                     exec_address *a,enum exec_context_kind *kind) {
    const executive *s=r->state;
    a->location=s->core[slot][EXEC_LOC]; a->bank=s->core[slot][EXEC_BANK];
    *kind=(a->location & AGC_SIGN) ? EXEC_CONTEXT_INTERPRETIVE : EXEC_CONTEXT_BASIC;
    if(agc_classify(s->core[slot][EXEC_PRIORITY])!=AGC_POSITIVE ||
       (a->bank & (agc_word)~UINT16_C(076167))!=0) return EXEC_OUT_OF_SCOPE;
    if(*kind==EXEC_CONTEXT_INTERPRETIVE) {
        if(agc_negate(a->location)>=07777) return EXEC_OUT_OF_SCOPE;
        a->location=agc_add_word(agc_negate(a->location),1);
    } else if(a->location==0 || a->location>07777) return EXEC_OUT_OF_SCOPE;
    return lookup_kind(r,*a,*kind)==0 ? EXEC_UNMAPPED : EXEC_OK;
}
/* CHANJOB+4 and ENDPRCHG/INTRSM, shared by mixed basic dispatch boundaries.
 * Preflight is complete before this mutates the saved contexts. */
static void exchange_saved(exec_native *r,uint8_t slot,exec_address outgoing,
                            exec_address incoming,enum exec_context_kind kind) {
    executive *s=r->state;
    agc_word debris=s->core[0][EXEC_BANK];
    uint8_t j;
    s->core[slot][EXEC_LOC]=outgoing.location;
    s->core[slot][EXEC_BANK]=outgoing.bank;
    for(j=0;j<8;++j) {
        agc_word old=s->core[0][j];
        s->core[0][j]=s->core[slot][j]; s->core[slot][j]=old;
    }
    if(s->ovfind!=0 && s->ovfind!=AGC_NEG_ZERO)
        s->core[0][EXEC_PUSH]=agc_negate(s->core[0][EXEC_PUSH]);
    for(j=EXEC_PUSH;j<=EXEC_PRIORITY;++j) {
        agc_word old=s->core[0][j];
        s->core[0][j]=s->core[slot][j]; s->core[slot][j]=old;
    }
    s->fixloc=(agc_word)(s->core[0][EXEC_PRIORITY]&EXEC_LOW9);
    s->ovfind=0;
    if(s->core[0][EXEC_PUSH]&AGC_SIGN) {
        s->core[0][EXEC_PUSH]=agc_negate(s->core[0][EXEC_PUSH]); s->ovfind=1;
    }
    s->newjob=0;
    s->core[0][EXEC_LOC]=0; s->core[0][EXEC_BANK]=debris;
    if(kind==EXEC_CONTEXT_INTERPRETIVE) {
        s->core[0][EXEC_LOC]=incoming.location;
        s->core[0][EXEC_BANK]=(agc_word)(incoming.bank&076007);
        r->interpreter_control.intbit15=(agc_word)(incoming.bank&040000);
        r->interpreter_control.edop=0;
    }
    r->current=incoming; r->current_kind=kind;
}
#if !defined(EXEC_NATIVE_FINISH_FAMILY)||EXEC_NATIVE_FINISH_FAMILY==1
static enum exec_status change_mixed_basic(exec_native *r,exec_address resume) {
    uint8_t slot;
    exec_address incoming;
    enum exec_context_kind kind;
    enum exec_status status;
    if(r->state->alarm!=0) return EXEC_BAILOUT_PENDING;
    if(agc_classify(r->state->core[0][EXEC_PRIORITY])!=AGC_POSITIVE ||
       r->state->newjob==0 || r->state->newjob>84u ||
       r->state->newjob%EXEC_CORE_WORDS!=0) return EXEC_OUT_OF_SCOPE;
    slot=(uint8_t)(r->state->newjob/EXEC_CORE_WORDS);
    status=saved_target(r,slot,&incoming,&kind);
    if(status!=EXEC_OK) return status;
    exchange_saved(r,slot,resume,incoming,kind);
    return EXEC_OK;
}
#endif
#if !defined(EXEC_NATIVE_FINISH_FAMILY)||EXEC_NATIVE_FINISH_FAMILY==2
static enum exec_status complete_mixed(exec_native *r) {
    executive *s=r->state;
    uint8_t slot=candidate(s,0);
    exec_address incoming,outgoing;
    enum exec_context_kind kind;
    enum exec_status status;
    if(s->alarm!=0) return EXEC_BAILOUT_PENDING;
    if(agc_classify(s->core[0][EXEC_PRIORITY])!=AGC_POSITIVE) return EXEC_OUT_OF_SCOPE;
    if(slot==0) return exec_complete_basic(s,(agc_word)(r->current.bank&0160),&r->current);
    status=saved_target(r,slot,&incoming,&kind);
    if(status!=EXEC_OK) return status;
    outgoing.location=s->core[0][EXEC_LOC];
    outgoing.bank=(agc_word)(s->core[0][EXEC_BANK] | (r->current.bank&0160));
    (void)exec_endjob_select(s);
    exchange_saved(r,slot,outgoing,incoming,kind);
    return EXEC_OK;
}
static enum exec_status sleep_mixed(exec_native *r,agc_word cadr) {
    executive *s=r->state;
    uint8_t slot=candidate(s,0);
    exec_address incoming,outgoing;
    enum exec_context_kind kind;
    enum exec_status status;
    if(s->alarm!=0) return EXEC_BAILOUT_PENDING;
    if(agc_classify(s->core[0][EXEC_PRIORITY])!=AGC_POSITIVE) return EXEC_OUT_OF_SCOPE;
    if(slot==0) return exec_sleep_basic(s,cadr,r->current.bank,&r->current);
    status=saved_target(r,slot,&incoming,&kind);
    if(status!=EXEC_OK) return status;
    s->core[0][EXEC_LOC]=cadr;
    s->core[0][EXEC_PRIORITY]=agc_negate(s->core[0][EXEC_PRIORITY]);
    s->core[0][EXEC_BANK]=(agc_word)(r->current.bank&0177);
    s->newjob=(agc_word)(slot*EXEC_CORE_WORDS);
    outgoing.location=cadr; outgoing.bank=s->core[0][EXEC_BANK];
    exchange_saved(r,slot,outgoing,incoming,kind);
    return EXEC_OK;
}
static enum exec_status priority_mixed(exec_native *r,exec_address resume) {
    executive *s=r->state;
    agc_word packed=(agc_word)(r->requested_priority+
                               (s->core[0][EXEC_PRIORITY]&EXEC_LOW9));
    uint8_t slot=candidate(s,(agc_word)(packed+1u));
    exec_address incoming;
    enum exec_context_kind kind;
    enum exec_status status;
    if(s->alarm!=0) return EXEC_BAILOUT_PENDING;
    if(agc_classify(s->core[0][EXEC_PRIORITY])!=AGC_POSITIVE) return EXEC_OUT_OF_SCOPE;
    if(slot==0) return exec_priority_basic(s,r->requested_priority,resume,&r->current);
    status=saved_target(r,slot,&incoming,&kind);
    if(status!=EXEC_OK) return status;
    s->newprio=r->requested_priority; s->core[0][EXEC_PRIORITY]=packed;
    s->core[0][EXEC_LOC]=resume.location;
    s->core[0][EXEC_BANK]=(agc_word)(resume.bank&076007);
    s->newjob=(agc_word)(slot*EXEC_CORE_WORDS);
    exchange_saved(r,slot,resume,incoming,kind);
    return EXEC_OK;
}
#endif
static enum exec_status start_locked(exec_native *r) {
    enum exec_status status;
    if(r->state->alarm!=0) return fail(r,EXEC_BAILOUT_PENDING);
    if (r->state->newjob == AGC_NEG_ZERO) return EXEC_IDLE;
    if(mixed(r) && r->state->newjob!=0 &&
       r->state->newjob<=84u && r->state->newjob%EXEC_CORE_WORDS==0 &&
       (r->state->core[r->state->newjob/EXEC_CORE_WORDS][EXEC_LOC]&AGC_SIGN)) {
        uint8_t slot=(uint8_t)(r->state->newjob/EXEC_CORE_WORDS);
        exec_address incoming,outgoing;
        enum exec_context_kind kind;
        status=saved_target(r,slot,&incoming,&kind);
        if(status==EXEC_OK) {
            outgoing.location=r->state->core[0][EXEC_LOC];
            outgoing.bank=r->state->core[0][EXEC_BANK];
            r->state->core[0][EXEC_LOC]=2; r->state->core[0][EXEC_BANK]=0;
            exchange_saved(r,slot,outgoing,incoming,kind);
        }
    } else {
        status=exec_start_basic(r->state,&r->current);
        if(status==EXEC_OK) r->current_kind=EXEC_CONTEXT_BASIC;
    }
    if (status == EXEC_OK) r->running=1;
    return settle(r,status);
}
enum exec_status exec_native_start(exec_native *r) {
    enum exec_status status;
    uint8_t token;
    if (r->fault != EXEC_OK) return r->fault;
    if (r->running != 0) return EXEC_OUT_OF_SCOPE;
    token=exec_native_lock(r);
    status=exec_irq_service_locked(r);
    if (status == EXEC_OK) status=start_locked(r);
    else status=fail(r,status);
    exec_native_unlock(r,token);
    return status;
}
enum exec_status exec_native_poll(exec_native *r) {
    enum exec_status status;
    uint8_t token;
    if (r->fault != EXEC_OK) return r->fault;
    token=exec_native_lock(r);
    status=fail(r,exec_irq_service_locked(r));
    exec_native_unlock(r,token);
    return status;
}
enum exec_status exec_native_admit(exec_native *r, agc_word priority,
                                  exec_address address, int needs_vac) {
    enum exec_status status;
    uint8_t token;
    if (r->fault != EXEC_OK) return r->fault;
    token=exec_native_lock(r);
    status=exec_irq_service_locked(r);
    if(status==EXEC_OK && mixed(r) &&
       lookup_kind(r,address,EXEC_CONTEXT_BASIC)==0) status=EXEC_UNMAPPED;
    if (status == EXEC_OK)
        status=exec_admit(r->state,priority,address.location,address.bank,needs_vac);
    status=fail(r,status);
    exec_native_unlock(r,token);
    return status;
}
/* These stages assume the resident caller owns the producer guard. Each
 * service frame returns normally before the callback or another disk module
 * runs. The callback pointer always refers to the resident registry. */
enum exec_status exec_native_prepare(exec_native *r,exec_continuation *selected) {
    enum exec_status status;
    exec_continuation call=0;
    status=exec_irq_service_locked(r);
    if (status == EXEC_OK && r->running == 0) status=start_locked(r);
    if (status == EXEC_OK) {
        call=lookup(r,r->current);
        if (call == 0) status=EXEC_UNMAPPED;
    }
    if (status != EXEC_OK && status != EXEC_IDLE) status=fail(r,status);
    if (status==EXEC_OK) { r->requested_priority=0; r->in_callback=1; }
    *selected=call;
    return status;
}
enum exec_status exec_native_finish(exec_native *r,enum exec_action action,exec_address resume) {
    enum exec_status status;
    r->in_callback=0;
    /* A callback helper may already have latched a fault. Preserve its exact
     * boundary/status; never complete or exchange that failed running job. */
    status=r->fault != EXEC_OK ? r->fault :
        (action==EXEC_DISPATCHED_BASIC||action==EXEC_DISPATCHED_INTERPRETER) ? EXEC_OK : exec_irq_service_locked(r);
    if (status==EXEC_OK && r->requested_priority!=0 && action!=EXEC_REPRIORITIZE)
        status=EXEC_INVALID_ACTION;
    if (status == EXEC_OK) {
        if (0) {status=EXEC_INVALID_ACTION;}
#if !defined(EXEC_NATIVE_FINISH_FAMILY)||EXEC_NATIVE_FINISH_FAMILY==3
        else if (action == EXEC_DISPATCHED_BASIC||action==EXEC_DISPATCHED_INTERPRETER) {
            enum exec_context_kind kind=action==EXEC_DISPATCHED_BASIC?EXEC_CONTEXT_BASIC:EXEC_CONTEXT_INTERPRETIVE;
            if(r->current_kind!=EXEC_CONTEXT_BASIC) status=EXEC_INVALID_ACTION;
            else if(resume.location==0 && resume.bank==0) {
                if(kind!=EXEC_CONTEXT_BASIC || r->state->newjob!=AGC_NEG_ZERO ||
                   agc_classify(r->state->core[0][EXEC_PRIORITY])==AGC_POSITIVE)
                    status=EXEC_OUT_OF_SCOPE;
                else {r->current=resume;status=EXEC_IDLE;}
            } else if(resume.location==0 || resume.location>07777 ||
                      (resume.bank & (agc_word)~UINT16_C(076167))!=0 ||
                      agc_classify(r->state->core[0][EXEC_PRIORITY])!=AGC_POSITIVE)
                status=EXEC_OUT_OF_SCOPE;
            else if(kind==EXEC_CONTEXT_INTERPRETIVE &&
                    (resume.location<2u || r->interpreter_control.edop!=0 ||
                     r->interpreter_control.intbit15!=(resume.bank&040000u)))status=EXEC_OUT_OF_SCOPE;
            else if(lookup_kind(r,resume,kind)==0) status=EXEC_UNMAPPED;
            else {r->current=resume;r->current_kind=kind;}
        }
#endif
#if !defined(EXEC_NATIVE_FINISH_FAMILY)||EXEC_NATIVE_FINISH_FAMILY==2
        else if (action == EXEC_FINISH) {
            if(r->current_kind!=EXEC_CONTEXT_BASIC) status=EXEC_INVALID_ACTION;
            else status=mixed(r) ? complete_mixed(r) :
                exec_complete_basic(r->state,(agc_word)(r->current.bank & 0160),&r->current);
        }
#endif
#if !defined(EXEC_NATIVE_FINISH_FAMILY)||EXEC_NATIVE_FINISH_FAMILY==1
        else if (action == EXEC_YIELD) {
            if (resume.location == 0 || resume.location > 07777 || resume.bank > AGC_MASK)
                status=EXEC_OUT_OF_SCOPE;
            else if(r->current_kind==EXEC_CONTEXT_INTERPRETIVE) {
                if(resume.location<2 || r->interpreter_control.edop!=0 ||
                   resume.bank!=(agc_word)(r->state->core[0][EXEC_BANK] |
                                           (r->current.bank&0160))) status=EXEC_OUT_OF_SCOPE;
                else if(lookup_kind(r,resume,EXEC_CONTEXT_INTERPRETIVE)==0) status=EXEC_UNMAPPED;
                else {
                    if(r->state->alarm!=0) status=EXEC_BAILOUT_PENDING;
                    else if(agc_classify(r->state->core[0][EXEC_PRIORITY])!=AGC_POSITIVE)
                        status=EXEC_OUT_OF_SCOPE;
                    else if(r->state->newjob!=0) {
                        exec_address target;
                        enum exec_context_kind target_kind;
                        if(r->state->newjob>84u || r->state->newjob%EXEC_CORE_WORDS!=0)
                            status=EXEC_OUT_OF_SCOPE;
                        else status=saved_target(r,
                            (uint8_t)(r->state->newjob/EXEC_CORE_WORDS),&target,&target_kind);
                    }
                    if(status==EXEC_OK) r->state->core[0][EXEC_LOC]=(agc_word)(resume.location-1u);
                    if(status==EXEC_OK && r->state->newjob==0) {
                        r->state->core[0][EXEC_LOC]=resume.location;
                        r->current=resume;
                    } else if(status==EXEC_OK) {
                        exec_transfer transfer;
                        status=exec_change_interpretive(r->state,
                            (agc_word)(r->current.bank&0160),&r->interpreter_control,&transfer);
                        if(status==EXEC_OK) {
                            r->current=transfer.address;
                            r->current_kind=transfer.interpretive ?
                                EXEC_CONTEXT_INTERPRETIVE : EXEC_CONTEXT_BASIC;
                        }
                    }
                }
            } else if(mixed(r) && lookup_kind(r,resume,EXEC_CONTEXT_BASIC)==0)
                status=EXEC_UNMAPPED;
            else if (r->state->newjob == 0) { r->current=resume; status=EXEC_OK; }
            else status=mixed(r) ? change_mixed_basic(r,resume) :
                exec_change_basic(r->state,resume,&r->current);
        }
#endif
#if !defined(EXEC_NATIVE_FINISH_FAMILY)||EXEC_NATIVE_FINISH_FAMILY==2
        else if (action==EXEC_SLEEP) {
            if(r->current_kind!=EXEC_CONTEXT_BASIC) status=EXEC_INVALID_ACTION;
            else if (resume.location<02000 || resume.location>03777 ||
                (resume.bank & (agc_word)~UINT16_C(076167))!=0 ||
                (resume.bank & 0177)!=(r->current.bank & 0177)) status=EXEC_OUT_OF_SCOPE;
            else if (lookup(r,resume)==0) status=EXEC_UNMAPPED;
            else {
                agc_word cadr=(agc_word)((resume.location-02000) | (resume.bank & 076000));
                status=mixed(r) ? sleep_mixed(r,cadr) :
                    exec_sleep_basic(r->state,cadr,r->current.bank,&r->current);
            }
        } else if (action==EXEC_REPRIORITIZE) {
            if (r->current_kind!=EXEC_CONTEXT_BASIC || r->requested_priority==0) status=EXEC_INVALID_ACTION;
            else if (lookup(r,resume)==0) status=EXEC_UNMAPPED;
            else status=mixed(r) ? priority_mixed(r,resume) :
                exec_priority_basic(r->state,r->requested_priority,resume,&r->current);
        }
#endif
#if !defined(EXEC_NATIVE_FINISH_FAMILY)||EXEC_NATIVE_FINISH_FAMILY==1
        else if(action==EXEC_ENTER_INTERPRETER || action==EXEC_EXIT_INTERPRETER) {
            enum exec_context_kind next_kind=action==EXEC_ENTER_INTERPRETER ?
                EXEC_CONTEXT_INTERPRETIVE : EXEC_CONTEXT_BASIC;
            if(!mixed(r) || r->current_kind==next_kind ||
               resume.location==0 || resume.location>07777 ||
               resume.bank!=r->current.bank ||
               (action==EXEC_EXIT_INTERPRETER && (resume.location<2 ||
                                                r->interpreter_control.edop!=0)))
                status=EXEC_OUT_OF_SCOPE;
            else if(lookup_kind(r,resume,next_kind)==0) status=EXEC_UNMAPPED;
            else {
                r->state->core[0][EXEC_LOC]=action==EXEC_ENTER_INTERPRETER ?
                    resume.location : (agc_word)(resume.location-1u);
                r->state->core[0][EXEC_BANK]=(agc_word)(resume.bank&076007);
                if(action==EXEC_ENTER_INTERPRETER) {
                    r->interpreter_control.intbit15=(agc_word)(resume.bank&040000);
                    r->interpreter_control.edop=0;
                }
                r->current=resume; r->current_kind=next_kind;
            }
        }
#endif
        else status=EXEC_INVALID_ACTION;
    }
    status=settle(r,status);
    return status;
}
/* The resident trampoline holds no frame in an unloadable service slot. */
__attribute__((noinline)) enum exec_status exec_native_step(exec_native *r) {
    exec_address resume={0,0};
    exec_continuation call=0;
    enum exec_action action;
    enum exec_status status;
    uint8_t token;
    if(r->fault!=EXEC_OK)return r->fault;
    if(r->in_callback!=0)return EXEC_OUT_OF_SCOPE;
    token=exec_native_lock(r);
    if(r->services!=0)status=r->services->prepare(r->service_user,r,&call);
    else {
#ifdef EXEC_NATIVE_RESIDENT_SERVICES
        status=fail(r,EXEC_OUT_OF_SCOPE);
#else
        status=exec_native_prepare(r,&call);
#endif
    }
    if(status==EXEC_OK&&call==0)status=fail(r,EXEC_UNMAPPED);
    exec_native_unlock(r,token);
    if(status!=EXEC_OK)return status;
    action=call(r->state,r->user,&resume);
    token=exec_native_lock(r);
    if(r->services!=0)status=r->services->finish(r->service_user,r,action,resume);
    else {
#ifdef EXEC_NATIVE_RESIDENT_SERVICES
        status=fail(r,EXEC_OUT_OF_SCOPE);
#else
        status=exec_native_finish(r,action,resume);
#endif
    }
    if(status!=EXEC_OK&&status!=EXEC_IDLE)status=fail(r,status);
    exec_native_unlock(r,token);
    return status;
}
enum exec_status exec_native_wake(exec_native *r, agc_word cadr) {
    enum exec_status status;
    uint8_t token;
    if (r->fault!=EXEC_OK) return r->fault;
    token=exec_native_lock(r);
    status=exec_irq_service_locked(r);
    if (status==EXEC_OK) {
        status=exec_wake_basic(r->state,cadr);
        if (status==EXEC_OK) {
            uint8_t slot=(uint8_t)(r->state->locctr/EXEC_CORE_WORDS);
            exec_address a={r->state->core[slot][EXEC_LOC],r->state->core[slot][EXEC_BANK]};
            if (lookup_kind(r,a,EXEC_CONTEXT_BASIC)==0) status=EXEC_UNMAPPED;
        }
    }
    if (status!=EXEC_NOT_FOUND) status=fail(r,status);
    exec_native_unlock(r,token);
    return status;
}
enum exec_status exec_native_request_priority(exec_native *r, agc_word priority) {
    enum exec_status status=EXEC_OK;
    uint8_t token;
    if (r->fault!=EXEC_OK) return r->fault;
    token=exec_native_lock(r);
    if (r->in_callback==0 || priority==0 || (priority & UINT16_C(0140777))!=0)
        status=EXEC_OUT_OF_SCOPE;
    else r->requested_priority=priority;
    status=fail(r,status);
    exec_native_unlock(r,token);
    return status;
}
